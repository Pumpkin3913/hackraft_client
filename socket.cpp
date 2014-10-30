#include "socket.h"

#include "string.h" // strerror()
#include "errno.h" // errno
#include <unistd.h> // close()
#include <sys/socket.h> // socket(), bind(), listen()
#include <netinet/in.h> // IPv4, IPv6
#include <arpa/inet.h> // inet_ntoa()
#include <fcntl.h>

// TODO : keep ip and port data in private fields, implement public accessors.

Socket::Socket() : fd(-1), ok(false) { }

Socket::Socket(int fd) : fd(fd), ok(true) { }

Socket::Socket(short port, std::string address) : ok(true) {
	struct sockaddr_in addr;
	this->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->fd == -1) {
		this->error = std::string("Unable to create socket: ")+strerror(errno);
		this->ok = false;
		return;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(address.c_str(), &(addr.sin_addr));
	if(connect(
		this->fd,
		(struct sockaddr *)&addr,
		sizeof(struct sockaddr_in)
	) == -1) {
		this->error = std::string("Unable to connect socket: ")+strerror(errno);
		this->ok = false;
		return;
	}
}

Socket::~Socket() {
	if(this->ok) {
		close(this->fd);
	}
}

void Socket::send(std::string message) {
	write(this->fd, message.c_str(), message.length());
}

std::string Socket::receive() {
	char buf[BUFSIZ];
	int nb_read = read(this->fd, buf, BUFSIZ);
	if(nb_read > 0) {
		return(std::string(buf, nb_read));
	} else {
		if(errno) {
			this->error = std::string("Reading from socket: ")+strerror(errno);
			this->ok = false;
		}
		return("");
	}
}

inline char Socket::getchar() {
	char c;
	int flag;
	flag = read(this->fd, &c, 1);
	if(flag != 1) {
		if(errno) {
			this->error = std::string("Reading from socket: ")+strerror(errno);
			this->ok = false;
		}
		return((char)0);
	}
	return(c);
}

std::string Socket::getline() {
	char c;
	while(c = this->getchar(), c != '\n' && c != 0) {
		this->buffer += c;
	}
	if(c == '\n') {
		std::string s = this->buffer;
		this->buffer = "";
		return(s);
	} else {
		return("");
	}
}

void Socket::setNonBlock() {
	int flags = fcntl(this->fd, F_GETFL, 0);
	if(flags == -1) {
		flags = 0;
	}
	int new_flags = flags | O_NONBLOCK;
	flags = fcntl(this->fd, F_SETFL, new_flags);
	if(flags == -1) {
	}
}

bool Socket::isOk() {
	return(this->ok);
}

std::string Socket::getError() {
	return(this->error);
}

ServerSocket::ServerSocket(short port) : Socket() {
	struct sockaddr_in addr;
	int yes=1;

	if((this->fd = socket(PF_INET, SOCK_STREAM, 0))==-1) {
		this->error = std::string("Unable to create socket: ")+strerror(errno);
	}
	if(setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1) {
		this->error =
			std::string("Unable to set socket option SO_REUSEADDR: ")
			+ strerror(errno);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = 0;
	// FIXME: is this really useless ?
	// memset(&(host_addr.sin_zero), '\0', 8);
	if(bind(this->fd,(struct sockaddr *) &addr, sizeof(struct sockaddr))==-1) {
		this->error =
			std::string("Unable to bind socket to port: ")
			+ strerror(errno);
		this->ok = false;
		return;
	}
	if(listen(this->fd, ServerSocket::max_queue)==-1) {
		this->error =
			std::string("Unable to listen on socket: ")
			+ strerror(errno);
		this->ok = false;
		return;
	}

	// Make asynchronous.
	// fcntl(this->fd, F_SETFL, fcntl(servsock, F_GETFL) | O_ASYNC);

	// Ask the kernel to send us SIGIO on new connexion.
	// fcntl(this->fd, F_SETOWN, getpid());
}

class Socket * ServerSocket::accept() {
	struct sockaddr_in remote_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int new_fd = ::accept(this->fd, (struct sockaddr*) &remote_addr, &addr_len);
	if(new_fd == -1) {
		this->error =
			std::string("Unable to accept new connexion: ")
			+ strerror(errno);
		return(NULL);
	} else {
		// this->remote_ip = std::string(inet_ntoa(remote_addr.sin_addr));
		// this->remote_port = ntohs(remote_addr.sin_port);
		return(new Socket(new_fd));
	}
}

