#pragma once

#include <winsock2.h>

// Attempt to disable TCP delayed acknowledgment
// - https://en.wikipedia.org/wiki/TCP_delayed_acknowledgment
// Vanilla is linked with Winsock 1. But WSAIoctl() is in Winsock 2.

// Attempt to set underlying interface MTU to a smaller value
// so that the connection could pass a stricter path

typedef int(WSAAPI* CONNECT)(SOCKET, const struct sockaddr FAR*, int);
extern CONNECT p_connect;
extern CONNECT p_original_connect;

int WSAAPI detoured_connect(SOCKET s, const struct sockaddr FAR* addr, int len);

bool gameSocket_isQuickACK();
bool gameSocket_hasSmallerMTU();
bool gameSocket_hasBiggerWindow();
