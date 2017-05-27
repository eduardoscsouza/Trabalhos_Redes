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

	void initialize(bool=true);
	void close_socket(bool=true);
	void clear();
};

class InetAddress
{
public:
	struct sockaddr_in address;

	InetAddress();
	InetAddress(const char *, unsigned short, bool=true);
	~InetAddress();

	int set(const char *, unsigned short, bool=true);
	void clear();
};

class Client
{
public:
	Socket socket;
	InetAddress server_addr;

	Client();
	Client(const char *, unsigned short, bool=true);
	~Client();

	int connect_to_server(const char *, unsigned short, bool=true);
	int send(void *, size_t, bool=true);
	int receive(void *, size_t, bool=true);

	void close_client(bool=true);
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
	Server(const char *, unsigned short, bool=true);
	~Server();

	int bind_to_server(const char *, unsigned short, bool=true);
	int accept_clients(size_t, bool=true);
	int send(size_t, void *, size_t, bool=true);
	int receive(size_t, void *, size_t, bool=true);

	void close_server(bool=true);
	void clear();
};


#endif