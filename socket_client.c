#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define REQUEST_LEN 19
#define RESPONSE_LEN 1024

int main()
{

	// declaring and initialization of variables if required
	int sock, connection_status;
	int recv_len = 0, total_recv_len = 0;
	struct sockaddr_in server_address;
	char *server_ip = "54.204.94.184";
	char request[REQUEST_LEN] = "GET / HTTP/1.0\r\n\r\n";
	char server_response[RESPONSE_LEN + 1];
	server_response[1024] = '\0'; // null character at the end of buffer so that output will not get overlapped

	// creating a socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// checking socket created or not
	if (sock == -1)
		printf("[ERROR] unable to create socket");

	// setting address of server to connect socket
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(80);
	server_address.sin_addr.s_addr = inet_addr(server_ip);

	// connecting socket
	connection_status = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));

	// checking connection status
	if (connection_status == -1)
		printf("[ERROR] connection failed");

	// sending request to server
	if (send(sock, request, REQUEST_LEN, 0) < 0)
	{
		printf("[ERROR] failed to send request");
	}
	printf("[request sended to server]\n\n");

	// receving response from server
	while ((recv_len = recv(sock, server_response, RESPONSE_LEN, 0) > 0))
	{
		printf("%s", server_response);
		memset(server_response, '\0', RESPONSE_LEN);
		total_recv_len += recv_len;
	}

	// closing socket
	close(sock);

	return 0;
}
