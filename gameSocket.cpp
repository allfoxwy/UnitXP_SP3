#include "pch.h"

#include <cstdint>

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mstcpip.h>

#include "gameSocket.h"

CONNECT p_connect = NULL;
CONNECT p_original_connect = NULL;

static bool quickACK_failed = false;
static bool smallerMTU_failed = false;
static bool biggerWindow_failed = false;

static void setFixedReceivingWindow(SOCKET s) {
    // The reason behind this function is that Windows TCP auto-tuning determines TCP window size by application retrieve rate. (and BDP)
    // I have a suspection of the game's data retrieving pattern is not well fit into Windows auto-tuning,
    // because @pepopo report he witness exceptional good FPS on a local server.

    // And it seems later version of the game is also tuning it manually.
    
    // According to https://learn.microsoft.com/en-us/windows/win32/winsock/sio-set-compatibility-mode
    // If an application sets a valid receive window size with the SO_RCVBUF socket option,
    // the stack will use the size specified and window receive auto-tuning will disabled.
    
    DWORD window = 1024 * 1024;
    if (0 != setsockopt(s, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&window), sizeof window)) {
        biggerWindow_failed = true;
    }
}

static void enableQuickACK(SOCKET s) {
    // From https://cygwin.com/git/?p=newlib-cygwin.git;a=commitdiff;h=ee2292413792f0360d357bc200c5e947eae516e6
    int quickAck = 1;
    DWORD dummy = 0;

    // SIO_TCP_SET_ACK_FREQUENCY is not supported in Linux WINE.
    if (0 != WSAIoctl(s, SIO_TCP_SET_ACK_FREQUENCY, &quickAck, sizeof(quickAck), NULL, 0, &dummy, NULL, NULL)) {
        quickACK_failed = true;
    }
}

static void setSmallerMTU(SOCKET s) {
    // According to https://www.cisco.com/c/en/us/support/docs/ip/generic-routing-encapsulation-gre/25885-pmtud-ipfrag.html
    // The MTU value of 1400 is recommended because it covers the most common GRE + IPv4sec mode combinations.
    // Also, there is no discernible downside to allowing for an extra 20 or 40 bytes overhead.
    // It is easier to remember and set one value and this value covers almost all scenarios.
    DWORD mtu = 1400;

    // IP_USER_MTU is not supported in Linux WINE.
    if (0 != setsockopt(s, IPPROTO_IP, IP_USER_MTU, reinterpret_cast<char*>(&mtu), sizeof mtu)) {
        smallerMTU_failed = true;
    }
}

int WSAAPI detoured_connect(SOCKET s, const struct sockaddr FAR* addr, int len) {
    WSAPROTOCOL_INFOW sInfo = {};
    int bufLen = sizeof sInfo;
    int r = getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFOW, reinterpret_cast<char*>(&sInfo), &bufLen);

    if (r == 0) {
        if ((sInfo.iAddressFamily == AF_INET || sInfo.iAddressFamily == AF_INET6) &&
            sInfo.iSocketType == SOCK_STREAM &&
            sInfo.iProtocol == IPPROTO_TCP) {

            setSmallerMTU(s);
            setFixedReceivingWindow(s);
            enableQuickACK(s);
        }
    }

    return p_original_connect(s, addr, len);
}

bool gameSocket_isQuickACK() {
    return quickACK_failed == false;
}

bool gameSocket_hasSmallerMTU() {
    return smallerMTU_failed == false;
}

bool gameSocket_hasBiggerWindow() {
    return biggerWindow_failed == false;
}
