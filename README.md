# HTTP Server in C

A lightweight HTTP/1.1 server written from scratch in C using POSIX sockets.

This project was built to explore low-level network programming and understand what happens between receiving raw TCP data and sending a valid HTTP response, without relying on an HTTP framework or networking library.

## Features

- HTTP/1.1 request handling
- Static file serving from the `public/` directory
- Request parsing for method, path, and HTTP version
- URL-to-file path resolution
- MIME type detection based on file extensions
- HTTP status handling:
  - `200 OK`
  - `400 Bad Request`
  - `404 Not Found`
  - `405 Method Not Allowed`
- Partial `recv()` handling for requests split across multiple TCP reads
- Reliable response transmission with partial `send()` handling
- File streaming in chunks instead of loading the entire file into memory
- Structured HTTP request and response data using C structs
- Modular source structure separating server, request, and response logic

## Project Structure

```text
.
├── include/
│   └── http.h
├── public/
│   ├── index.html
│   └── ...
├── src/
│   ├── server.c
│   ├── request.c
│   └── response.c
├── Makefile
└── README.md
```

### `server.c`

Responsible for the TCP server lifecycle:

- Creating the socket
- Binding to the server address
- Listening for connections
- Accepting clients
- Passing connected clients to the HTTP layer

### `request.c`

Responsible for receiving and interpreting HTTP requests:

- Receiving request data
- Parsing the HTTP request line
- Resolving requested paths
- Validating requests

### `response.c`

Responsible for constructing and sending HTTP responses:

- Resolving MIME types
- Building HTTP headers
- Generating error responses
- Streaming files to the client
- Ensuring all response bytes are transmitted

## Build

The project includes a Makefile.

From the project root:

```bash
make
```

This compiles the source files and links them into the `server` executable.

To remove intermediate object files:

```bash
make clean
```

The project is compiled using C17 with additional compiler warnings enabled.

## Run

After building:

```bash
./server
```

The server listens for incoming TCP connections and serves files from the `public/` directory.

You can then access the server from a browser or send requests manually using a tool such as `curl`.

For example:

```bash
curl http://localhost:<port>/
```

Replace `<port>` with the port configured in `server.c`.

## How It Works

At a high level, each connection goes through the following pipeline:

```text
TCP connection
      |
      v
Receive HTTP request
      |
      v
Parse request line
      |
      v
Resolve requested path
      |
      v
Validate request
      |
      v
Determine response type
      |
      v
Build HTTP response
      |
      v
Send headers and body
      |
      v
Close connection
```

The server operates directly on TCP sockets. Since TCP is a byte stream, a complete HTTP request is not assumed to arrive in a single `recv()` call. Incoming data is accumulated until the end of the HTTP headers is detected.

Similarly, responses are not assumed to be transmitted completely by a single `send()` call. The server keeps track of the number of bytes sent until the complete response has been transmitted.

## Supported Requests

The server currently supports:

```http
GET / HTTP/1.1
```

and GET requests for static resources inside the `public/` directory.

Other HTTP methods are rejected with `405 Method Not Allowed`.

The project intentionally implements a limited subset of HTTP/1.1. Its goal is to explore HTTP and TCP internals rather than provide a production-ready web server.

## What I Learned

Building the server from scratch involved working directly with several low-level concepts, including:

- POSIX socket programming
- TCP stream behavior
- HTTP request and response structure
- Partial network reads and writes
- Buffer and string management in C
- File I/O and streaming
- HTTP status codes and MIME types
- Struct-based data organization
- Error propagation and validation
- Separating a C project into modules and header files
- Compilation, linking, object files, and Makefiles

## Possible Future Improvements

The current implementation is intentionally small and focused. Possible extensions include:

- Concurrent client handling
- HTTP keep-alive
- Additional HTTP methods
- Request timeouts
- Non-blocking sockets
- `epoll`-based event handling
- More extensive HTTP parsing and validation

## Purpose

This project is primarily an educational implementation designed to build a deeper understanding of C, networking, TCP, and HTTP by implementing the core server behavior manually.
