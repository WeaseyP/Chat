
// server.cpp
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8082
#define BUFFER_SIZE 1024

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
    
    if ((client1_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Client 1 connected." << std::endl;

    if ((client2_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Client 2 connected." << std::endl;

    while (true) {
        // Relay messages between clients.
        memset(buffer, 0, BUFFER_SIZE);
        read(client1_fd, buffer, BUFFER_SIZE);
        send(client2_fd, buffer, strlen(buffer), 0);
        
        memset(buffer, 0, BUFFER_SIZE);
        read(client2_fd, buffer, BUFFER_SIZE);
        send(client1_fd, buffer, strlen(buffer), 0);
    }

    return 0;
}
