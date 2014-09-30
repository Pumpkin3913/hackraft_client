#ifndef SOCKET_H
#define SOCKET_H

#include <string>

class Socket {
	protected:
		int fd;
		bool ok;
		std::string error;
		Socket();
	public:
		Socket(short port, std::string address);
		Socket(int fd); // Create an already open socket.
		~Socket();
		void send(std::string message);
		std::string receive();
		bool isOk();
		std::string getError();
};

// TODO : surcharge operators << and >> with send and receive.

class ServerSocket : public Socket {
	private:
		void send(std::string message);
		std::string receive();
		static const int max_queue = 10;
	public:
		ServerSocket(short port);
		class Socket * accept();
};

#endif