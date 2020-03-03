#include "tls_socket.h"
#include <string.h>
#include <functional>

#undef MBEDTLS_DEBUG_C

#if defined(MBEDTLS_DEBUG_C)

#define DEBUG_LEVEL 1
static void my_debug(void *ctx, int level, const char *file, int line, const char *str)
{
	((void)level);

	mbedtls_fprintf((FILE *)ctx, "%s:%04d: %s", file, line, str);
	fflush((FILE *)ctx);
}
#endif

TlsSocket::TlsSocket()
{
#if defined(MBEDTLS_DEBUG_C)
	mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

	const char *pers = "tls_client";

	// Initialize the RNG and the session data
	cert_pem = nullptr;
	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
	mbedtls_x509_crt_init(&cacert);
	mbedtls_ctr_drbg_init(&ctr_drbg);

	// Seeding the random number generator...
	mbedtls_entropy_init(&entropy);

	int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers));
	if (ret != 0)
		mbedtls_printf("Failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
}

TlsSocket::~TlsSocket()
{
	mbedtls_x509_crt_free(&cacert);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
}

int TlsSocket::open(const char* address, int port, int timeout)
{
	int ret;

	if (cert_pem)
	{
		ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)cert_pem, strlen(cert_pem)+1);
		if (ret < 0)
			return ret;
	}

	// 1. Start the connection
	ret = XSocket::open(address, port, timeout);
	if (ret <= 0)
		return ret;

	// 2. Setup stuff

	// Setting up the SSL/TLS structure...
	if ((ret = mbedtls_ssl_config_defaults(&conf, 
		MBEDTLS_SSL_IS_CLIENT,
		MBEDTLS_SSL_TRANSPORT_STREAM,
		MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
		return ret;

	// OPTIONAL is not optimal for security, but makes interop easier in this simplified example
	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
	mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

#if defined(MBEDTLS_DEBUG_C)
	mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);
#endif

	if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
		return ret;

	if ((ret = mbedtls_ssl_set_hostname(&ssl, address)) != 0)
		return ret;

	mbedtls_ssl_set_bio(&ssl, this, ssl_send, ssl_recv, NULL);

	// 4. Handshake
	while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
			return ret;
	}

	// 5. Verify the server X.509 certificate
	if (cert_pem)
	{
		// In real life, we probably want to bail out when ret != 0
		uint32_t flags;
		if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
		{
			char vrfy_buf[512];

			mbedtls_printf(" failed\n");
			mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
			mbedtls_printf("%s\n", vrfy_buf);
			return flags;
		}
	}
	
	return 1;
}

void TlsSocket::close()
{
	if (is_open())
	{
		mbedtls_ssl_close_notify(&ssl);
		XSocket::close();
	}
}

// > 0 means send len
int TlsSocket::send(const void* buffer, int length, int timeout)
{
	int ret;
	while ((ret = mbedtls_ssl_write(&ssl, (const unsigned char*)buffer, length)) <= 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
			return ret;
	}

	return ret;
}

// > 0 means send len
int TlsSocket::send(const char* buffer, int timeout)
{
	return send(buffer, (int)strlen(buffer), timeout);
}

// > 0 means recv len
int TlsSocket::recv(void* buffer, int length, int timeout)
{
	int ret;

	do
	{
		ret = mbedtls_ssl_read(&ssl, (unsigned char *)buffer, length);
		printf("%d\n", ret);
	} while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

	if (ret <= 0)
	{
		if ((ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) || (ret == 0))	// EOF
			mbedtls_ssl_close_notify(&ssl);
	}

	if (ret < length)
		*((unsigned char*)buffer+ret) = 0;

	return ret;
}

int TlsSocket::get_server_cert(char* buffer, int length)
{
	// get information about the TLS connection
	return mbedtls_x509_crt_info(buffer, length, "\r  ", mbedtls_ssl_get_peer_cert(&ssl));
	//mbedtls_printf("Server certificate:\n%s\n", buffer);
}

// call before open
void TlsSocket::set_cert(const char* cert_pem)
{
	if (cert_pem)
		this->cert_pem = STRDUP(cert_pem);
}

int TlsSocket::ssl_send(void *ctx, const unsigned char *buffer, size_t length)
{
	XSocket* This = static_cast<XSocket*>(ctx);
	return This->send(buffer, (int)length);
}

int TlsSocket::ssl_recv(void *ctx, unsigned char *buffer, size_t length)
{
	XSocket* This = static_cast<XSocket*>(ctx);
	return This->recv(buffer, (int)length);
}
