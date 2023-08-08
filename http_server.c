#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define RESPONSE_LEN 118
#define CLIENT_REQUEST_LEN 1024

int sock;
pthread_t threads[3];

void sigint_handler(int signal)
{
	printf("\nWaiting for thread termination\n");
	for (int i = 0; i < 3; i++)
		pthread_join(threads[i], NULL);

	close(sock);
	printf("[SIGINT] Closing socket\n");
	exit(0);
}

void *request_handler(void *client)
{
	// decalring buffers for request and response
	char client_request[CLIENT_REQUEST_LEN + 1];
	client_request[1024] = '\0';
	char response[RESPONSE_LEN] = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\nContent-Length: 13\nConnection: close\n\nHello World!\r\n\r\n";

	// getting client socket descriptor from void * pointer in argument
	int client_fd = *(int *)client;
	printf("[REQUEST]\n");

	// printing out request to the console
	while (recv(client_fd, client_request, CLIENT_REQUEST_LEN, 0) > 0)
	{
		printf("%s", client_request);
		memset(client_request, '\0', CLIENT_REQUEST_LEN);
	}

	// sending back response to the client
	send(client_fd, response, RESPONSE_LEN, 0);
	printf("[RESPONDED]\n\n");

	// closing the client socket this will close the tcp connection
	close(client_fd);
	free(client); // this will free the dynamically allocated memory

	return NULL;
}

int main()
{
	signal(SIGINT, sigint_handler); // closes socket on terminating program with SIGINT

	// declaring and initialization of variables if required
	int binding_status, client_fd;
	int client_sockaddr_len;
	struct sockaddr_in server_info, client;

	// creating a socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// checking socket created or not
	if (sock == -1)
		printf("[ERROR] unable to create socket");

	// setting address of server to connect socket
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(8000);
	server_info.sin_addr.s_addr = INADDR_ANY;

	// binding socket
	binding_status = bind(sock, (struct sockaddr *)&server_info, sizeof(server_info));

	// checking connection status
	if (binding_status == -1)
		printf("[ERROR] binding failed");

	// listening for incoming connections on the socket
	if (listen(sock, 3) < 0)
	{
		printf("[ERROR] failed to listen connections on the socket");
	}
	printf("[listening for incoming connections on the socket]\n\n");

	int current_index = 0;
	// receving response from server
	while ((client_fd = accept(sock, (struct sockaddr *)&client, (socklen_t *)&client_sockaddr_len)))
	{
		fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK); // this will make client socket non blocking mode so that it will not wait for data while we perform read() or recv() calls

		int *client_fd_ptr = malloc(1 * sizeof(int)); // dynamically creating a variable to store client_fd
		*client_fd_ptr = client_fd;

		// creating a new thread to handle the request and making main thread free so that it could respond to other tcp request
		if (pthread_create(&threads[current_index], NULL, request_handler, (void *)client_fd_ptr) < 0)
		{
			printf("[ERROR] failed to create thread");
			free(client_fd_ptr);
		}
		current_index = (current_index + 1) % 3;
	}

	// closing socket
	close(sock);

	return 0;
}
