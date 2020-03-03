#ifndef __XSocket__
#define __XSocket__

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define STRDUP _strdup
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define Sleep(msec) usleep(msec*1000)
#define closesocket ::close
#define STRDUP strdup
#endif

#define CONNECT_TIMEOUT	5000		// 5 sec
#define SEND_TIMEOUT	10000		// 10 sec
#define RECV_TIMEOUT	10000		// 10 sec

void show_error(int errcode);

// Return: > 0 success，= 0 timeout，< 0 error
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
