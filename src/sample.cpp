#include <stdio.h>
#include "xsocket.h"

void test()
{
	XSocket xs;
	char buffer[4097];
	int ret;

	buffer[4096] = 0;
	ret = xs.open("www.baidu.com", 80);
	//ret = xs.open("192.168.130.8", 8280);
	if (ret <= 0)
	{
		show_error(ret);
		return;
	}

	ret = xs.send("GET / HTTP/1.1\n\n");
	if (ret <= 0)
	{
		show_error(ret);
		return;
	}

	while (1)
	{
		ret = xs.recv(buffer, 4096);
		if (ret < 0)
		{
			show_error(ret);
			return;
		}
		printf("[%d] %s\n", ret, buffer);
	}
}

// XSocket /s
void do_server(int port)
{
	int ret;
	char buffer[4096];
	XSocket server, client;

	while (1)
	{
		ret = server.listen(port);
		if (ret <= 0)
		{
			show_error(ret);
			return;
		}
		printf("Listen %d\n", port);

		ret = server.accept(client);
		if (ret <= 0)
		{
			show_error(ret);
			return;
		}

		while (1)
		{
			ret = client.recv(buffer, 4096);
			if (ret < 0)
			{
				show_error(ret);
				client.close();
				break;
			}

			printf("RECV[%d]: %s\n", ret, buffer);
			strcat_s(buffer, 4096, " <<<");
			ret = client.send(buffer);
			if (ret <= 0)
			{
				show_error(ret);
				client.close();
				break;
			}
			printf("SEND: %s\n", buffer);
		}
	}
}

// XSocket
void do_client(char* address, int port)
{
	int ret;
	char buffer[4096];

	XSocket client;
	ret = client.open(address, port);
	if (ret <= 0)
	{
		show_error(ret);
		return;
	}

	for (int i = 0; i < 1000; i++)
	{
		snprintf(buffer, 4096, "%d.%d.%d.%d", i, i, i, i);
		ret = client.send(buffer);
		if (ret <= 0)
		{
			show_error(ret);
			return;
		}

		Sleep(500);
		printf("SEND: %s\n", buffer);

		ret = client.recv(buffer, 4096);
		if (ret < 0)
		{
			show_error(ret);
			return;
		}
		printf("RECV [%d]: %s\n", ret, buffer);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Usage: xsocket /s port					Server Mode\n");
		printf("Usage: xsocket server_address port		Client Mode\n");
		return 0;
	}

	int port = atoi(argv[2]);
	if (strcmp(argv[1], "/s") == 0)
	{
		do_server(port);
	}
	else if (argc == 1)
	{
		do_client(argv[1], port);
	}

	return 0;
}