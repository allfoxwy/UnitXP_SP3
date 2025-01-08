#include "pch.h"

#include <cstdint>

#include <winsock2.h>
#include <mstcpip.h>

#include "gameSocket.h"

extern SEND_RECV p_original_send = NULL;
extern SEND_RECV p_original_recv = NULL;

static void setSocketOpt(SOCKET s) {
	const DWORD noDelay = TRUE;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));

	int quickAck = 1;
	DWORD dummy = 0;
	WSAIoctl(s, SIO_TCP_SET_ACK_FREQUENCY, &quickAck, sizeof(quickAck), NULL, 0, &dummy, NULL, NULL);
}

int PASCAL FAR detoured_send(SOCKET s, const char FAR* buf, int len, int flags) {
	setSocketOpt(s);
	return p_original_send(s, buf, len, flags);
}

int PASCAL FAR detoured_recv(SOCKET s, const char FAR* buf, int len, int flags) {
	setSocketOpt(s);
	return p_original_recv(s, buf, len, flags);
}
