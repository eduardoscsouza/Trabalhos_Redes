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

	void initialize();
	void close_socket();
	void clear();
};

class InetAddress
{
public:
	struct sockaddr_in address;

	InetAddress();
	InetAddress(const char *, unsigned short);
	~InetAddress();

	int set(const char *, unsigned short);
	void clear();
};

class Client
{
public:
	Socket socket;
	InetAddress server_addr;

	Client();
	Client(const char *, unsigned short);
	~Client();

	int connect_to_server(const char *, unsigned short);
	int send(void *, size_t);
	void * receive();

	void close_client();
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
	Server(const char *, unsigned short);
	~Server();

	int bind_to_server(const char *, unsigned short);
	int accept_clients(size_t);

	void close_server();
	void clear();
};


#endif