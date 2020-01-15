#pragma once

#include "xsocket.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time            time
#define mbedtls_time_t          time_t
#define mbedtls_fprintf         fprintf
#define mbedtls_printf          printf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif // MBEDTLS_PLATFORM_C

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

// Return: > 0 success，= 0 timeout，< 0 error
class TlsSocket : public XSocket
{
public:
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cacert;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;

	TlsSocket();
	~TlsSocket();

	int open(const char* address, int port, int timeout = CONNECT_TIMEOUT);
	void close();
	int send(const void* buffer, int length, int timeout = SEND_TIMEOUT);
	int send(const char* buffer, int timeout = SEND_TIMEOUT);
	int recv(void* buffer, int length, int timeout = RECV_TIMEOUT);

	void set_cert(const char* cert_pem);
	int get_server_cert(char* buffer, int length);

private:
	char* cert_pem;
	static int ssl_send(void *ctx, const unsigned char *buffer, size_t length);
	static int ssl_recv(void *ctx, unsigned char *buffer, size_t length);
};

