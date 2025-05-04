#include "pch.h"

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#include <new>
#include <unordered_map>
#include <exception>
#include <sstream>

#include "Vanilla1121_functions.h"
#include "LuaDebug.h"

static SOCKET debuggerSocket = INVALID_SOCKET;
static bool continueTillBreakpoint = false;

static int LuaDebug_sendLoop(const char* data, const int total) {
    int sent = 0;
    while (sent < total) {
        int wish = total - sent;
        int loopSent = send(debuggerSocket, &data[sent], wish, 0);
        if (loopSent == SOCKET_ERROR) {
            throw std::exception("exception thrown at send loop");
        }
        sent += loopSent;
    }
    return sent;
}

static int LuaDebug_recvLoop(char* data, const int total) {
    int received = 0;
    while (received < total) {
        int wish = total - received;
        int loopRecv = recv(debuggerSocket, &data[received], wish, 0);
        if (loopRecv == SOCKET_ERROR) {
            throw std::exception("exception thrown at recv loop");
        }
        received += loopRecv;
    }
    return received;
}

static int LuaDebug_sendInteger(const int value) {
    u_long data = htonl(*reinterpret_cast<const u_long*>(&value));
    return LuaDebug_sendLoop(reinterpret_cast<const char*>(&data), sizeof(data));
}

static int LuaDebug_recvInteger(int& data) {
    u_long recvData = 0;

    int result = LuaDebug_recvLoop(reinterpret_cast<char*>(&recvData), sizeof(recvData));

    recvData = ntohl(recvData);
    data = *reinterpret_cast<int*>(&recvData);

    return result;
}

static int LuaDebug_sendDouble(const double value) {
    uint64_t data = htond(value);
    return LuaDebug_sendLoop(reinterpret_cast<const char*>(&data), sizeof(data));
}

static int LuaDebug_sendString(const std::string str) {
    int len = str.length();
    LuaDebug_sendInteger(len);

    return LuaDebug_sendLoop(str.data(), len);
}

static int LuaDebug_recvString(std::string& str) {
    int len = 0;
    LuaDebug_recvInteger(len);

    char* recvBuffer = new(std::nothrow) char[len + 1];
    if (recvBuffer == nullptr) {
        throw std::exception("failed to allocate recv buffer");
    }

    try {
        LuaDebug_recvLoop(recvBuffer, len);
    }
    catch (const std::exception& err) {
        delete[] recvBuffer;
        throw err;
    }

    recvBuffer[len] = '\0';
    str = recvBuffer;

    delete[] recvBuffer;

    return 1;
}

