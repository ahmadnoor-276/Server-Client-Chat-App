# TCP Chat Application

A simple TCP-based chat application written in C++ that supports multi-user communication between a server and multiple clients.

## Features

- **Server-Client Architecture**: Centralized server that manages all client connections
- **Multi-User Support**: Server can handle multiple clients simultaneously using threads
- **Real-time Messaging**: Messages are broadcast to all connected clients
- **Cross-platform**: Uses standard socket APIs (Linux/Unix compatible)
- **Thread-safe**: Proper synchronization using mutexes for client management

## Requirements

- C++11 compatible compiler (GCC 4.8+ or Clang 3.3+)
- Linux/Unix system (uses POSIX socket APIs)
- pthread library (usually included with GCC)

## Building

1. **Compile both server and client:**
   ```bash
   make
   ```

2. **Compile only server:**
   ```bash
   make server
   ```

3. **Compile only client:**
   ```bash
   make client
   ```

4. **Clean build files:**
   ```bash
   make clean
   ```

## Usage

### Starting the Server

1. **Start the server:**
   ```bash
   ./server
   ```
   The server will start listening on port 8080.

2. **Server output:**
   ```
   Server started on port 8080
   Chat Server
   Press Ctrl+C to stop the server
   Waiting for clients...
   ```

### Connecting Clients

1. **Connect a client to localhost:**
   ```bash
   ./client
   ```

2. **Connect a client to a specific IP:**
   ```bash
   ./client 192.168.1.100
   ```

3. **Client output:**
   ```
   Chat Client
   Connecting to server: 127.0.0.1
   Type your messages and press Enter to send
   Type 'quit' to exit

   Connected to server at 127.0.0.1:8080
   Welcome to the chat! You are Client-4
   ```

### Chatting

- **Send messages**: Simply type your message and press Enter
- **Receive messages**: Messages from other clients will appear automatically
- **Exit**: Press Ctrl+C or close the terminal

## Example Session

### Terminal 1 (Server):
```bash
$ ./server
Server started on port 8080
Chat Server
Press Ctrl+C to stop the server
Waiting for clients...
New client connected: Client-4
New client connected: Client-5
Client-4: Hello everyone!
Client-5: Hi there!
```

### Terminal 2 (Client 1):
```bash
$ ./client
Connected to server at 127.0.0.1:8080
Welcome to the chat! You are Client-4
Hello everyone!
Client-5: Hi there!
```

### Terminal 3 (Client 2):
```bash
$ ./client
Connected to server at 127.0.0.1:8080
Welcome to the chat! You are Client-5
Client-4: Hello everyone!
Hi there!
```

## Technical Details

### Architecture

- **Server**: Single-threaded main loop that accepts connections and spawns worker threads
- **Client**: Dual-threaded (one for sending, one for receiving)
- **Communication**: TCP sockets with newline-delimited messages
- **Synchronization**: Mutex-protected client list on server

### Key Components

1. **ChatServer Class**:
   - Manages server socket and client connections
   - Broadcasts messages to all connected clients
   - Handles client disconnections gracefully

2. **ChatClient Class**:
   - Connects to server and maintains connection
   - Separate threads for sending and receiving messages
   - Handles connection errors and disconnections

### Message Format

- Messages are sent as plain text with newline delimiters
- Server prefixes messages with client identifier (e.g., "Client-4: message")
- No special serialization required for simple text chat

## Limitations

- Maximum 10 concurrent clients (configurable in server code)
- No persistent message history
- No user authentication
- No private messaging
- No file transfer capabilities

## Troubleshooting

### Common Issues

1. **"Address already in use" error:**
   - Wait a few minutes for the port to be released
   - Or change the port number in the code

2. **"Connection refused" error:**
   - Make sure the server is running
   - Check if the IP address is correct
   - Verify firewall settings

3. **Compilation errors:**
   - Ensure you have a C++11 compatible compiler
   - Install build-essential package: `sudo apt-get install build-essential`

### Port Configuration

To change the default port (8080), modify the `PORT` constant in both `server.cpp` and `client.cpp`.

## Future Enhancements

- Private messaging between users
- File transfer capabilities
- Message encryption
- Persistent chat history