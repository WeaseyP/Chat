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

#define PORT 3776
#define BUFFER_SIZE 1024

struct ChatMessage {
    char username[100]; // Fixed size for simplicity
    char message[BUFFER_SIZE - 100]; // Use the rest of the buffer for the actual message
};

//Mutex for cout for send/receive.
std::mutex cout_mutex;

/*
Receive message from server
Arguments: int sock - socket
Listens for messages from the server and prints them.
*/
void receiveMessage(int sock, const std::string& username) {
    ChatMessage chatMsg;

    while (true) {
        memset(&chatMsg, 0, sizeof(chatMsg));
        ssize_t bytes_read = read(sock, &chatMsg, sizeof(chatMsg));

        if (bytes_read <= 0) {
            // Handle disconnection or error.
            break;
        }
        {
            // Lock std::cout
            std::lock_guard<std::mutex> lock(cout_mutex);

            std::cout << chatMsg.username << ": " << chatMsg.message << std::endl;
        } // unlock std::cout after leaving the scope
    }
}
/*
Send messages to the server.
Arguments:
  int sock - socket file descriptor
  const std::string& username - the username of the sender
Waits for user input and sends the prefixed message to the server.
*/
void sendMessage(int sock, const std::string& username) {
    ChatMessage chatMsg;
    strncpy(chatMsg.username, username.c_str(), sizeof(chatMsg.username) - 1);

    while (true) {
        // Lock std::cout
        std::cin.getline(chatMsg.message, sizeof(chatMsg.message));
        // Unlock std::cout

        send(sock, &chatMsg, sizeof(chatMsg), 0);
    }
}

int main() {
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
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    std::string username;
    std::cout << "Please enter your name: ";
    std::getline(std::cin, username);
    std::cout << username << std::endl;

    // Send username to the server.
    send(sock, username.c_str(), username.length(), 0);

    std::cout << "Welcome " << username << ". Type messages and press enter." << std::endl;

    std::thread sendThread(sendMessage, sock, username);
    std::thread recvThread(receiveMessage, sock, username);


    sendThread.join();  // Detach the sendThread.
    recvThread.join();    // Wait for the recvThread to finish.


    return 0;
}
