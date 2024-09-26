# tiny_http - Work in Progress...
tiny_http is my attempt at creating a lightweight, easy-to-use, and embeddable HTTP server library in C99.
It is not designed to be a full-fledged web server, but rather a simple tool to build small web applications or to serve static files. No threading or forking is used, so it's not suitable for high-traffic applications.

Hello, World! example:
```c
#include "th.h"

static th_err
handler(void* userp, const th_request* req, th_response* res)
{
    th_set_body(res, "Hello, World!");
    th_add_header(res, "Content-Type", "text/plain");
    return TH_ERR_OK;
}

int main()
{
    th_server* server;
    th_server_create(&server, NULL);
    th_bind(server, "0.0.0.0", "8080", NULL);
    th_route(server, TH_METHOD_GET, "/", handler, NULL);
    while (1) {
        th_poll(server, 1000);
    }
}
```
Simply copy the code above to a file (e.g. `hello.c`) where you have the `th.h` and `th.c` files, and compile it with:
```sh
$ gcc -o hello hello.c th.c
```
Then run the server with:
```sh
$ ./hello
```

I wrote this library because I wanted a simple drop-in solution for the legacy C and C++ codebases I work with, hence the lack of dependencies and new language features. It lacks lots of important features and is not production-ready,
but I'll be adding more features and fixing bugs if I have the time.

## Features

- Simple integration (just copy the `th.h` and `th.c` files to your project)
- Simple API
- HTTPS support (via OpenSSL) (Works, but still needs to be optimized as it's quite slow)
- File serving
- Path capturing (e.g. `/user/{id}`)
- Supports Linux and MacOS (Windows support is planned)
- Fully customizable memory allocation and logging

## Dependencies
- OpenSSL (optional, for HTTPS support)
- gperf (optinal, for binary builds and amalgamation)
- python3 (optional, for running the amalgamation script)
- picohttpparser (included) - a small, fast HTTP parser
- The C standard library

## Planned features

- File Uploads
- Websockets
- Windows support

## Enabling HTTPS

To enable HTTPS support, you need to link against OpenSSL and set `TH_WITH_SSL=1` when compiling the library.
```sh
$ gcc -o myserver myserver.c th.c -lssl -lcrypto -DTH_WITH_SSL=1
```

## Building the project and CMake integration

Library builds, examples, and tests can be built using CMake (This requires gperf to be installed).
```sh
$ mkdir build; cd build
$ cmake ..
$ make
```

## More examples

Example - Path capturing:
```c
#include <th.h>

static th_err
handler(void* userp, const th_request* req, th_response* res)
{
    const char* msg = th_get_path_param(req, "msg");
    th_body_printf(res, "Hello, %s!", msg);
    th_add_header(res, "Content-Type", "text/plain");
    return TH_ERR_OK;
}

int main()
{
    th_server* server;
    th_server_create(&server, NULL);
    th_bind(server, "0.0.0.0", "8080", NULL);
    th_route(server, TH_METHOD_GET, "/{msg}", handler, NULL);
    while (1) {
        th_poll(server, 1000);
    }
}
```
It's possible to specify a capture type by adding a colon before the parameter name: `{string:param}` (Default if nothing is specified), `{int:param}`, `{path:param}`.

Example - File serving:
```c
#include <th.h>

static th_err
handle_path(void* userp, const th_request* req, th_response* res)
{
    const char* path = th_get_path_param(req, "path");
    th_body_from_file(res, "root", path);
    return TH_ERR_OK;
}

static th_err
handle_index(void* userp, const th_request* req, th_response* res)
{
    th_body_from_file(res, "root", "index.html");
    return TH_ERR_OK;
}

int main()
{
    th_server* server;
    th_server_create(&server, NULL);
    th_bind(server, "0.0.0.0", "8080", NULL);
    th_add_root(server, "root", "/path/to/your/files");
    th_route(server, TH_METHOD_GET, "/{path:path}", handle_path, NULL);
    th_route(server, TH_METHOD_GET, "/", handle_index, NULL);
    while (1) {
        th_poll(server, 1000);
    }
}
```

More detailed examples can be found in the `examples` directory.
