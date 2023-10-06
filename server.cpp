
// server.cpp
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>


#define PORT 8084
#define BUFFER_SIZE 1024

int client1_fd, client2_fd;

void handleClient(int client_fd, int other_client_fd) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            // Handle disconnection or error.
            break;
        }
        send(other_client_fd, buffer, strlen(buffer), 0);
    }
}

int main() {
    int server_fd, client1_fd, client2_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Waiting for clients to connect..." << std::endl;
    
    client1_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    std::cout << "Client 1 connected." << std::endl;

    client2_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    std::cout << "Client 2 connected." << std::endl;

    std::thread client1Thread(handleClient, client1_fd, client2_fd);
    std::thread client2Thread(handleClient, client2_fd, client1_fd);

    client1Thread.join();
    client2Thread.join();

    return 0;
}
