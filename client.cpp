#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class ChatClient {
private:
    int clientSocket;
    bool running;
    std::string serverIP;
    const int PORT = 8080;

public:
    ChatClient(const std::string& ip = "127.0.0.1") : running(false), serverIP(ip) {}

    bool connectToServer() {
        // Create socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        // Set up server address
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        
        if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address" << std::endl;
            return false;
        }

        // Connect to server
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            return false;
        }

        std::cout << "Connected to server at " << serverIP << ":" << PORT << std::endl;
        return true;
    }

    void receiveMessages() {
        char buffer[1024];
        
        while (running) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytesReceived <= 0) {
                std::cout << "Disconnected from server" << std::endl;
                running = false;
                break;
            }

            std::string message = std::string(buffer);
            std::cout << message;
        }
    }

    void sendMessages() {
        std::string message;
        
        while (running) {
            std::getline(std::cin, message);
            
            if (!running) break;
            
            if (!message.empty()) {
                message += "\n";
                if (send(clientSocket, message.c_str(), message.length(), 0) < 0) {
                    std::cerr << "Failed to send message" << std::endl;
                    running = false;
                    break;
                }
            }
        }
    }

    void run() {
        running = true;
        
        // Start receive thread
        std::thread receiveThread(&ChatClient::receiveMessages, this);
        
        // Send messages in main thread
        sendMessages();
        
        // Wait for receive thread to finish
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
    }

    void stop() {
        running = false;
        close(clientSocket);
    }

    ~ChatClient() {
        stop();
    }
};

int main(int argc, char* argv[]) {
    std::string serverIP = "127.0.0.1";
    
    if (argc > 1) {
        serverIP = argv[1];
    }

    std::cout << "Chat Client" << std::endl;
    std::cout << "Connecting to server: " << serverIP << std::endl;
    std::cout << "Type your messages and press Enter to send" << std::endl;
    std::cout << "Type 'quit' to exit" << std::endl << std::endl;

    ChatClient client(serverIP);
    
    if (!client.connectToServer()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    try {
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
} 