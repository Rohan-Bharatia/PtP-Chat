#pragma region LICENSE

// MIT License
//
// Copyright (c) 2025 Rohan Bharatia
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma endregion LICENSE

#ifndef CHAT_C
    #define CHAT_C

#include "chat.h"

void clear_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

message_t* create_message(char* sender, char* message)
{
    if (!sender || !message) return NULL;
    message_t* m = (message_t*)malloc(sizeof(message_t));
    if (!m) return NULL;
    size_t sl = strlen(sender);
    size_t ml = strlen(message);

    m->sender = (char*)malloc(sl + 1);
    m->message = (char*)malloc(ml + 1);

    if (!m->sender || !m->message)
    {
        free(m->sender);
        free(m->message);
        free(m);
        return NULL;
    }

    memcpy(m->sender, sender, sl + 1);
    memcpy(m->message, message, ml + 1);

    /* timestamp as ISO-like: YYYY-MM-DD HH:MM:SS */
    time_t t = time(NULL);
    struct tm tm_buf;
#if defined(_WIN32)
    gmtime_s(&tm_buf, &t);
#else
    struct tm* tmp = gmtime(&t);
    if (tmp)
        tm_buf = *tmp;
#endif
    char ts[64];
    if (strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S UTC", &tm_buf) == 0)
        strcpy(ts, "1970-01-01 00:00:00 UTC");

    m->timestamp = (char*)malloc(strlen(ts) + 1);
    if (!m->timestamp)
    {
        free(m->sender);
        free(m->message);
        free(m);
        return NULL;
    }
    strcpy(m->timestamp, ts);
    return m;
}

void print_message(message_t* msg)
{
    if (!msg) return;
    printf("[%s] %s: %s\n", msg->timestamp ? msg->timestamp : "?", msg->sender ? msg->sender : "?", msg->message ? msg->message : "");
    fflush(stdout);
}

void free_message(message_t* msg)
{
    if (!msg) return;
    free(msg->sender);
    free(msg->message);
    free(msg->timestamp);
    free(msg);
}

/* socket/IO helpers */

int init_sockets(void)
{
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return -1;
#endif
    return 0;
}

void cleanup_sockets(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

/* connect to host:port, return connected socket or INVALID_SOCKET */
socket_t tcp_connect(const char* host, const char* port)
{
    struct addrinfo hints, *res, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int rc = getaddrinfo(host, port, &hints, &res);
    if (rc != 0)
    {
#ifdef _WIN32
        fprintf(stderr, "getaddrinfo: %d\n", rc);
#else
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
#endif
        return INVALID_SOCKET;
    }

    socket_t s = INVALID_SOCKET;
    for (rp = res; rp != NULL; rp = rp->ai_next)
    {
        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s == INVALID_SOCKET) continue;
        if (connect(s, rp->ai_addr, (int)rp->ai_addrlen) == 0) break;
        close_socket(s);
        s = INVALID_SOCKET;
    }

    freeaddrinfo(res);
    if (s == INVALID_SOCKET)
    {
        fprintf(stderr, "Could not connect to %s:%s\n", host, port);
    }
    return s;
}

/* Listen on port and accept one connection, return connected socket or INVALID_SOCKET */
socket_t tcp_listen_accept(const char* port)
{
    struct addrinfo hints, *res, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; /* allow v4 or v6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; /* for bind */
    hints.ai_protocol = IPPROTO_TCP;

    int rc = getaddrinfo(NULL, port, &hints, &res);
    if (rc != 0)
    {
#ifdef _WIN32
        fprintf(stderr, "getaddrinfo: %d\n", rc);
#else
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
#endif
        return INVALID_SOCKET;
    }

    socket_t listen_sock = INVALID_SOCKET;
    for (rp = res; rp != NULL; rp = rp->ai_next)
    {
        listen_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listen_sock == INVALID_SOCKET) continue;

        /* Allow reuse */
        int yes = 1;
#ifdef _WIN32
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
#else
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif

        if (bind(listen_sock, rp->ai_addr, (int)rp->ai_addrlen) == 0)
            break;

        close_socket(listen_sock);
        listen_sock = INVALID_SOCKET;
    }

    freeaddrinfo(res);

    if (listen_sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Failed to bind on port %s\n", port);
        return INVALID_SOCKET;
    }

    if (listen(listen_sock, 1) != 0)
    {
        fprintf(stderr, "listen failed\n");
        close_socket(listen_sock);
        return INVALID_SOCKET;
    }

    printf("Listening on port %s, waiting for a connection...\n", port);
    struct sockaddr_storage peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    socket_t client = accept(listen_sock, (struct sockaddr*)&peeraddr, &peerlen);
    if (client == INVALID_SOCKET)
    {
        fprintf(stderr, "accept failed\n");
        close_socket(listen_sock);
        return INVALID_SOCKET;
    }
    close_socket(listen_sock); /* single-peer app; close listen socket */
    return client;
}

