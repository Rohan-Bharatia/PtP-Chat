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

#ifndef CHAT_BASE_C
    #define CHAT_BASE_C

#include "chat.h"

static char* strdup(const char* s)
{
    size_t len = strlen(s) + 1;
    char* dup  = malloc(len);
    if (dup != NULL)
        memcpy(dup, s, len);
    return dup;
}

message_t* create_message(char* sender, char* message)
{
    time_t now = time(NULL);
    char* timestamp = malloc(sizeof(char) * 32);
    strftime(timestamp, sizeof(char) * 32, "%Y-%m-%d %H:%M:%S", localtime(&now));

    message_t* msg = malloc(sizeof(message_t));
    msg->sender    = strdup(sender);
    msg->message   = strdup(message);
    msg->timestamp = timestamp;
    return msg;
}

void print_message(message_t* msg)
{
    printf("%s - %s:\n%s\n", msg->sender, msg->timestamp, msg->message);
}

void free_message(message_t* msg)
{
    free(msg->timestamp);
    free(msg);
}

#endif
