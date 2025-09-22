#include "chat.h"

static void print_help(void)
{
    printf("Available commands:\n");
    printf("  /quit           - Exit the chat\n");
    printf("  /nick <name>    - Change your username\n");
    printf("  /clear          - Clear the chat screen\n");
    printf("  /help           - Show this help menu\n");
}

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
        sock = tcp_listen_accept(argv[2]);
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s client <host> <port>\n", argv[0]);
            cleanup_sockets();
            return EXIT_FAILURE;
        }
        sock = tcp_connect(argv[2], argv[3]);
    }

    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Failed to establish connection.\n");
        cleanup_sockets();
        return EXIT_FAILURE;
    }

    // Ask for username
    char username[64];
    printf("Enter your username: ");
    fflush(stdout);
    if (!fgets(username, sizeof(username), stdin))
        strcpy(username, "anonymous");
    username[strcspn(username, "\n")] = '\0';
    if (strlen(username) == 0) strcpy(username, "anonymous");

    chat_ctx_t ctx;
    ctx.sock = sock;
    strncpy(ctx.username, username, sizeof(ctx.username));

    // Start receiver thread
    start_recv_thread(&ctx);

    // Main thread = sending messages
    char input[1024];
    while (true)
    {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        // --- Slash commands ---
        if (strcmp(input, "/quit") == 0)
            break;
        else if (strncmp(input, "/nick ", 6) == 0)
        {
            const char* newnick = input + 6;
            if (strlen(newnick) > 0 && strlen(newnick) < sizeof(ctx.username))
            {
                strcpy(ctx.username, newnick);
                printf("Username changed to: %s\n", ctx.username);
            }
            else
            {
                printf("Invalid nickname.\n");
            }
            continue;
        }
        else if (strcmp(input, "/clear") == 0)
        {
            // ANSI escape code: clear screen + move cursor to top-left
            clear_screen();
            fflush(stdout);
            continue;
        }
        else if (strcmp(input, "/help") == 0)
        {
            print_help();
            continue;
        }

        // --- Normal message ---
        message_t* msg = create_message(ctx.username, input);
        if (!msg || !send_message(sock, msg))
        {
            fprintf(stderr, "Failed to send message.\n");
            free_message(msg);
            break;
        }
        free_message(msg);
    }

    close_socket(sock);
    cleanup_sockets();
    return EXIT_SUCCESS;
}
