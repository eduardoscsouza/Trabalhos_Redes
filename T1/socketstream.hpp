/*
Eduardo Santos Carlos de Souza	9293481
Fabrício Guedes Faria			9293522
Guilherme Hideo Tubone			9019403
Lucas de Oliveira Pacheco		9293182
*/


#ifndef SOCKETSTREAM_HPP
#define SOCKETSTREAM_HPP

#include <arpa/inet.h>
#include <vector>

/*
Classe para lidar com os sockets da biblioteca sys/socket
*/
class Socket
{
public:
	int socket_fd;
	
	Socket();
	~Socket();

	void initialize(bool=true);
	/*
	Fecha o socket
	*/
	void close_socket(bool=true);
	void clear();
};

/*
Classe para lidar com os enderecos IP da biblioteca arpa/inet
*/
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

/*
Classe do cliente para o padrao cliente/servidor
*/
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

/*
Classe do servidor para o padrao cliente/servidor
*/
class Server
{
public:
	Socket socket;
	InetAddress addr;
	int backlog_size;

	/*
	Classe aninhada para armazenar os dados dos clientes
	*/
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