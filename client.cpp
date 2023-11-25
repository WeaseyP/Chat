// client.cpp
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <algorithm>

#define BUFFER_SIZE 1024

struct ChatMessage 
{
    char username[100]; // Fixed size for simplicity
    char message[BUFFER_SIZE - 100]; // Use the rest of the buffer for the actual message
};

struct ConnectedUser {
    int socket;
    std::string username;

    ConnectedUser(int s, const std::string& u) : socket(s), username(u) {}
};

std::vector<ConnectedUser> connectedUsers;  // Vector to store connected users

//Mutex for cout for send/receive.
std::mutex cout_mutex;

void displayConnectedUsers() {
    std::cout << "Connected Users: ";
    for (const auto& user : connectedUsers) {
        std::cout << user.username << " ";
    }
    std::cout << std::endl;
}

void addUser(int userSocket, const std::string& username) {
    connectedUsers.emplace_back(userSocket, username);
    displayConnectedUsers();
}

void removeUser(int userSocket) {
    auto it = std::remove_if(connectedUsers.begin(), connectedUsers.end(),
                             [userSocket](const ConnectedUser& user) {
                                 return user.socket == userSocket;
                             });
    connectedUsers.erase(it, connectedUsers.end());
    displayConnectedUsers();
}

void receiveMessage(int sock, const std::string& username) {
    ChatMessage chatMsg;

    while (true) {
        memset(&chatMsg, 0, sizeof(chatMsg));
        ssize_t bytes_read = read(sock, &chatMsg, sizeof(chatMsg));

        if (bytes_read <= 0) {
            // Handle disconnection or error.
            break;
        }

        std::cout << chatMsg.username << ": " << chatMsg.message << std::endl;
    }

    // The user has disconnected, remove them from the list
    removeUser(sock);
}

void sendMessage(int sock, const std::string& username) {
    ChatMessage chatMsg;
    strncpy(chatMsg.username, username.c_str(), sizeof(chatMsg.username) - 1);

    while (true) {
        std::cin.getline(chatMsg.message, sizeof(chatMsg.message));

        send(sock, &chatMsg, sizeof(chatMsg), 0);
    }
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int port = std::stoi(argv[2]);

    int sock = 0;
    int flag = 1;
    struct sockaddr_in serv_addr;

    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    std::string username;
    std::cout << "Please enter your name: ";
    std::getline(std::cin, username);

    // Send username to the server.
    send(sock, username.c_str(), username.length(), 0);

    // Add the user to the list of connected users
    addUser(sock, username);
    
    std::cout << "Welcome " << username << ". Type messages and press enter." << std::endl;

    std::thread sendThread(sendMessage, sock, username);
    std::thread recvThread(receiveMessage, sock, username);

    sendThread.join();  // Detach the sendThread.
    recvThread.join();  // Wait for the recvThread to finish.

    // Close the socket when the threads finish
    close(sock);

    return 0;
}
