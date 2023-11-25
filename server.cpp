
// server.cpp
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#define BUFFER_SIZE 1024

struct ChatMessage
{
    char username[100];              // Fixed size for simplicity
    char message[BUFFER_SIZE - 100]; // Use the rest of the buffer for the actual message
};

std::ofstream chatLogFile; // Declare a file stream for the chat log
std::vector<int> clients;

// Function to log chat messages to the file
void logMessage(const ChatMessage &chatMsg)
{
    chatLogFile << chatMsg.username << ": " << chatMsg.message << std::endl;
}

void readFromClient(int client_fd)
{
    ChatMessage chatMsg;

    while (true)
    {
        memset(&chatMsg, 0, sizeof(chatMsg));

        // Read the message from the client
        ssize_t bytes_read = read(client_fd, &chatMsg, sizeof(chatMsg));

        if (bytes_read <= 0)
        {
            auto it = std::find(clients.begin(), clients.end(), client_fd);
            if (it != clients.end())
            {
                clients.erase(it);
            }
            break;
        }

        logMessage(chatMsg);

        // Send the structured message to other clients
        for (int fd : clients)
        {
            if (fd != client_fd)
            { // don't send back to the sender
                send(fd, &chatMsg, sizeof(chatMsg), 0);
            }
        }
    }
}

void sendToClient(int client_fd)
{
    // Might be required later.
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    std::atexit([]()
                { chatLogFile.close(); });

    int port = std::stoi(argv[1]);

    int server_fd, client1_fd, client2_fd;
    struct sockaddr_in address;
    int flag = 1;
    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &flag, sizeof(int)))
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port); // Use the specified port

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 2) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    chatLogFile.open("chatlog.txt", std::ios::out | std::ios::app);

    std::cout << "Waiting for clients to connect..." << std::endl;

    client1_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    clients.push_back(client1_fd);
    std::cout << "Client 1 connected." << std::endl;

    std::thread client1ReadThread(readFromClient, client1_fd);

    client2_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    clients.push_back(client2_fd);
    std::cout << "Client 2 connected." << std::endl;

    std::thread client2ReadThread(readFromClient, client2_fd);

    std::string servermessage;
    std::getline(std::cin, servermessage);

    if (servermessage == "exit")
    {
        std::cout << "Closing server." << std::endl;

        // Stop the client thread
        if (client1ReadThread.joinable())
        {
            client1ReadThread.join();
        }

        // Stop the client thread
        if (client2ReadThread.joinable())
        {
            client2ReadThread.join();
        }

        // Close all client sockets
        for (int fd : clients)
        {
            close(fd);
        }

        close(server_fd);
        return 0;
    }

    client1ReadThread.join();
    client2ReadThread.join();

    return 0;
}
