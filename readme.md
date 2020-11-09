# Event based, Multithreaded HTTP Server.
`eventloop.c` uses Linux kernel's epoll API for managing multiple connections at once.

* `general.h` contains some macro definitions.
* `chunk.h`, contains the defintion for `struct chunk` which is a length delimetered pointer.
it is similar to `string_view` in C++.
* `chunk_builderer.h` defines the `struct chunk_builder` which is for constructing strings.
It keeps a list of chunks.
* `eventloop.c` event managment system.
* `list.h` contains a linux style circular linked list.
* `http.c` HTTP parsing logic.
* `main.c` Listens socket, creates event loop, manages threads.
* `debug.c` Logging functions.