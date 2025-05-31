#include "server.h"
#include "FileSys.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Usage: ./nfsserver port#\n";
    return -1;
  }

  // networking part: create the socket and accept the client connection

  addrinfo conn{}, *servinfo, *p;
  memset(&conn, 0, sizeof(conn));
  conn.ai_family = AF_UNSPEC;
  conn.ai_socktype = SOCK_STREAM;
  int status, sock;

  if ((status = getaddrinfo("cs1.seattleu.edu", argv[1], &conn, &servinfo)) !=
      0) {
    cerr << "status" << endl;
    exit(-1);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("create socket");
      continue;
    }

    if (bind(sock, p->ai_addr, p->ai_addrlen)) {
      perror("bind");
      continue;
    }
    if (listen(sock, 10)) {
      perror("listen");
      continue;
    }
    break;
  }

  // mount the file system
  FileSys fs;
  fs.mount(sock); // assume that sock is the new socket created
  //  for a TCP connection between the client and the server.

  // loop: get the command from the client and invoke the file
  // system operation which returns the results or error messages back to
  // the client until the client closes the TCP connection.

  char* buf = (char*) malloc(1024);
  size_t buf_size = sizeof(buf);
  socklen_t addr_len = p->ai_addrlen;
  int new_sock = accept(sock, p->ai_addr, &addr_len);

  if (new_sock < 0) {
    perror("accept");
	}

	bool close = true;
  while (close) {
    read_in(new_sock, buf, buf_size);
  }

  // close the listening socket

  // unmout the file system
  // fs.unmount();

  return 0;
}

void read_in(int sock, char *buf, size_t buf_size) {
  size_t bytes_sent{0};
  int n;
  while (bytes_sent < buf_size) {
    if (n = recv(sock, buf + bytes_sent, buf_size - bytes_sent, 0) == -1) {
      perror("receive");
      break;
    } else if (n == 0) {
      perror("connection closed");
    }
    bytes_sent += n;
    if (bytes_sent < buf_size) {
      buf[bytes_sent] = '\0';
    }
  }
}
/*
client
read in command from CLI
parse through it formating to how we expect to receive it
string streaming is how it happens
we need identifiers for splitting arguments, we can assume its ws
requires a buffer to read in the response, atoil for length,

server
loop through recv buffer, consuming until we see \r\n, linear algo
we can use a finite state machine to move between what were are doing
and what is next
the response message has a header and body
header with code text, length in bytes (int), \r\n separates body
everything is ended with \r\n
stlmap for body, or integer codes that mean something


OOP
isolating processes into classes / functions

itoa &YY atoi

*/
