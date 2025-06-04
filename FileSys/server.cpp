#include "server.h"
#include "FileSys.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Usage: ./nfsserver port#\n";
    return -1;
  }

  // networking part: create the socket and accept the client connection

  addrinfo conn{}, *servinfo, *p;
  memset(&conn, 0, sizeof(conn));
  conn.ai_family = AF_INET;
  conn.ai_socktype = SOCK_STREAM;
  int status, sock;

  if ((status = getaddrinfo(nullptr, argv[1], &conn, &servinfo)) != 0) {
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
  // assume that sock is the new socket created
  //  for a TCP connection between the client and the server.

  // loop: get the command from the client and invoke the file
  // system operation which returns the results or error messages back to
  // the client until the client closes the TCP connection.
  size_t buf_size = 1024;
  char *buf = (char *)malloc(buf_size);
  Command cmd;

  sockaddr_storage client_addr;
  socklen_t addr_len = sizeof(client_addr);
  int new_sock = accept(sock, (sockaddr *)&client_addr, &addr_len);
  /*
  if (new_sock < 0) {
          perror("accept");
  }*/
  FileSys fs;
  fs.mount(new_sock);

  size_t byte_recv{0};
  bool finished = false;
  while (!finished) {
    memset(buf, 0, buf_size);
    if ((byte_recv = read_in(new_sock, buf, buf_size)) <= 0) {
      finished = true;
    } else {
      cmd = get_cmd(buf);
      finished = execute_cmd(cmd, fs);
    }

    if (cmd.name) {
      free(cmd.name);
    }
    if (cmd.file_name) {
      free(cmd.file_name);
    }
    if (cmd.append_data) {
      free(cmd.append_data);
    }
  }

  // close the listening socket
  close(new_sock);
  free(buf);
  // unmout the file system
  fs.unmount();
  close(sock);
  return 0;
}

ssize_t read_in(int sock, char *buf, size_t buf_size) {
  size_t bytes_recv{0};
  ssize_t n{0};
  bool finished = false;
  while (!finished) {
    if ((n = recv(sock, buf, buf_size, 0)) <= 0) {
      perror("receive");
      return -1;
    } else if (n == 0) {
      perror("connection closed");
      break;
    }
    bytes_recv += n;
    for (int i = bytes_recv - n; i < bytes_recv; i++) {
      if (buf[i] == '\0') {
        finished = true;
      }
    }
  }
  buf[bytes_recv] = '\0';
  return bytes_recv;
}

struct Command get_cmd(char *buf) {
  Command temp;
  char *buf_temp = strdup(buf);
  char *token = strtok(buf_temp, " ");
  temp.name = token ? strdup(token) : nullptr;
  token = strtok(nullptr, " ");
  temp.file_name = token ? strdup(token) : nullptr;
  token = strtok(nullptr, " ");
  temp.append_data = token ? strdup(token) : nullptr;
  free(buf_temp);

  return temp;
}
bool execute_cmd(struct Command cmd, FileSys &fs) {
  if (cmd.name == nullptr) {
    perror("command");
    return true;
  }

  if (strcmp(cmd.name, "mkdir") == 0) {
    fs.mkdir(cmd.file_name);
  } else if (strcmp(cmd.name, "ls") == 0) {
    fs.ls();
  } else if (strcmp(cmd.name, "rmdir") == 0) {
    fs.rmdir(cmd.file_name);
  } else if (strcmp(cmd.name, "create") == 0) {
    fs.create(cmd.file_name);
  } else if (strcmp(cmd.name, "append") == 0) {
    fs.append(cmd.file_name, cmd.append_data);

  } else if (strcmp(cmd.name, "cd") == 0) {
    fs.cd(cmd.file_name);
  } else if (strcmp(cmd.name, "cat") == 0) {
    cout << cmd.name << " " << cmd.file_name << '\n';
    fs.cat(cmd.file_name);
  } else if (strcmp(cmd.name, "head") == 0) {
    if (cmd.append_data) {
      unsigned int n = atoi(cmd.append_data);
      fs.head(cmd.file_name, n);
    }
  } else if (strcmp(cmd.name, "home") == 0) {
    fs.home();
  } else if (strcmp(cmd.name, "rm") == 0) {
    fs.rm(cmd.file_name);
  } else if (strcmp(cmd.name, "stat") == 0) {
    fs.stat(cmd.file_name);
  } else if (strcmp(cmd.name, "quit") == 0) {
    return true;
  }
  return false;
}
