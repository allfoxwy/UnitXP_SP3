#pragma once

#include <winsock2.h>

// Attempt to disable TCP delayed acknowledgment
// - https://en.wikipedia.org/wiki/TCP_delayed_acknowledgment
// Vanilla is linked with Winsock 1. But WSAIoctl() is in Winsock 2.

typedef int(WSAAPI* SEND)(SOCKET, const char FAR*, int, int);
typedef int(WSAAPI* RECV)(SOCKET, char FAR*, int, int);
typedef int(WSAAPI* SENDTO)(SOCKET, const char FAR*, int, int, const struct sockaddr FAR*, int);
typedef int(WSAAPI* RECVFROM)(SOCKET, char FAR*, int, int, struct sockaddr FAR*, int FAR*);

extern SEND p_send;
extern RECV p_recv;
extern SENDTO p_sendto;
extern RECVFROM p_recvfrom;

extern SEND p_original_send;
extern RECV p_original_recv;
extern SENDTO p_original_sendto;
extern RECVFROM p_original_recvfrom;
extern bool TCP_quickACK;

int WSAAPI detoured_send(SOCKET s, const char FAR* buf, int len, int flags);
int WSAAPI detoured_recv(SOCKET s, char FAR* buf, int len, int flags);
int WSAAPI detoured_sendto(SOCKET s, const char FAR* buf, int len, int flags, const struct sockaddr FAR* to, int tolen);
int WSAAPI detoured_recvfrom(SOCKET s, char FAR* buf, int len, int flags, struct sockaddr FAR* from, int FAR* fromlen);
