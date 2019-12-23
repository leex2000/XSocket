#ifndef __XSocket__
#define __XSocket__

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h> 
#define SOCKET int
#endif

#define CONNECT_TIMEOUT	5000		// 5 秒
#define SEND_TIMEOUT	10000		// 10 秒
#define RECV_TIMEOUT	10000		// 10 秒

void show_error(int errcode);

// 返回值 > 0 表示正确，0 表示超时，< 0 表示错误
class XSocket
{
public:
	SOCKET hsocket;

	XSocket();
	~XSocket();

	int open(const char* address, int port, int timeout = CONNECT_TIMEOUT);
	void close();
	bool is_open();
	int send(const void* buffer, int length, int timeout = SEND_TIMEOUT);
	int send(const char* buffer, int timeout = SEND_TIMEOUT);
	int recv(void* buffer, int length, int timeout = RECV_TIMEOUT);
	int listen(int port);
	int accept(XSocket& newsocket);

private:
	int get_error_code();
	bool is_pending(int errcode);
	int wait(int& status, int timeout);
};

#endif	// __XSocket__
