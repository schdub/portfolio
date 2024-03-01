#pragma once

#ifdef WIN32
  #include <winsock.h>
  #pragma comment(lib,"ws2_32.lib")
  #define CLOSE_SOCKET(sd) ::shutdown(sd, 2), ::closesocket(sd)
  #define socklen_t        int  
  #define NETINIT          { WSADATA wd; if (WSAStartup(0x202, &wd)) { WARN("WSAStartup error %d", WSAGetLastError()); return -1; } }
  #define IS_EAGAIN        (WSAGetLastError() == WSAETIMEDOUT)
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <errno.h>
  #include <unistd.h>
  #define INVALID_SOCKET   (-1)
  #define SOCKET_ERROR     (-1)
  #define CLOSE_SOCKET(sd) ::shutdown(sd, 2), ::close(sd)
  #define SOCKET           int
  #define NETINIT
  #define IS_EAGAIN        (errno == EAGAIN)
#endif // !WIN32

#if (DEBUG_ENABLED == 1)
  #include "debug.hpp"
#else
  #define LOG(m, ...)   ((void)0)
  #define LOGINIT(n, l) ((void)0)
  #define LOGUNINIT()   ((void)0)
  #define LOG(m, ...)   ((void)0)
  #define WARN(m, ...)  ((void)0)
  #define HEXDUMP(m,b,s)((void)0)
  #define LOGCTX
#endif

#include <cstring> // memset

namespace op {

class TCPSocket {
public:
    explicit TCPSocket(SOCKET sd) : mSocket(sd) {}

    explicit TCPSocket(const char * host, unsigned port) : mSocket(INVALID_SOCKET) {
        mSocket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (mSocket == INVALID_SOCKET) {
            WARN("socket() err='%d'", get_lasterror());
            return;
        }
        sockaddr_in addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(port);
        addr.sin_addr.s_addr = ::inet_addr(host);
        if (::connect(mSocket, (sockaddr*) &addr, sizeof(addr)) < 0) {
            WARN("connect() err='%d'", get_lasterror());
            CLOSE_SOCKET(mSocket);
            mSocket = INVALID_SOCKET;
            return;
        }
    }

    ~TCPSocket() {
        CLOSE_SOCKET(mSocket);
    }

    SOCKET sd() const { return mSocket; };

    bool isOk() const { return mSocket != INVALID_SOCKET; }

    bool isEAGAIN() const { return IS_EAGAIN; }

    static int write_all(SOCKET sd, const char * buff, int count) {
        int total = 0;
        for (int iret = 0; total < count;) {
            iret = send(sd, buff + total, count - total, 0);
            if (iret <= 0) {
                WARN("send() err='%d'", get_lasterror());
                total = iret;
                break;
            }
            total += iret;
        }
        return total;
    }

    int recvthis(char * buff, int count) {
        return recv(mSocket, buff, count, 0);
    }

    int write_all(const char * buff, int count) {
        return write_all(mSocket, buff, count);
    }

    static int read_all(SOCKET sd, char * buff, int count) {
        int total = 0;
        for (int iret = 0; total < count;) {
            iret = recv(sd, buff + total, count - total, 0);
            if (iret <= 0) {
                WARN("recv() sd=%d err='%d'", sd, get_lasterror());
                total = iret;
                break;
            }
            total += iret;
        }
        return total;
    }

    int read_all(char * buff, int count) {
        return read_all(mSocket, buff, count);
    }
    static void gracefulclose(SOCKET sd) {
        CLOSE_SOCKET(sd);
    }

    static int get_lasterror() {
		#if WIN32
			return WSAGetLastError();
		#else
			return errno;
		#endif
    }

    static void set_keepalive(SOCKET sd, bool value) {
        const int optval = value ? 1 : 0;
        ::setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof(optval));
    }
    static void set_reuseaddr(SOCKET sd, bool value) {
        const int optval = value ? 1 : 0;
        ::setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
    }
    static void set_rcvtimeo(SOCKET sd, unsigned sec = 5, unsigned msec = 0) {
        timeval optval;
        optval.tv_sec  = sec;
        optval.tv_usec = msec;
        ::setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&optval, sizeof(optval));
    }

private:
    SOCKET mSocket;

    TCPSocket(const TCPSocket &);
    TCPSocket operator= (const TCPSocket &);
};

class TCPServer {
public:
    explicit TCPServer(unsigned port) { mIsOk = listenPort(port); }
    virtual ~TCPServer() {}
    virtual void incomingConnection(SOCKET sd, sockaddr * sa, socklen_t sa_len) = 0;

    // no errors?
    bool isOk() { return mIsOk; }

    // main server loop
    void loop() {
        if (!mIsOk) {
            WARN("can't enter server loop");
            return;
        }

        LOG("+LOOP");

        SOCKET      in_socket;
        sockaddr_in in_addr;
        socklen_t   in_addr_len;
        for (;;) {
            if (listen(mSrvSocket , 10) == SOCKET_ERROR) {
                WARN("listen() err='%d'", TCPSocket::get_lasterror());
                break;
            }
            in_addr_len = sizeof(in_addr);
            in_socket   = ::accept(mSrvSocket, (sockaddr*) &in_addr, &in_addr_len);
            if (in_socket <= 0) {
                WARN("accept() err='%d'", TCPSocket::get_lasterror());
                break;
            }
            LOG("accept() ip='%s'", inet_ntoa(in_addr.sin_addr));
            this->incomingConnection(in_socket, (sockaddr*) &in_addr, in_addr_len);
        }

        CLOSE_SOCKET(mSrvSocket);

        LOG("-LOOP");
    }

protected:
    SOCKET mSrvSocket;
    bool mIsOk;

    bool listenPort(unsigned port, bool reuseaddr = false) {
        bool bRet = false;
        mSrvSocket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (mSrvSocket  == INVALID_SOCKET) {
            WARN("socket() err='%d'", TCPSocket::get_lasterror());
        } else {
            sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family      = AF_INET;
            addr.sin_port        = htons(port);
            addr.sin_addr.s_addr = INADDR_ANY;

            if (::bind(mSrvSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
                CLOSE_SOCKET(mSrvSocket);
                WARN("bind() err='%d'", TCPSocket::get_lasterror());
            } else {
                TCPSocket::set_reuseaddr(mSrvSocket, reuseaddr);
                bRet = true;
            }
        }
        return bRet;
    }
};

} // namespace op