void __fastcall LuaDebug_hook(void* L, lua_Debug* active_record) {

    if (debuggerSocket == INVALID_SOCKET) {
        lua_sethook(L, &LuaDebug_hook, 0, 0);
        return;
    }

    if (continueTillBreakpoint) {
        return;
    }

    try {
        LuaDebug_sendString("debug event");

        LuaDebug_sendInteger(active_record->event);

        std::unordered_map<int, lua_Debug> stack;

        lua_Debug* current_stack_item = NULL;

        while (true) {
            std::string cmd = "";

            LuaDebug_recvString(cmd);

            if (cmd == "next") {
                return;
            }

            if (cmd == "continue till breakpoint") {
                continueTillBreakpoint = true;
                return;
            }

            // same as lua_getstack
            // recv 1 integer as stack depth
            // return 0 for fail at lua_getstack
            // return -1 for fail at lua_getinfo
            if (cmd == "get stack") {
                int depth = -1;
                LuaDebug_recvInteger(depth);

                auto i = stack.find(depth);
                if (i != stack.end()) {
                    current_stack_item = &(i->second);
                    LuaDebug_sendInteger(1);
                    continue;
                }

                stack[depth] = {};
                int getstack_result = lua_getstack(L, depth, &stack[depth]);
                if (1 != getstack_result) {
                    stack.erase(depth);
                    LuaDebug_sendInteger(0);
                    continue;
                }

                int getinfo_result = lua_getinfo(L, "Sunl", &stack[depth]);
                if (0 == getinfo_result) {
                    stack.erase(depth);
                    LuaDebug_sendInteger(-1);
                    continue;
                }

                current_stack_item = &stack[depth];
                LuaDebug_sendInteger(1);
                continue;
            }

            if (cmd == "get stack item currentline") {
                if (current_stack_item) {
                    LuaDebug_sendInteger(current_stack_item->currentline);
                }
                else {
                    LuaDebug_sendInteger(-1);
                }
                continue;
            }
            if (cmd == "get stack item short_src") {
                if (current_stack_item && current_stack_item->short_src) {
                    LuaDebug_sendString(current_stack_item->short_src);
                }
                else {
                    LuaDebug_sendString("");
                }
                continue;
            }
            if (cmd == "get stack item source") {
                if (current_stack_item && current_stack_item->source) {
                    LuaDebug_sendString(current_stack_item->source);
                }
                else {
                    LuaDebug_sendString("");
                }
                continue;
            }
            if (cmd == "get stack item linedefined") {
                if (current_stack_item) {
                    LuaDebug_sendInteger(current_stack_item->linedefined);
                }
                else {
                    LuaDebug_sendInteger(-1);
                }
                continue;
            }
            if (cmd == "get stack item what") {
                if (current_stack_item && current_stack_item->what) {
                    LuaDebug_sendString(current_stack_item->what);
                }
                else {
                    LuaDebug_sendString("");
                }
                continue;
            }
            if (cmd == "get stack item name") {
                if (current_stack_item && current_stack_item->name) {
                    LuaDebug_sendString(current_stack_item->name);
                }
                else {
                    LuaDebug_sendString("");
                }
                continue;
            }
            if (cmd == "get stack item namewhat") {
                if (current_stack_item && current_stack_item->namewhat) {
                    LuaDebug_sendString(current_stack_item->namewhat);
                }
                else {
                    LuaDebug_sendString("");
                }
                continue;
            }
            if (cmd == "get stack item nups") {
                if (current_stack_item) {
                    LuaDebug_sendInteger(current_stack_item->nups);
                }
                else {
                    LuaDebug_sendInteger(-1);
                }
                continue;
            }
            if (cmd == "get nil to current value") {
                lua_pushnil(L);
                LuaDebug_sendInteger(1);
                continue;
            }
            if (cmd == "get local to current value") {
                int i = -1;
                LuaDebug_recvInteger(i);

                if (current_stack_item) {
                    LuaDebug_sendString(lua_getlocal(L, current_stack_item, i));
                }
                else {
                    // we send 0 as head because Lua identifier won't begin with a digit
                    LuaDebug_sendString("0NULL");
                }
                continue;
            }
            if (cmd == "get upvalue to current value") {
                int i = -1;
                LuaDebug_recvInteger(i);

                if (current_stack_item) {
                    if (0 != lua_getinfo(L, "f", current_stack_item)) {
                        std::string result = lua_getupvalue(L, -1, i);

                        // lua_getinfo("f") would push the function onto stack
                        if (result != "0NULL") {
                            lua_remove(L, -2);
                        }
                        else {
                            lua_pop(L, 1);
                        }

                        LuaDebug_sendString(result);
                    }
                    else {
                        // we send 0 as head because Lua identifier won't begin with a digit
                        LuaDebug_sendString("0NULL");
                    }
                }
                else {
                    // we send 0 as head because Lua identifier won't begin with a digit
                    LuaDebug_sendString("0NULL");
                }
                continue;
            }
            if (cmd == "discard current value") {
                lua_pop(L, 1);
                LuaDebug_sendInteger(1);
                continue;
            }
            if (cmd == "discard specific value") {
                int index = -1;
                LuaDebug_recvInteger(index);

                lua_remove(L, index);

                LuaDebug_sendInteger(1);
                continue;
            }
            if (cmd == "get current value type") {
                LuaDebug_sendInteger(lua_type(L, -1));
                continue;
            }
            if (cmd == "get current value as debug string") {
                LuaDebug_sendString(lua_todebugstring(L, -1));
                continue;
            }
            if (cmd == "get table key as debug string") {
                lua_pushvalue(L, -2);
                LuaDebug_sendString(lua_todebugstring(L, -1));
                lua_pop(L, 1);
                continue;
            }
            if (cmd == "get next table item") {
                LuaDebug_sendInteger(lua_next(L, -2));
                continue;
            }
            if (cmd == "get next global item") {
                LuaDebug_sendInteger(lua_next(L, LUA_GLOBALSINDEX));
                continue;
            }
            if (cmd == "get function environment") {
                if (current_stack_item) {
                    if (0 != lua_getinfo(L, "f", current_stack_item)) {
                        lua_getfenv(L, -1);

                        // lua_getinfo("f") would push the function onto stack
                        lua_remove(L, -2);

                        LuaDebug_sendInteger(1);
                    }
                    else {
                        LuaDebug_sendInteger(0);
                    }
                }
                else {
                    LuaDebug_sendInteger(0);
                }
                continue;
            }

            throw std::exception("Unknown debug command");
        }
    }
    catch (std::exception&) {
        LuaDebug_end(L);
    }
}



