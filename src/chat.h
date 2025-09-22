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

#pragma once

#ifndef CHAT_H
    #define CHAT_H

#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 600
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #define UNICODE
    #define _UNICODE
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    typedef SOCKET socket_t;
    #define close_socket closesocket
    #pragma comment(lib, "ws2_32.lib")
#else
    #define __STDC_LIMIT_MACROS
    #define __STDC_FORMAT_MACROS
    #define __STDC_CONSTANT_MACROS
    #define __STDC_WANT_LIB_EXT1__
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define close_socket close
#endif
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

typedef struct message_t
{
    char* sender;
    char* message;
    char* timestamp;
} __attribute__((__packed__)) message_t;

message_t* create_message(char* sender, char* msg);
void print_message(message_t* msg);
void free_message(message_t* msg);
int init_sockets(void);
void cleanup_sockets(void);
socket_t tcp_connect(const char* host, const char* port);
socket_t tcp_listen_accept(const char* port);
ssize_t send_all(socket_t sock, const void* buffer, size_t len);
ssize_t recv_all(socket_t sock, void* buffer, size_t len);
bool send_message(socket_t sock, message_t* msg);
message_t* recv_message(socket_t sock);

#endif
