#include "socket.h"

#include "string.h" // strerror()
#include "errno.h" // errno
#include <unistd.h> // close()
#include <fcntl.h>

#ifdef __linux__
# include <sys/socket.h> // socket(), bind(), listen()
# include <netinet/in.h> // IPv4, IPv6
# include <arpa/inet.h> // inet_ntoa()
#endif
#ifdef _WIN32
# include <winsock2.h>
# ifndef BUFSIZ
#  define BUFSIZ 1024
# endif
#endif

// TODO : keep ip and port data in private fields, implement public accessors.

Socket::Socket() : fd(-1), ok(false) { }

Socket::Socket(int fd) : fd(fd), ok(true) { }

Socket::Socket(short port, std::string address) : ok(false) {
	struct sockaddr_in addr;

#ifdef _WIN32
	WSADATA wsaData;
	int flag;
	flag = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(flag != 0) {
		this->error =
			std::string("Unable to create socket: WSAStartup failed: ")
			+ "Windows error code " + std::to_string(flag);
		return;
	}
#endif

	this->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->fd == -1) {
		this->error = std::string("Unable to create socket: ") +
#ifdef __linux__
			strerror(errno)
#endif
#ifdef _WIN32
			"Windows error code " + std::to_string(WSAGetLastError())
#endif
		;
		return;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef __linux__
	inet_aton(address.c_str(), &(addr.sin_addr));
#endif
#ifdef _WIN32
	addr.sin_addr.s_addr = inet_addr(address.c_str());
#endif
	if(connect(
		this->fd,
		(struct sockaddr *)&addr,
		sizeof(struct sockaddr_in)
	) == -1) {
		this->error = std::string("Unable to connect socket: ")+strerror(errno);
		return;
	}
	this->ok = true;
}

Socket::~Socket() {
	if(this->ok) {
		close(this->fd);
	}
}

void Socket::send(std::string message) {
#ifdef __linux__
	write(this->fd, message.c_str(), message.length());
#endif
#ifdef _WIN32
	::send(this->fd, message.c_str(), message.length(), 0);
#endif
}

std::string Socket::receive() {
#ifdef __linux__
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
#endif
#ifdef _WIN32
	this->ok = false;
	this->error = "Socket::receive : unimplemented method";
	return("");
// TODO
#endif
}

inline char Socket::getchar() {
	char c;
	int flag;
	flag =
#ifdef __linux__
		read(this->fd, &c, 1);
#endif
#ifdef _WIN32
		recv(this->fd, &c, 1, 0);
#endif
	if(flag == -1) {
		this->ok = false;
		this->error = std::string("Reading from socket: ") +
#ifdef __linux__
			strerror(errno)
#endif
#ifdef _WIN32
			"Windows error code " + std::to_string(WSAGetLastError())
#endif
		;
	}
	if(flag <= 0) {
		return((char)0);
	} else {
		return(c);
	}
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

void Socket::setBlocking(bool status) {
#ifdef __linux__
	int flags = fcntl(this->fd, F_GETFL, 0);
	if(flags == -1) {
		flags = 0;
	}
	int new_flags = status ? flags & ~O_NONBLOCK : flags | O_NONBLOCK;
	flags = fcntl(this->fd, F_SETFL, new_flags);
	if(flags == -1) {
	}
#endif
#ifdef _WIN32
	long unsigned int i = status ? 0 : 1;
	ioctlsocket(this->fd, FIONBIO, &i);
#endif
}

bool Socket::isOk() {
	return(this->ok);
}

std::string Socket::getError() {
	return(this->error);
}

ServerSocket::ServerSocket(short port) : Socket() {
	struct sockaddr_in addr;
#ifdef __linux__
	int yes=1;
#endif
#ifdef _WIN32
	const char yes = (const char) TRUE;
#endif

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
#ifdef _WIN32
	typedef int socklen_t;
#endif
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

