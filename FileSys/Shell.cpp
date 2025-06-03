// CPSC 3500: Shell
// Implements a basic shell (command line interface) for the file system
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <numeric>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
using namespace std;

#include "Shell.h"

static const string PROMPT_STRING = "NFS> "; // shell prompt

// Mount the network file system with server name and port number in the format
// of server:port
void Shell::mountNFS(string fs_loc) {
  // create the socket cs_sock and connect it to the server and port specified
  // in fs_loc if all the above operations are completed successfully, set
  // is_mounted to true
  auto [host_s, port_s] = parse_input(fs_loc);
  const char *host{host_s.c_str()};
  const char *port{port_s.c_str()};
  addrinfo conn{}, *servinfo, *p;
  memset(&conn, 0, sizeof(conn));
  conn.ai_family = AF_INET;
  conn.ai_socktype = SOCK_STREAM;

  if ((getaddrinfo(host, port, &conn, &servinfo)) != 0) {
    cerr << "status" << endl;
    exit(-1);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((cs_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("create socket");
      continue;
    }

    if (connect(cs_sock, p->ai_addr, p->ai_addrlen) != 0) {
      perror("connect");
      close(cs_sock);
      freeaddrinfo(servinfo);
      continue;
    }
    is_mounted = true;
    break;
  }
}
// Unmount the network file system if it was mounted
void Shell::unmountNFS() {
  // close the socket if it was mounted
  if (is_mounted) {
    close(cs_sock);
  }
  // out put an error saying nothing is mounted?
}

// Remote procedure call on mkdir
void Shell::mkdir_rpc(string dname) {
  // to implement
  // mkdir dname
  string cmd = "mkdir " + dname;
  write_out(cs_sock, cmd);
}

// Remote procedure call on cd
void Shell::cd_rpc(string dname) {
  // to implement
  string cmd = "cd " + dname;
  write_out(cs_sock, cmd);
}

// Remote procedure call on home
void Shell::home_rpc() {
  // to implement
  write_out(cs_sock, "home");
}

// Remote procedure call on rmdir
void Shell::rmdir_rpc(string dname) {
  // to implement
  string cmd = "rmdir " + dname;
  write_out(cs_sock, cmd);
}

// Remote procedure call on ls
void Shell::ls_rpc() {
  // to implement
  write_out(cs_sock, "ls");
}

// Remote procedure call on create
void Shell::create_rpc(string fname) {
  // to implement
	string cmd = "create " + fname;
	write_out(cs_sock, cmd);
}

// Remote procedure call on append
void Shell::append_rpc(string fname, string data) {
  // to implement
	string cmd = "append " + fname + " " + data;
	write_out(cs_sock, cmd);
}

// Remote procedure call on cat
void Shell::cat_rpc(string fname) {
  // to implement
	string cmd = "cat " + fname;
	write_out(cs_sock, cmd);
}

// Remote procedure call on head
void Shell::head_rpc(string fname, int n) {
  // to implement
	string intr = to_string(n);
	string cmd = "head " + fname + " " + intr;
	write_out(cs_sock, cmd);
}

// Remote procedure call on rm
void Shell::rm_rpc(string fname) {
  // to implement
	string cmd = "rm " + fname;
	write_out(cs_sock, cmd);
}

// Remote procedure call on stat
void Shell::stat_rpc(string fname) {
  // to implement
	string cmd = "stat " + fname;
	write_out(cs_sock, cmd);
}

// Executes the shell until the user quits.
void Shell::run() {
  // make sure that the file system is mounted
  if (!is_mounted)
    return;

  // continue until the user quits
  bool user_quit = false;
  while (!user_quit) {

    // print prompt and get command line
    string command_str;
    cout << PROMPT_STRING;
    getline(cin, command_str);

    // execute the command
    user_quit = execute_command(command_str);
  }
  // unmount the file system
  unmountNFS();
}

// Execute a script.
void Shell::run_script(char *file_name) {
  // make sure that the file system is mounted
  if (!is_mounted)
    return;
  // open script file
  ifstream infile;
  infile.open(file_name);
  if (infile.fail()) {
    cerr << "Could not open script file" << endl;
    return;
  }

  // execute each line in the script
  bool user_quit = false;
  string command_str;
  getline(infile, command_str, '\n');
  while (!infile.eof() && !user_quit) {
    cout << PROMPT_STRING << command_str << endl;
    user_quit = execute_command(command_str);
    getline(infile, command_str);
  }

  // clean up
  unmountNFS();
  infile.close();
}

// Executes the command. Returns true for quit and false otherwise.
bool Shell::execute_command(string command_str) {
  // parse the command line
  struct Command command = parse_command(command_str);

  // look for the matching command
  if (command.name == "") {
    return false;
  } else if (command.name == "mkdir") {
    mkdir_rpc(command.file_name);
  } else if (command.name == "cd") {
    cd_rpc(command.file_name);
  } else if (command.name == "home") {
    home_rpc();
  } else if (command.name == "rmdir") {
    rmdir_rpc(command.file_name);
  } else if (command.name == "ls") {
    ls_rpc();
  } else if (command.name == "create") {
    create_rpc(command.file_name);
  } else if (command.name == "append") {
    append_rpc(command.file_name, command.append_data);
  } else if (command.name == "cat") {
    cat_rpc(command.file_name);
  } else if (command.name == "head") {
    errno = 0;
    unsigned long n = strtoul(command.append_data.c_str(), NULL, 0);
    if (0 == errno) {
      head_rpc(command.file_name, n);
    } else {
      cerr << "Invalid command line: " << command.append_data;
      cerr << " is not a valid number of bytes" << endl;
      return false;
    }
  } else if (command.name == "rm") {
    rm_rpc(command.file_name);
  } else if (command.name == "stat") {
    stat_rpc(command.file_name);
  } else if (command.name == "quit") {
    return true;
  }

  return false;
}

// Parses a command line into a command struct. Returned name is blank
// for invalid command lines.
Shell::Command Shell::parse_command(string command_str) {
  // empty command struct returned for errors
  struct Command empty = {"", "", ""};

  // grab each of the tokens (if they exist)
  struct Command command;
  istringstream ss(command_str);
  int num_tokens = 0;
  if (ss >> command.name) {
    num_tokens++;
    if (ss >> command.file_name) {
      num_tokens++;
      if (ss >> command.append_data) {
        num_tokens++;
        string junk;
        if (ss >> junk) {
          num_tokens++;
        }
      }
    }
  }

  // Check for empty command line
  if (num_tokens == 0) {
    return empty;
  }

  // Check for invalid command lines
  if (command.name == "ls" || command.name == "home" ||
      command.name == "quit") {
    if (num_tokens != 1) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  } else if (command.name == "mkdir" || command.name == "cd" ||
             command.name == "rmdir" || command.name == "create" ||
             command.name == "cat" || command.name == "rm" ||
             command.name == "stat") {
    if (num_tokens != 2) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  } else if (command.name == "append" || command.name == "head") {
    if (num_tokens != 3) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  } else {
    cerr << "Invalid command line: " << command.name;
    cerr << " is not a command" << endl;
    return empty;
  }

  return command;
}

// helper funcs
pair<string, string> Shell::parse_input(string &fs_loc) {
  size_t col = fs_loc.find(':');
  string host_s = fs_loc.substr(0, col);
  string port_s = fs_loc.substr(col + 1);
  return {host_s, port_s};
}

void Shell::write_out(int sock, string cmd) {
  cout << "Writing out\n";
  char *buf = (char *)malloc(cmd.length() + 1);
  strcpy(buf, cmd.c_str());
  size_t buf_size = strlen(buf) + 1;
  size_t bytes_sent{0};
  ssize_t n{0};

  while (bytes_sent < buf_size) {
    n = send(sock, buf + bytes_sent, buf_size - bytes_sent, 0);
    if (n <= 0) {
      perror("send");
      break;
    }
    bytes_sent += n;
  }
  cout << buf << " at write_out function\n";
  free(buf);
}
