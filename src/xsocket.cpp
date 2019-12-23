#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include "xsocket.h"

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

// ==========================================================================================

// Windows Socket 组件初始化类，自动加载和卸载 Windows Socket 组件
class Win32InitSocket
{
private:
	int result;

public:
	Win32InitSocket(LPWSADATA lpWSAData = nullptr, BYTE minorVersion = 2, BYTE majorVersion = 2)
	{
		struct WSAData wsa_data;
		result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	}

	~Win32InitSocket()
	{
		if (result == 0)
			WSACleanup();
	}
};

Win32InitSocket auto_init_win32_socket;

#endif

// ==========================================================================================

class XDns
{
public:
	addrinfo* info;

	XDns() : info(nullptr) {}
	~XDns()
	{
		if (info)
		{
			freeaddrinfo(info);
			info = nullptr;
		}
	}

	void dump()
	{
		if (!info)
			return;

		int i = 1;
		LPSOCKADDR sockaddr_ip;
		struct sockaddr_in* sockaddr_ipv4;
		DWORD ipbufferlength = 46;
		char ipstringbuffer[46];
		INT iRetval;

		for (struct addrinfo *ptr = info; ptr != NULL; ptr = ptr->ai_next)
		{

			printf("getaddrinfo response %d\n", i++);
			printf("\tFlags: 0x%x\n", ptr->ai_flags);
			printf("\tFamily: ");
			switch (ptr->ai_family)
			{
			case AF_UNSPEC:
				printf("Unspecified\n");
				break;
			case AF_INET:
				printf("AF_INET (IPv4)\n");
				sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
				printf("\tIPv4 address %s\n", inet_ntoa(sockaddr_ipv4->sin_addr));
				break;
			case AF_INET6:
				printf("AF_INET6 (IPv6)\n");
				//struct sockaddr_in* sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
				//printf("\tIPv6 address %s\n", InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

				// We use WSAAddressToString since it is supported on Windows XP and later
				sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
				// The buffer length is changed by each call to WSAAddresstoString
				// So we need to set it for each iteration through the loop for safety
				ipbufferlength = 46;
				iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL, ipstringbuffer, &ipbufferlength);
				if (iRetval)
					printf("WSAAddressToString failed with %u\n", WSAGetLastError());
				else
					printf("\tIPv6 address %s\n", ipstringbuffer);
				break;
			case AF_NETBIOS:
				printf("AF_NETBIOS (NetBIOS)\n");
				break;
			default:
				printf("Other %ld\n", ptr->ai_family);
				break;
			}

			printf("\tSocket type: ");
			switch (ptr->ai_socktype)
			{
			case 0:
				printf("Unspecified\n");
				break;
			case SOCK_STREAM:
				printf("SOCK_STREAM (stream)\n");
				break;
			case SOCK_DGRAM:
				printf("SOCK_DGRAM (datagram) \n");
				break;
			case SOCK_RAW:
				printf("SOCK_RAW (raw) \n");
				break;
			case SOCK_RDM:
				printf("SOCK_RDM (reliable message datagram)\n");
				break;
			case SOCK_SEQPACKET:
				printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
				break;
			default:
				printf("Other %ld\n", ptr->ai_socktype);
				break;
			}

			printf("\tProtocol: ");
			switch (ptr->ai_protocol)
			{
			case 0:
				printf("Unspecified\n");
				break;
			case IPPROTO_TCP:
				printf("IPPROTO_TCP (TCP)\n");
				break;
			case IPPROTO_UDP:
				printf("IPPROTO_UDP (UDP) \n");
				break;
			default:
				printf("Other %ld\n", ptr->ai_protocol);
				break;
			}
			printf("\tLength of this sockaddr: %d\n", (int)ptr->ai_addrlen);
			printf("\tCanonical name: %s\n", ptr->ai_canonname);
		}
	}

	bool getinfo(const char* address, int port)
	{
		addrinfo hints;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET;			// AF_UNSPEC
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = 0;					// AI_CANONNAME

		char sport[32];
		snprintf(sport, 32, "%d", port);

		if (getaddrinfo(address, sport, &hints, &info) != 0)
			return false;

		return true;
	}
};

// ==========================================================================================

#ifdef _WIN32
#include <windows.h>
#else
#include <string.h>
#endif

void show_error(int errcode)
{
	if (errcode == 0)
	{
		printf("Timeout\n");
		return;
	}

	errcode = -errcode;

#ifdef _WIN32
	char buffer[1024];

	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer), NULL) == 0)
		printf("[%d]: (错误未知)\n", errcode);
	else
		printf("[%d] %s\n", errcode, buffer);
#else
	printf("[%d] %s\n", errcode, strerror(errcode));
#endif
}

// ==========================================================================================

#define XSOCKET_READ		1
#define XSOCKET_WRITE		2
#define XSOCKET_EXCEPTION	4

XSocket::XSocket()
{
	hsocket = INVALID_SOCKET;
}

XSocket::~XSocket()
{
	close();
}

// 返回负值的错误码
int XSocket::get_error_code()
{
	int errcode;
#ifdef _WIN32
	errcode = WSAGetLastError();
#else
	errcode = errno;
#endif
	return -errcode;
}

// 是否已打开
bool XSocket::is_open()
{
	return(hsocket != INVALID_SOCKET);
}

