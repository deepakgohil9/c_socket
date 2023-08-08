#include <iostream>
#include <vector>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define RECV_BUFF_LEN 1024

std::vector<int> clients;
std::atomic<bool> terminate(false);

inline void check(bool expr)
{
    if (!expr)
    {
        perror("[!ERROR]");
        exit(EXIT_FAILURE);
    }
}

void signal_handler(int signal)
{
    std::cout << "Program terminations starts: Please connect a new client in order to bypass blocking accept() and terminate the program\n";
    terminate = true;
}

void broker_routine()
{
    int recv_len;
    char recv_buff[RECV_BUFF_LEN + 1];
    recv_buff[1024] = '\0';

    while (!terminate)
    {
        for (int i = 0; i < clients.size(); i++)
        {
            while ((recv_len = recv(clients[i], recv_buff, 1024, 0)))
            {
                if (recv_len == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    break;
                else if (recv_len == -1)
                {
                    recv_len = 0;
                    break;
                }
                else
                {
                    for (int j = 0; j < clients.size(); j++) // broadcast
                    {
                        if (j == i)
                            continue;
                        send(clients[j], recv_buff, recv_len, 0);
                    }
                }
            }
            if (recv_len == 0)
            {
                clients.erase(clients.begin() + i);
            }
        }
    }

    for (int i = 0; i < clients.size(); i++)
    {
        close(clients[i]);
    }
    std::cout << "worker thread ends\n";
}

int main()
{
    signal(SIGINT, signal_handler);

    int sock, client_fd;
    socklen_t client_len;
    struct sockaddr_in broker, client;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    check(sock != -1);

    broker.sin_family = AF_INET;
    broker.sin_port = htons(8000);
    broker.sin_addr.s_addr = INADDR_ANY;

    check(bind(sock, (const sockaddr *)&broker, sizeof(broker)) != -1);
    check(listen(sock, 3) != -1);

    std::cout << "creating worker thread!\n";
    std::thread worker(broker_routine);

    while (!terminate && (client_fd = accept(sock, (struct sockaddr *)&client, &client_len)))
    {
        fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);
        clients.push_back(client_fd);
    }
    worker.join();
    std::cout << "worker thread joined to main thread\n";
    close(sock);
    std::cout << "terminating program\n";

    return 0;
}