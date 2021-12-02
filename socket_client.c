#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h> //programm also works without this header file
#include <arpa/inet.h>

#define REQUEST_LEN 19
#define RESPONSE_LEN 1024

int main()
{

    // declaring and initialization of variables if required
    int sock, connection_status;
    struct sockaddr_in server_address;
    char *server_ip = "142.251.33.68+";
    char request[REQUEST_LEN] = "GET / HTTP/1.0\r\n\r\n";
    char server_response[RESPONSE_LEN];

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
    send(sock, request, REQUEST_LEN, 0);
    printf("[request sended to server]\n\n");

    // receving response from server
    recv(sock, server_response, RESPONSE_LEN, 0);
    printf("[response recevied from server]\n\n");

    // printing the server response
    printf("%s", server_response);

    // closing socket
    close(sock);

    return 0;
}