/* send_all / recv_all - ensure full transfer */
ssize_t send_all(socket_t s, const void* buf, size_t len)
{
    const char* p = (const char*)buf;
    size_t left = len;
    while (left > 0)
    {
#ifdef _WIN32
        int sent = send(s, p, (int)left, 0);
#else
        ssize_t sent = send(s, p, left, 0);
#endif
        if (sent == SOCKET_ERROR || sent == 0)
        {
#ifdef _WIN32
            int err = WSAGetLastError();
            (void)err;
#else
            (void)errno;
#endif
            return -1;
        }
        p += sent;
        left -= sent;
    }
    return (ssize_t)len;
}

ssize_t recv_all(socket_t s, void* buf, size_t len)
{
    char* p = (char*)buf;
    size_t left = len;
    while (left > 0)
    {
#ifdef _WIN32
        int r = recv(s, p, (int)left, 0);
#else
        ssize_t r = recv(s, p, left, 0);
#endif
        if (r == 0) return 0; /* orderly shutdown */
        if (r == SOCKET_ERROR)
        {
#ifdef _WIN32
            int err = WSAGetLastError();
            (void)err;
#else
            (void)errno;
#endif
            return -1;
        }
        p += r;
        left -= r;
    }
    return (ssize_t)len;
}

/* Protocol:
 * header: 3 x uint32_t (network order) - lengths of sender, message, timestamp
 * then raw bytes (no terminating zeros required, but we will send them so receiver can use strcpy)
 */
bool send_message(socket_t s, message_t* m)
{
    if (!m || s == INVALID_SOCKET) return false;
    uint32_t sl = (uint32_t)strlen(m->sender);
    uint32_t ml = (uint32_t)strlen(m->message);
    uint32_t tl = (uint32_t)strlen(m->timestamp);

    uint32_t nl = htonl(sl);
    uint32_t nl2 = htonl(ml);
    uint32_t nl3 = htonl(tl);

    /* send header */
    uint32_t header[3];
    header[0] = nl;
    header[1] = nl2;
    header[2] = nl3;
    if (send_all(s, header, sizeof(header)) != (ssize_t)sizeof(header)) return false;

    /* send payloads (we send exact bytes without an extra null; receiver will allocate +1 and null-terminate) */
    if (sl > 0)
    {
        if (send_all(s, m->sender, sl) != (ssize_t)sl) return false;
    }
    if (ml > 0)
    {
        if (send_all(s, m->message, ml) != (ssize_t)ml) return false;
    }
    if (tl > 0)
    {
        if (send_all(s, m->timestamp, tl) != (ssize_t)tl) return false;
    }
    return true;
}

message_t* recv_message(socket_t s)
{
    uint32_t header[3];
    ssize_t r = recv_all(s, header, sizeof(header));
    if (r == 0) return NULL; /* peer closed */
    if (r != (ssize_t)sizeof(header)) return NULL;

    uint32_t sl = ntohl(header[0]);
    uint32_t ml = ntohl(header[1]);
    uint32_t tl = ntohl(header[2]);

    /* basic sanity check to avoid runaway allocations */
    const uint32_t MAX_FIELD = 1 << 20; /* 1 MB per field */
    if (sl > MAX_FIELD || ml > MAX_FIELD || tl > MAX_FIELD) return NULL;

    char* s_buf = (char*)malloc((size_t)sl + 1);
    char* m_buf = (char*)malloc((size_t)ml + 1);
    char* t_buf = (char*)malloc((size_t)tl + 1);
    if (!s_buf || !m_buf || !t_buf)
    {
        free(s_buf); free(m_buf); free(t_buf);
        return NULL;
    }

    if (sl > 0)
    {
        if (recv_all(s, s_buf, sl) != (ssize_t)sl) { free(s_buf); free(m_buf); free(t_buf); return NULL; }
    }
    if (ml > 0)
    {
        if (recv_all(s, m_buf, ml) != (ssize_t)ml) { free(s_buf); free(m_buf); free(t_buf); return NULL; }
    }
    if (tl > 0)
    {
        if (recv_all(s, t_buf, tl) != (ssize_t)tl) { free(s_buf); free(m_buf); free(t_buf); return NULL; }
    }

    s_buf[sl] = '\0';
    m_buf[ml] = '\0';
    t_buf[tl] = '\0';

    message_t* msg = (message_t*)malloc(sizeof(message_t));
    if (!msg) { free(s_buf); free(m_buf); free(t_buf); return NULL; }
    msg->sender = s_buf;
    msg->message = m_buf;
    msg->timestamp = t_buf;
    return msg;
}

thread_t recv_thread(void* arg)
{
    chat_ctx_t* ctx = (chat_ctx_t*)arg;

    while (true)
    {
        message_t* msg = recv_message(ctx->sock);

        if (!msg)
        {
            fprintf(stderr, "Connection closed\n");
            break;
        }

        printf("\n");
        print_message(msg);
        free_message(msg);
        printf("> ");
        fflush(stdout);
    }

    return 0;
}

void start_recv_thread(chat_ctx_t* ctx)
{
#ifdef _WIN32
    uintptr_t th = _beginthreadex(NULL, 0, recv_thread, ctx, 0, NULL);
#else
    pthread_t th;
    pthread_create(&th, NULL, recv_thread, ctx);
#endif
}

#endif
