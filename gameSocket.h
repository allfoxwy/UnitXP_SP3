#pragma once

#include <winsock2.h>

// Attempt to disable TCP delayed acknowledgment
// Attempt to disable Nagle's algorithm (The 1.12 game already doing this)
// - https://en.wikipedia.org/wiki/TCP_delayed_acknowledgment
// - https://en.wikipedia.org/wiki/Nagle%27s_algorithm
// - https://cygwin.com/git/?p=newlib-cygwin.git;a=commitdiff;h=ee2292413792f0360d357bc200c5e947eae516e6

typedef int(PASCAL FAR* SEND_RECV)(SOCKET, const char FAR*, int, int);

extern SEND_RECV p_original_send;
extern SEND_RECV p_original_recv;

int PASCAL FAR detoured_send(SOCKET s, const char FAR* buf, int len, int flags);
int PASCAL FAR detoured_recv(SOCKET s, const char FAR* buf, int len, int flags);