int LuaDebug_breakpoint() {
    continueTillBreakpoint = false;

    void* L = GetContext();

    if (debuggerSocket == INVALID_SOCKET) {
        struct addrinfo hints = {}, * result = NULL;
        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        if (0 != getaddrinfo(NULL, "2323", &hints, &result)) {
            lua_sethook(L, &LuaDebug_hook, 0, 0);
            return -1;
        }

        SOCKET s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (s == INVALID_SOCKET) {
            lua_sethook(L, &LuaDebug_hook, 0, 0);
            freeaddrinfo(result);
            return -2;
        }

        // Disable Nagle
        DWORD disableNagle = 1u;
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&disableNagle), sizeof(disableNagle));

        // Enable TCP keep-alive so that we would timeout when debugger die
        DWORD nouse = 0;
        tcp_keepalive keepalive = {};
        keepalive.onoff = 1;
        keepalive.keepalivetime = 5000;
        keepalive.keepaliveinterval = 1000;
        WSAIoctl(s, SIO_KEEPALIVE_VALS, &keepalive, sizeof(keepalive), NULL, 0, &nouse, NULL, NULL);

        if (0 != connect(s, result->ai_addr, result->ai_addrlen)) {
            lua_sethook(L, &LuaDebug_hook, 0, 0);
            freeaddrinfo(result);
            closesocket(s);
            return -3;
        }

        debuggerSocket = s;
        freeaddrinfo(result);
    }

    if (NULL == lua_gethook(L)) {
        lua_sethook(L, &LuaDebug_hook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
    }

    return 1;
}

void LuaDebug_end(void* L) {
    if (L == NULL) {
        L = GetContext();
    }

    if (lua_gethook(L)) {
        lua_sethook(L, NULL, 0, 0);
    }

    if (debuggerSocket != INVALID_SOCKET) {
        closesocket(debuggerSocket);
        debuggerSocket = INVALID_SOCKET;
    }
}

int lua_getstack(void* L, int level, lua_Debug* ar) {
    typedef int(__fastcall* LUA_GETSTACK)(void*, int, lua_Debug*);
    auto p_lua_getstack = reinterpret_cast<LUA_GETSTACK>(0x6fbaa0);

    return p_lua_getstack(L, level, ar);
}

int lua_getinfo(void* L, std::string what, lua_Debug* ar) {
    typedef int(__fastcall* LUA_GETINFO)(void*, const char*, lua_Debug*);
    auto p_lua_getinfo = reinterpret_cast<LUA_GETINFO>(0x6fbc70);

    return p_lua_getinfo(L, what.data(), ar);
}

std::string lua_getlocal(void* L, const lua_Debug* ar, int n) {
    typedef const char* (__fastcall* LUA_GETLOCAL)(void*, const lua_Debug*, int);
    auto p_lua_getlocal = reinterpret_cast<LUA_GETLOCAL>(0x6fbb20);

    const char* p_result = p_lua_getlocal(L, ar, n);
    if (p_result == NULL) {
        // we send 0 as head because Lua identifier won't begin with a digit
        return "0NULL";
    }

    std::string result{ p_result };
    return result;
}

