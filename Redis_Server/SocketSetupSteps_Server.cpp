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

void msg(const char* s) {
	fprintf(stderr, "%s\n", s);
}

static void do_something(int connfd) {
	char rbuf[64] = {};
	ssize_t n = read(connfd, rbuf, sizeof(rbuf) -1);
	if (n < 0) {
		msg("read() error");
		return;
	}
	printf("client says: %s\n", rbuf);

	char wbuf[] = "world";
	write(connfd, wbuf, strlen(wbuf));
}

int main() {
	// obtain a socket handle, AF_INET - IPv4, SOCK_STREAM - TCP
	// additionally, AF_INET6 - IPv6, SOCK_DGRAM - UDP
	// third argument is useless for our purpose
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	// socketsockopt() changes the behavior of the socket (no TCPdelay, IP Qos)

	// set socket options
	// arguments 2 and 3 set options for the socket
	// we are setting SO_REUSEADDR to an int val of 1
	// this option accepts boolean values
	// its imporant to set this to 1 for all listening sockets for connection
	int val = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));


	// bind to an address
	struct sockaddr_in addr; // prototyping the struct

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234); // port, host to network short
	addr.sin_addr.s_addr = htonl(0); // IP 0.0.0.0
	int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
	if (rv) { die("bind()"); }

	struct sockaddr_in {
		uint16_t sin_family; // AF_INET
		uint16_t sin_port; // port in big-endian
		struct in_addr sin_addr; // IPv4
	};

	struct in_addr {
		uint32_t s_addr; // IPv4 in big-endian
	};

	/*
	Side notes

	There are two ways to store integers in memory
	- little-endian: least significant byte comes first
	- big-endian (network byte order): most significant byte comes first

	The difference is in the order of bytes, and reversing the order is called byte swapping.

	In 2025 only litte-endian CPUs are relevant. If a format is using big-endian byte swapping is required.

	Networking uses big-endian, so conversion is required at the CPU level

	short and long is used to equate little vs big endian 

	All of the work up until now is just passing parameters to determine the behavior of the socket connections

	Listen is when the socket is actually created and avaliable for accept() connections
	*/

	// listen
	rv = listen(fd, SOMAXCONN); // the 2nd argument is the size of queue, SOMAXCONN is 4096, doesnt matter that much because accept is not a bottleneck
	if (rv) { die("listen()"); }; // OS automatically handles the handshake of tcp

	// accept connections
	while (true) {
		struct sockaddr_in client_addr = {};
		socklen_t addrlen = sizeof(client_addr);
		int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
		if (connfd < 0) {
			continue;
		} // pass over if connection is made

		do_something(connfd);
		close(connfd);
	}

	// read & write
	// read/write & send/recv have minimal differences, send/recv can pass some optional flags

	// create a TCP client
	/*
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < ) {
		die("socket()");
	}

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // INADDR_LOOPBACK is defined as an address of 127.0.0.1
	int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
	if (rv) {
		die("connect");
	}

	char msg[] = "hello";
	write(fd, msg, strlen(msg));

	char rbuf[64] = {};
	ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
	if (n < 0) {
		die("read");
	}
	printf("server says: %s\n", rbuf);
	close(fd);
	*/

	return 0;
};
