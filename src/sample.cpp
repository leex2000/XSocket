#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xsocket.h"
#include "tls_socket.h"

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
			strcat(buffer, " <<<");
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

const char cert_pem[] =
	// baidu CA
	// GlobalSign Organization Calidation CA - SHA256 - G2
	"-----BEGIN CERTIFICATE-----\r\n" \
	"MIIEaTCCA1GgAwIBAgILBAAAAAABRE7wQkcwDQYJKoZIhvcNAQELBQAwVzELMAkG\r\n" \
	"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n" \
	"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw0xNDAyMjAxMDAw\r\n" \
	"MDBaFw0yNDAyMjAxMDAwMDBaMGYxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n" \
	"YWxTaWduIG52LXNhMTwwOgYDVQQDEzNHbG9iYWxTaWduIE9yZ2FuaXphdGlvbiBW\r\n" \
	"YWxpZGF0aW9uIENBIC0gU0hBMjU2IC0gRzIwggEiMA0GCSqGSIb3DQEBAQUAA4IB\r\n" \
	"DwAwggEKAoIBAQDHDmw/I5N/zHClnSDDDlM/fsBOwphJykfVI+8DNIV0yKMCLkZc\r\n" \
	"C33JiJ1Pi/D4nGyMVTXbv/Kz6vvjVudKRtkTIso21ZvBqOOWQ5PyDLzm+ebomchj\r\n" \
	"SHh/VzZpGhkdWtHUfcKc1H/hgBKueuqI6lfYygoKOhJJomIZeg0k9zfrtHOSewUj\r\n" \
	"mxK1zusp36QUArkBpdSmnENkiN74fv7j9R7l/tyjqORmMdlMJekYuYlZCa7pnRxt\r\n" \
	"Nw9KHjUgKOKv1CGLAcRFrW4rY6uSa2EKTSDtc7p8zv4WtdufgPDWi2zZCHlKT3hl\r\n" \
	"2pK8vjX5s8T5J4BO/5ZS5gIg4Qdz6V0rvbLxAgMBAAGjggElMIIBITAOBgNVHQ8B\r\n" \
	"Af8EBAMCAQYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUlt5h8b0cFilT\r\n" \
	"HMDMfTuDAEDmGnwwRwYDVR0gBEAwPjA8BgRVHSAAMDQwMgYIKwYBBQUHAgEWJmh0\r\n" \
	"dHBzOi8vd3d3Lmdsb2JhbHNpZ24uY29tL3JlcG9zaXRvcnkvMDMGA1UdHwQsMCow\r\n" \
	"KKAmoCSGImh0dHA6Ly9jcmwuZ2xvYmFsc2lnbi5uZXQvcm9vdC5jcmwwPQYIKwYB\r\n" \
	"BQUHAQEEMTAvMC0GCCsGAQUFBzABhiFodHRwOi8vb2NzcC5nbG9iYWxzaWduLmNv\r\n" \
	"bS9yb290cjEwHwYDVR0jBBgwFoAUYHtmGkUNl8qJUC99BM00qP/8/UswDQYJKoZI\r\n" \
	"hvcNAQELBQADggEBAEYq7l69rgFgNzERhnF0tkZJyBAW/i9iIxerH4f4gu3K3w4s\r\n" \
	"32R1juUYcqeMOovJrKV3UPfvnqTgoI8UV6MqX+x+bRDmuo2wCId2Dkyy2VG7EQLy\r\n" \
	"XN0cvfNVlg/UBsD84iOKJHDTu/B5GqdhcIOKrwbFINihY9Bsrk8y1658GEV1BSl3\r\n" \
	"30JAZGSGvip2CTFvHST0mdCF/vIhCPnG9vHQWe3WVjwIKANnuvD58ZAWR65n5ryA\r\n" \
	"SOlCdjSXVWkkDoPWoC209fN5ikkodBpBocLTJIg1MGCUF7ThBCIxPTsvFwayuJ2G\r\n" \
	"K1pp74P1S8SqtCr4fKGxhZSM9AyHDPSsQPhZSZg=\r\n" \
	"-----END CERTIFICATE-----\r\n"	\
	// GlobalSign Root CA - R1
	"-----BEGIN CERTIFICATE-----\r\n" \
	"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n" \
	"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n" \
	"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n" \
	"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n" \
	"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n" \
	"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n" \
	"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n" \
	"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n" \
	"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n" \
	"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n" \
	"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n" \
	"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n" \
	"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n" \
	"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n" \
	"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n" \
	"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n" \
	"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n" \
	"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n" \
	"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n" \
	"-----END CERTIFICATE-----\r\n";

void tls_test()
{
	TlsSocket ts;
	char buffer[4097];
	int ret;

	ts.set_cert(cert_pem);
	ret = ts.open("www.baidu.com", 443);
	if (ret <= 0)
	{
		show_error(ret);
		return;
	}

	ret = ts.send("GET / HTTP/1.1\n\n");
	if (ret <= 0)
	{
		show_error(ret);
		return;
	}

	while (1)
	{
		ret = ts.recv(buffer, 4096);
		if (ret < 0)
		{
			show_error(ret);
			return;
		}
		if (ret == 0)
			break;
		//printf("[%d] %s\n", ret, buffer);
	}
}

int main(int argc, char* argv[])
{
	/*
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
	else
	{
		do_client(argv[1], port);
	}
	*/
	tls_test();

	return 0;
}