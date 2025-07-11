#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class ChatServer {
private:
    int serverSocket;
    std::vector<int> clientSockets;
    std::mutex clientsMutex;
    bool running;
    const int PORT = 8080;
    const int MAX_CLIENTS = 10;

public:
    ChatServer() : running(false) {}

    bool initialize() {
        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        // Set socket options to reuse address
        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options" << std::endl;
            return false;
        }

        // Bind socket
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            return false;
        }

        // Listen for connections
        if (listen(serverSocket, MAX_CLIENTS) < 0) {
            std::cerr << "Failed to listen" << std::endl;
            return false;
        }

        std::cout << "Server started on port " << PORT << std::endl;
        return true;
    }

    void broadcastMessage(const std::string& message, int senderSocket) {
        std::lock_guard<std::mutex> lock(clientsMutex);
        
        for (int clientSocket : clientSockets) {
            if (clientSocket != senderSocket) {
                send(clientSocket, message.c_str(), message.length(), 0);
            }
        }
    }

    void handleClient(int clientSocket) {
        char buffer[1024];
        std::string clientName = "Client-" + std::to_string(clientSocket);
        
        std::cout << "New client connected: " << clientName << std::endl;
        
        // Send welcome message
        std::string welcomeMsg = "Welcome to the chat! You are " + clientName + "\n";
        send(clientSocket, welcomeMsg.c_str(), welcomeMsg.length(), 0);

        while (running) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytesReceived <= 0) {
                std::cout << "Client " << clientName << " disconnected" << std::endl;
                break;
            }

            std::string message = std::string(buffer);
            if (!message.empty() && message[message.length() - 1] == '\n') {
                message = message.substr(0, message.length() - 1);
            }

            if (!message.empty()) {
                std::string formattedMessage = clientName + ": " + message + "\n";
                std::cout << formattedMessage;
                broadcastMessage(formattedMessage, clientSocket);
            }
        }

        // Remove client from list
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
            if (it != clientSockets.end()) {
                clientSockets.erase(it);
            }
        }

        close(clientSocket);
    }

    void run() {
        running = true;
        std::cout << "Waiting for clients..." << std::endl;

        while (running) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            
            if (clientSocket < 0) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }

            // Check if we can accept more clients
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                if (clientSockets.size() >= static_cast<size_t>(MAX_CLIENTS)) {
                    std::string rejectMsg = "Server is full. Please try again later.\n";
                    send(clientSocket, rejectMsg.c_str(), rejectMsg.length(), 0);
                    close(clientSocket);
                    continue;
                }
                clientSockets.push_back(clientSocket);
            }

            // Start a new thread to handle this client
            std::thread clientThread(&ChatServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }

    void stop() {
        running = false;
        
        // Close all client connections
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (int clientSocket : clientSockets) {
                close(clientSocket);
            }
            clientSockets.clear();
        }
        
        close(serverSocket);
        std::cout << "Server stopped" << std::endl;
    }

    ~ChatServer() {
        stop();
    }
};

int main() {
    ChatServer server;
    
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }

    std::cout << "Chat Server" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;

    try {
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
} 