std::string lua_setlocal(void* L, const lua_Debug* ar, int n) {
    typedef const char* (__fastcall* LUA_SETLOCAL)(void*, const lua_Debug*, int);
    auto p_lua_setlocal = reinterpret_cast<LUA_SETLOCAL>(0x6fbbb0);

    const char* p_result = p_lua_setlocal(L, ar, n);
    if (p_result == NULL) {
        // we send 0 as head because Lua identifier won't begin with a digit
        return "0NULL";
    }

    std::string result{ p_result };
    return result;
}

std::string lua_getupvalue(void* L, int funcindex, int n) {
    typedef const char* (__fastcall* LUA_GETUPVALUE)(void*, int, int);
    auto p_lua_getupvalue = reinterpret_cast<LUA_GETUPVALUE>(0x6f4660);

    const char* p_result = p_lua_getupvalue(L, funcindex, n);
    if (p_result == NULL) {
        // we send 0 as head because Lua identifier won't begin with a digit
        return "0NULL";
    }

    std::string result{ p_result };
    return result;
}

std::string lua_setupvalue(void* L, int funcindex, int n) {
    typedef const char* (__fastcall* LUA_SETUPVALUE)(void*, int, int);
    auto p_lua_setupvalue = reinterpret_cast<LUA_SETUPVALUE>(0x6f47b0);

    const char* p_result = p_lua_setupvalue(L, funcindex, n);
    if (p_result == NULL) {
        // we send 0 as head because Lua identifier won't begin with a digit
        return "0NULL";
    }

    std::string result{ p_result };
    return result;
}

int lua_sethook(void* L, lua_Hook func, int mask, int count) {
    typedef int(__fastcall* LUA_SETHOOK)(void*, lua_Hook, int, int);
    auto p_lua_sethook = reinterpret_cast<LUA_SETHOOK>(0x6fba40);

    return p_lua_sethook(L, func, mask, count);
}

lua_Hook lua_gethook(void* L) {
    uint32_t ptr = reinterpret_cast<uint32_t>(L);
    return *reinterpret_cast<lua_Hook*>(ptr + 0x3c);
}

int lua_gethookmask(void* L) {
    uint32_t ptr = reinterpret_cast<uint32_t>(L);

    // Lua saves hookmask as a byte, but lua_gethookmask return an int
    return *reinterpret_cast<uint8_t*>(ptr + 0x30);
}

void lua_getfenv(void* L, int index) {
    typedef void(__fastcall* LUA_GETFENV)(void*, int);
    auto p_lua_getfenv = reinterpret_cast<LUA_GETFENV>(0x6F3D50);

    p_lua_getfenv(L, index);
    return;
}

int lua_setfenv(void* L, int index) {
    typedef int(__fastcall* LUA_SETFENV)(void*, int);
    auto p_lua_setfenv = reinterpret_cast<LUA_SETFENV>(0x6F40D0);

    return p_lua_setfenv(L, index);
}

std::string lua_todebugstring(void* L, int index) {
    typedef void* (__fastcall* LUA_TOPOINTER)(void*, int);
    auto p_lua_topointer = reinterpret_cast<LUA_TOPOINTER>(0x6F3790);

    std::stringstream ss{};
    int type = lua_type(L, index);

    std::string strType = lua_typename(L, type);
    if (strType.length() > 0) {
        ss << "(" << strType << ")";
    }
    else {
        ss << "(" << "Invalid type" << ")";
    }

    switch (type) {
    case LUA_TFUNCTION:
    case LUA_TTABLE:
    case LUA_TTHREAD:
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(p_lua_topointer(L, index));
        ss << "0x" << std::uppercase << std::hex << ptr;
    }
    break;
    case LUA_TBOOLEAN:
    {
        if (lua_toboolean(L, index)) {
            ss << "True";
        }
        else {
            ss << "False";
        }
    }
    break;
    case LUA_TNUMBER:
    {
        ss << lua_tonumber(L, index);
    }
    break;
    case LUA_TSTRING:
    {
        ss << lua_tostring(L, index);
    }
    break;
    case LUA_TNIL:
    {
        ss << "nil";
    }
    break;
    default:
    {
        ss << "Invalid";
    }
    }

    return ss.str();
}
