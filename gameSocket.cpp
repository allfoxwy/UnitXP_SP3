#include "pch.h"

#include <cstdint>

#include <winsock2.h>
#include <mstcpip.h>

#include "gameSocket.h"

SEND p_send = NULL;
RECV p_recv = NULL;
SENDTO p_sendto = NULL;
RECVFROM p_recvfrom = NULL;
SEND p_original_send = NULL;
RECV p_original_recv = NULL;
SENDTO p_original_sendto = NULL;
RECVFROM p_original_recvfrom = NULL;
bool TCP_quickACK = false;

static void TCP_QUICKACK(SOCKET s) {
	WSAPROTOCOL_INFOW sInfo = {};
	int bufLen = sizeof(sInfo);
	int r = getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFOW, reinterpret_cast<char*>(&sInfo), &bufLen);

	if (r == 0) {
		if ((sInfo.iAddressFamily == AF_INET || sInfo.iAddressFamily == AF_INET6) &&
			sInfo.iSocketType == SOCK_STREAM &&
			sInfo.iProtocol == IPPROTO_TCP) {

			// From https://cygwin.com/git/?p=newlib-cygwin.git;a=commitdiff;h=ee2292413792f0360d357bc200c5e947eae516e6
			int quickAck = 1;
			DWORD dummy = 0;
			int ioctlResult = WSAIoctl(s, SIO_TCP_SET_ACK_FREQUENCY, &quickAck, sizeof(quickAck), NULL, 0, &dummy, NULL, NULL);
			// SIO_TCP_SET_ACK_FREQUENCY is not supported in Linux WINE.

			if (ioctlResult == 0) {
				TCP_quickACK = true;
			}
			else {
				TCP_quickACK = false;
			}
		}
	}
}

int WSAAPI detoured_send(SOCKET s, const char FAR* buf, int len, int flags) {
	TCP_QUICKACK(s);
	return p_original_send(s, buf, len, flags);
}

int WSAAPI detoured_recv(SOCKET s, char FAR* buf, int len, int flags) {
	TCP_QUICKACK(s);
	return p_original_recv(s, buf, len, flags);
}

int WSAAPI detoured_sendto(SOCKET s, const char FAR* buf, int len, int flags, const struct sockaddr FAR* to, int tolen) {
	TCP_QUICKACK(s);
	return p_original_sendto(s, buf, len, flags, to, tolen);
}

int WSAAPI detoured_recvfrom(SOCKET s, char FAR* buf, int len, int flags, struct sockaddr FAR* from, int FAR* fromlen) {
	TCP_QUICKACK(s);
	return p_original_recvfrom(s, buf, len, flags, from, fromlen);
}
