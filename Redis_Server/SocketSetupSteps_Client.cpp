#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

void die(const char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int main() {
// specifying the socket parameters, IPv4, TCP, not sure what 3rd arg is
	int fd = socket(AF_INET, SOCK_STREAM, 0);
// error handling 
	if (fd < 0) {
		die("socket()");
	}
// creating an empy sockaddr_in type, which is already defined 
	struct sockaddr_in addr = {};
// adding the parameters to the struct
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
// bind can be used before connect to specify an IP and Port but
// isn't required as OS auto chooses for connect if not specified
// which is okay if there is only one connection to choose from
// connect sends a request with the socket, 
// the address parameters specifically type casted, and the size 
	int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
// error handling
	if (rv) {
		die("connect");
	}
// simple message to send
	char msg[] = "hello";
// write the message to the socket
	write(fd, msg, strlen(msg));
// set up for a return message
	char rbuf[64] = {};
// read what the server has sent
	ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
// error handling
	if (n < 0) {
		die("read");
	}
// visible printing what was received from the server
	printf("server says: %s\n", rbuf);
// close the connection after the exchange
	close(fd);

	return 0;
}
