// client.cpp
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#define PORT 8084
#define BUFFER_SIZE 1024

//Mutex for cout for send/receive.
std::mutex cout_mutex;

/*
Receive message from server
Arguments: int sock - socket
Listens for messages from the server and prints them.
*/
void receiveMessage(int sock) {
    // Buffer to store the message.
    char buffer[BUFFER_SIZE]; 
    //Keep listening for messages.
    while (true) {
        // Clear the buffer.
        memset(buffer, 0, BUFFER_SIZE); 
        // Read a message from the server. 
        ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
        // Disconnect or error. 
        if (bytes_read <= 0) {
            // Handle disconnection or error.
            break;
        }
        //Lock std::cout
        cout_mutex.lock();
        std::cout << "Someone Else: " << buffer << std::endl;
        std::cout << "You: ";
        //Unlock std::cout
        cout_mutex.unlock();
    }
}
/*
Receive message from server
Arguments: int sock - socket
Listens for messages from the client and sends them to the server
*/
void sendMessage(int sock) {
    // Buffer to store the message.
    char message[BUFFER_SIZE];
    //Keep listening for messages.
    while (true) {
        // Lock std::cout
        cout_mutex.lock();
        std::cout << "You: ";
        //Read a message from the user
        std::cin.getline(message, BUFFER_SIZE);
        //Unlock std::cout
        cout_mutex.unlock();
        //Send the message to the server
        send(sock, message, strlen(message), 0);
    }
}


int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

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

    std::cout << "Connected to server. Type messages and press enter." << std::endl;

    std::thread recvThread(receiveMessage, sock);
    std::thread sendThread(sendMessage, sock);

    sendThread.detach();  // Detach the sendThread.
    recvThread.join();    // Wait for the recvThread to finish.


    return 0;
}
