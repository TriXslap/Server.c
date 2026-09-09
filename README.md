# HTTP Server in C 🌐

A small HTTP/1.1 server built from scratch in C as a hands-on way to learn how web servers work below the framework level.

I built this project to get more comfortable with **POSIX sockets, TCP connections, HTTP request parsing, file I/O, error handling, and organizing a C codebase across multiple source files**.

> This is a learning project, not a production-ready web server. The point was to implement the important pieces myself and understand what happens between a client connecting and a response being sent back.

---

## What it does

The server listens on port `8080`, accepts TCP connections, parses basic HTTP/1.1 requests, resolves requested files from the `public/` directory, and sends static files back to the client.

Currently it supports:

- `GET` requests over HTTP/1.1
- Static file serving from `public/`
- `/` mapped to `public/index.html`
- HTTP status responses for `400`, `404`, and `405`
- MIME type detection for HTML, CSS, JavaScript, PNG, and JPEG files
- A fallback `application/octet-stream` content type for unknown file extensions
- Partial-send handling through a `send_all()` helper
- File streaming in chunks instead of loading an entire file into memory
- A Makefile-based build

---

## Request flow

```text
Client
  |
  | TCP connection
  v
socket -> bind -> listen -> accept
                           |
                           v
                    receive request
                           |
                           v
                    parse request line
                           |
                           v
              resolve path + validate request
                           |
                           v
                    resolve MIME type
                           |
                           v
                    build HTTP response
                           |
                           v
                  stream file to client
```

The server is currently **single-threaded**, so connections are handled one at a time. I kept it that way on purpose while focusing on understanding the request/response flow first.

---

## Project structure

```text
Server.c/
├── include/
│   └── http.h
├── public/
│   └── index.html
├── src/
│   ├── server.c
│   ├── request.c
│   └── response.c
├── Makefile
└── README.md
```

### `server.c`
Creates the listening socket, binds it to port `8080`, starts listening, accepts clients, and passes each connected socket to the request handler.

### `request.c`
Receives the request, waits for the end of the HTTP headers, parses the request line, validates the method/version/path, and prepares the response metadata.

### `response.c`
Resolves filesystem paths and MIME types, builds HTTP responses, handles error responses, and streams files to the client.

### `http.h`
Contains the shared structs and function declarations used by the different parts of the server.

---

## Build & run

### Requirements

- Linux or another POSIX-like environment
- GCC
- `make`

### Build

```bash
make
```

### Run

```bash
./server
```

The server listens on:

```text
http://localhost:8080
```

You can also test it with `curl`:

```bash
curl -v http://localhost:8080/
```

### Clean object files

```bash
make clean
```

---

## Things I learned

This project taught me much more than just how to send an HTML file.

Some of the main concepts I worked with were:

- The lifecycle of a TCP server: `socket()`, `bind()`, `listen()`, and `accept()`
- The difference between TCP and HTTP
- File descriptors and socket lifetimes
- Why `recv()` and `send()` cannot always be treated as all-or-nothing operations
- Parsing an HTTP request manually
- Detecting the end of HTTP headers with `\r\n\r\n`
- Mapping file extensions to MIME types
- Building valid HTTP headers
- Streaming files with `fread()`
- Handling errors without crashing the server
- Splitting a C project into source files, headers, and a Makefile

A big part of the project was also learning to debug assumptions I had about sockets, buffers, return values, and the HTTP protocol itself.

---

## Current limitations

The project intentionally implements a small subset of HTTP.

It currently does **not** include:

- Multiple simultaneous clients
- Persistent HTTP connections
- Request bodies or `POST`
- Dynamic routes
- HTTPS/TLS
- Full HTTP/1.1 compliance
- Production-grade path security

---

## Possible next steps

If I continue the project, the areas I would most like to explore are:

- Concurrent request handling
- Better HTTP parsing
- More robust path validation
- Logging and tests
- Configurable ports and server settings
- Additional HTTP methods

---

## Why I built it

I learn best when I have to build something myself.

Instead of using a web framework and letting it handle networking and HTTP for me, I wanted to see what was actually happening underneath. This server is part of my broader goal of getting stronger at **C, Linux, networking, low-level development, and security**.
