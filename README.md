# Simple Web Server in C 🌐

A lightweight, low-level HTTP/1.1 web server written from scratch in C using POSIX sockets. This project demonstrates raw TCP/IP networking, socket programming, manual HTTP request parsing, and memory-efficient static file streaming.

---

## Architecture & Flow 🛠️

The server is designed with a clean, modular architecture separating network events from client handling and file streaming:

[Client (Browser)] 
       │
       │  TCP Connection (Port 8080)
       ▼
[server_loop()] ─── (accept) ───► [client_socket]
                                        │
                                        ▼
                                [handle_client()]
                                        │
                                        ├─► Parse HTTP Request
                                        │
                                        ▼
                                 [send_file()] ◄─── (fread 1KB chunks) ─── [index.html]

### Modular Structure:
1. server_loop: The core daemon that runs an infinite loop, listening for connections via accept() and passing them to the worker safely.
2. handle_client: Manages the socket lifecycle of an active connection, handles incoming buffers with recv(), and acts as the router.
3. send_file: Handles the filesystem layer. Opens the requested path (fopen), builds the HTTP status response, and streams the payload efficiently in chunks using fread().

---

## Features 🚀

* Zero-Dependency POSIX Sockets: Built completely on top of GNU/Linux standard libraries (sys/socket.h).
* Efficient Memory Footprint: Streams files in small, configurable chunks (default: 1024 bytes) using custom buffer pooling to avoid loading entire huge files into memory.
* Early-Return Error Handling (Guard Clauses): Prevents crashes, memory leaks, and undefined behaviors by strictly handling closed sockets, broken pipes, and missing assets.
* Extensible Architecture: Designed to easily integrate Multi-threading (pthread) and dynamic routing.

---

## Getting Started 💻

### Prerequisites
Make sure you have a C compiler (gcc or clang) installed on your Linux/macOS development environment.

### 1. Clone & Setup
Clone the repository and place your index.html file in the root directory next to your source files.

### 2. Compilation
Compile the server using gcc:

gcc -O3 main.c -o server

### 3. Run the Server
Run the compiled binary:

./server

*The server will start listening on port 8080 by default.*

### 4. Test it
Open your browser and navigate to:

http://localhost:8080

---

## Future Roadmap 🗺️
- [ ] Request Routing: Extracting the requested URI (e.g., /about.html) from the HTTP header and serving the matching asset.
- [ ] Multi-threaded Worker Pool: Spawning pthread workers inside handle_client to process concurrent client requests without blocking.
- [ ] MIME Type Detection: Dynamically serving images, CSS, and JS files by identifying file extensions.
