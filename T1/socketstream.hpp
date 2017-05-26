#ifndef SOCKETSTREAM_HPP
#define SOCKETSTREAM_HPP

#include <arpa/inet.h>
#include <vector>

class Socket
{
public:
	int socket_fd;
	
	Socket();
	~Socket();

	void initialize(bool=false);
	void close_socket(bool=false);
	void clear();
};

class InetAddress
{
public:
	struct sockaddr_in address;

	InetAddress();
	InetAddress(const char *, unsigned short, bool=false);
	~InetAddress();

	int set(const char *, unsigned short, bool=false);
	void clear();
};

class Client
{
public:
	Socket socket;
	InetAddress server_addr;

	Client();
	Client(const char *, unsigned short, bool=false);
	~Client();

	int connect_to_server(const char *, unsigned short, bool=false);
	int send(void *, size_t, bool=false);
	int receive(void *, size_t, bool=false);

	void close_client(bool=false);
	void clear();
};

class Server
{
public:
	Socket socket;
	InetAddress addr;
	int backlog_size;

	class Peer
	{
	public:
		Socket socket;
		InetAddress addr;
		socklen_t addr_size;

		Peer();
		~Peer();

		void clear();
	};
	std::vector<Peer> peers;

	Server();
	Server(const char *, unsigned short, bool=false);
	~Server();

	int bind_to_server(const char *, unsigned short, bool=false);
	int accept_clients(size_t, bool=false);
	int send(size_t, void *, size_t, bool=false);
	int receive(size_t, void *, size_t, bool=false);

	void close_server(bool=false);
	void clear();
};


#endif