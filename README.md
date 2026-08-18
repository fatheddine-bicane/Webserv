*This project has been created as part of the 42 curriculum by fbicane, mael-gho, mgamraou.*

# webserv

## Description

webserv is a from-scratch HTTP/1.1 server written in C++98, built as part of the 42 school curriculum. The goal is to deeply understand how web servers work by implementing one yourself — no external libraries, no shortcuts.

The server is fully non-blocking and driven by a single `epoll` event loop in edge-triggered mode, capable of handling multiple simultaneous client connections without spawning threads or processes. It parses a custom configuration file (inspired by NGINX) to define virtual servers, routes, and behavior, then serves static files, executes CGI scripts, and handles file uploads — all while staying RFC-compliant.

### Features

- **HTTP/1.1** compliant — correct status codes, headers, and persistent connections
- **Non-blocking I/O** — single-threaded event loop using `epoll` in edge-triggered mode (`EPOLLIN | EPOLLET`)
- **Multiple virtual servers** — host several servers on different ports/hostnames from one config
- **Custom configuration file** — define server blocks, locations, allowed methods, redirections, root directories, and more
- **Static file serving** — serves HTML, CSS, JS, images, and any other static assets
- **Directory listing** — optional autoindex for directory browsing
- **Default error pages** — customizable per server or location
- **Supported methods** — `GET`, `POST`, `DELETE`
- **CGI execution** — runs scripts via the CGI/1.1 interface
- **File upload** — handles `multipart/form-data` uploads
- **Cookies & session management** — sets and reads cookies, maintains basic sessions
- **Chunked transfer encoding** — correctly handles chunked request bodies
- **Client body size limit** — configurable max body size per location

---

## Instructions

### Requirements

- A C++98-compatible compiler (`c++` / `g++` / `clang++`)
- `make`
- Linux or macOS

### Compilation

```bash
git clone git@github.com:fatheddine-bicane/Webserv.git
cd webserv
make
```

This produces the `webserv` binary at the project root.

### Running the server

```bash
./webserv [configuration_file]
```

```bash
# Example
./webserv www/webserv.conf
```

### Configuration file (quick overview)

```nginx
server {
    listen       8080;
    server_name  localhost;
    root         ./www;
    index        index.html;
    client_max_body_size 10M;

    location / {
        allow_methods GET POST;
        autoindex on;
    }

    location /upload {
        allow_methods POST;
        upload_store ./www/uploads;
    }

    location /cgi-bin {
        allow_methods GET POST;
        cgi_pass .py /usr/bin/python3;
        cgi_pass .js /usr/bin/node;
    }

    error_page 404 /errors/404.html;
}
```

### Stopping the server

```
Ctrl+C
```

---

## Usage Examples

```bash
# Serve a static page
curl http://localhost:8080/index.html

# Upload a file
curl -F "file=@photo.jpg" http://localhost:8080/upload

# Trigger a CGI script
curl http://localhost:8080/cgi-bin/hello.py

# Delete a resource
curl -X DELETE http://localhost:8080/upload/photo.jpg
```

---

## Resources

### HTTP & Networking

- [RFC 7230 — HTTP/1.1: Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 7231 — HTTP/1.1: Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231)
- [RFC 3875 — CGI/1.1 Specification](https://datatracker.ietf.org/doc/html/rfc3875)
- [MDN Web Docs — HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)

### Parsing & Configuration

- [NGINX documentation](https://nginx.org/en/docs/) — used as inspiration for the config file format

---

### AI Usage

AI was used during this project in the following ways:

- **Concept understanding** — clarifying parts of the HTTP/1.1 RFCs (chunked encoding, header parsing rules, CGI environment variables, etc.) when documentation was dense or ambiguous.
- **Debugging assistance** — occasionally helping interpret cryptic error messages or spotting off-by-one issues during testing.

AI was **not** used to write any part of the source code. All implementation decisions and code were authored by the team.
