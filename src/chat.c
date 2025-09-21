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

message_t* create_message(const char* sender, const char* msg)
{
    if (!sender || !msg)
        return NULL;

    message_t* m = (message_t*)malloc(sizeof(message_t));
    if (!m)
        return NULL;

    size_t sl = strlen(sender);
    size_t ml = strlen(msg);

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
    memcpy(m->message, msg, ml + 1);

    time_t now = time(NULL);
    char ts[32];
    if (strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S UTC", localtime(&now)) == 0)
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
    if (!msg)
        return;

    printf("[%s] %s: %s\n",
            msg->timestamp ? msg->timestamp : "?",
            msg->sender ? msg->sender : "?",
            msg->message ? msg->message : "");
    fflush(stdout);
}

void free_message(message_t* msg)
{
    if (!msg)
        return;

    free(msg->sender);
    free(msg->message);
    free(msg->timestamp);
    free(msg);
}

int main(int argc, char** argv)
{
    message_t* msg = create_message("Rohan", "Hello World!");
    print_message(msg);
    free_message(msg);
    return 0;
}

#endif
