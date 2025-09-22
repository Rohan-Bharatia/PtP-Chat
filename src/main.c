#pragma region LICENSE
#pragma endregion LICENSE

#ifndef MAIN_C
    #define MAIN_C

#include "chat.h"

#include "chat.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s server <port>\n", argv[0]);
        fprintf(stderr, "  %s client <host> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (init_sockets() != 0)
    {
        fprintf(stderr, "Failed to initialize sockets.\n");
        return EXIT_FAILURE;
    }

    socket_t sock = INVALID_SOCKET;

    if (strcmp(argv[1], "server") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "Usage: %s server <port>\n", argv[0]);
            cleanup_sockets();
            return EXIT_FAILURE;
        }

        const char* port = argv[2];
        sock = tcp_listen_accept(port);
        if (sock == INVALID_SOCKET)
        {
            fprintf(stderr, "Server failed to accept connection on port %s.\n", port);
            cleanup_sockets();
            return EXIT_FAILURE;
        }

        printf("Client connected on port %s. Start chatting!\n", port);
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s client <host> <port>\n", argv[0]);
            cleanup_sockets();
            return EXIT_FAILURE;
        }

        const char* host = argv[2];
        const char* port = argv[3];
        sock = tcp_connect(host, port);
        if (sock == INVALID_SOCKET)
        {
            fprintf(stderr, "Client failed to connect to %s:%s.\n", host, port);
            cleanup_sockets();
            return EXIT_FAILURE;
        }

        printf("Connected to %s:%s. Start chatting!\n", host, port);
    }
    else
    {
        fprintf(stderr, "Unknown mode: %s\n", argv[1]);
        cleanup_sockets();
        return EXIT_FAILURE;
    }

    // Chat loop
    char input[1024];
    while (true)
    {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0'; // strip newline

        if (strcmp(input, "/quit") == 0)
            break;

        message_t* msg = create_message("me", input);
        if (!send_message(sock, msg))
        {
            fprintf(stderr, "Failed to send message.\n");
            free_message(msg);
            break;
        }
        free_message(msg);

        message_t* reply = recv_message(sock);
        if (!reply)
        {
            fprintf(stderr, "Connection closed or failed.\n");
            break;
        }
        print_message(reply);
        free_message(reply);
    }

    close_socket(sock);
    cleanup_sockets();
    return EXIT_SUCCESS;
}

#endif