// 是否处于延迟状态
bool XSocket::is_pending(int errcode)
{
	errcode = -errcode;

#ifdef _WIN32
	if (errcode == WSAEWOULDBLOCK)
		return true;
#else
	if (errcode == EINPROGRESS)
		return true;
#endif
	return false;
}

// 是否处于延迟状态
void XSocket::close()
{
	if (hsocket != INVALID_SOCKET)
	{
		closesocket(hsocket);
		hsocket = INVALID_SOCKET;
	}
}

int XSocket::listen(int port)
{
	//  1. 创建 socket
	close();
	hsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hsocket == INVALID_SOCKET)
		return get_error_code();

	// 2. 绑定端口
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(port);

	if (::bind(hsocket, (const sockaddr*)&saddr, sizeof(saddr)) < 0)
		return get_error_code();

	// 3. 监听
	if (::listen(hsocket, SOMAXCONN) < 0)
		return get_error_code();

	return 1;
}

int XSocket::accept(XSocket& newsocket)
{
	newsocket.close();
	newsocket.hsocket = ::accept(hsocket, NULL, NULL);		// 如果返回的是 INVALID_SOCKET, 不需要做什么
	if (newsocket.hsocket == INVALID_SOCKET)
		return get_error_code();

	return 1;
}

int XSocket::wait(int& status, int timeout)
{
	struct timeval tv_timeout;
	tv_timeout.tv_sec = timeout / 1000;
	tv_timeout.tv_usec = (timeout % 1000) * 1000;

	fd_set read_fds, write_fds, exception_fds;
	fd_set *pread_fds = nullptr, *pwrite_fds = nullptr, *pexception_fds = nullptr;

	if (status & XSOCKET_READ)
	{
		FD_ZERO(&read_fds);
		FD_SET(hsocket, &read_fds);
		pread_fds = &read_fds;
	}

	if (status & XSOCKET_WRITE)
	{
		FD_ZERO(&write_fds);
		FD_SET(hsocket, &write_fds);
		pwrite_fds = &write_fds;
	}

	if (status & XSOCKET_EXCEPTION)
	{
		FD_ZERO(&exception_fds);
		FD_SET(hsocket, &exception_fds);
		pexception_fds = &exception_fds;
	}

	int errcode = select((int)(hsocket + 1), pread_fds, pwrite_fds, pexception_fds, &tv_timeout);
	if (errcode < 0)
		return get_error_code();
	if (errcode == 0)
		return 0;

	int nstatus = 0;
	if (status & XSOCKET_READ)
		if (FD_ISSET(hsocket, &read_fds))
			nstatus |= XSOCKET_READ;

	if (status & XSOCKET_WRITE)
		if (FD_ISSET(hsocket, &write_fds))
			nstatus |= XSOCKET_WRITE;

	if (status & XSOCKET_EXCEPTION)
		if (FD_ISSET(hsocket, &exception_fds))
			nstatus |= XSOCKET_EXCEPTION;

	printf("status: %d -> %d\n", status, nstatus);

	int len = sizeof(int);
	getsockopt(hsocket, SOL_SOCKET, SO_ERROR, (char*)&errcode, (socklen_t *)&len);
	if (errcode != 0)
		return -errcode;

	status = nstatus;
	return 1;
}

// 返回 0 表示成功
int XSocket::open(const char* address, int port, int timeout)
{
	int errcode;

	//  1. 创建 socket
	close();
	hsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hsocket == INVALID_SOCKET)
		return get_error_code();

	// 2. dns 解析
	XDns xdns;
	if (!xdns.getinfo(address, port))
		return get_error_code();

	// 3. 设置为非阻塞模式
	int opt = 1;
#ifdef _WIN32
	if (ioctlsocket(hsocket, FIONBIO, (unsigned long *)&opt) < 0)
#else
	if (ioctl(hsocket, FIONBIO, &opt) < 0)
#endif
		return get_error_code();

	// 4. 连接
	if (::connect(hsocket, (const sockaddr *)xdns.info->ai_addr, sizeof(sockaddr)) < 0)
	{
		errcode = get_error_code();
		if (!is_pending(errcode))
			return errcode;

		int status = XSOCKET_WRITE;
		errcode = wait(status, timeout);
		if (errcode <= 0)
			return errcode;
	}

	// 连接成功
	return 1;
}

// 返回发送的长度
int XSocket::send(const void* buffer, int length, int timeout)
{
	int left, written;
	char* pbuf = (char*)buffer;
	left = length;

	while (left > 0)
	{
		int status = XSOCKET_WRITE;
		int errcode = wait(status, timeout);
		if (errcode <= 0)
			return errcode;

		written = ::send(hsocket, pbuf, left, 0);
		if (written < 0)
			return get_error_code();

		left -= written;
		pbuf += written;
	}

	return(length - left);
}

// 返回发送的长度，错误码用负值返回
int XSocket::send(const char* buffer, int timeout)
{
	return send(buffer, (int)strlen(buffer), timeout);
}

// 返回读取的长度，错误码用负值返回
int XSocket::recv(void* buffer, int length, int timeout)
{
	//int status = XSOCKET_READ;
	int status = XSOCKET_READ | XSOCKET_EXCEPTION;
	int errcode = wait(status, timeout);
	if (errcode <= 0)
		return errcode;

	int readed = ::recv(hsocket, (char*)buffer, length, 0);
	if (readed < 0)
		return get_error_code();

	if (readed < length)
		*(((char*)buffer + readed)) = 0;

	return(readed);
}
