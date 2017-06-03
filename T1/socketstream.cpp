/*
Eduardo Santos Carlos de Souza	9293481
Fabrício Guedes Faria			9293522
Guilherme Hideo Tubone			9019403
Lucas de Oliveira Pacheco		9293182
*/

#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#include <iostream>

#include "socketstream.hpp"

#define STD_BACKLOG_SIZE 2048

using namespace std;



Socket::Socket()
{
	this->socket_fd = -1;
}

Socket::~Socket()
{
	this->clear();
}

/*
Incializa o socket usando a funcao do sistema
O socket é do tipo Stream e utiliza o protocolo IP
*/
void Socket::initialize(bool verbose)
{
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd < 0) if (verbose) cout<<"Error creating socket"<<endl;
}

/*
Fecha o socket usando a funcao do sistema
*/
void Socket::close_socket(bool verbose)
{
	if (this->socket_fd>0 && close(this->socket_fd)<0) if (verbose) cout<<"Error closing socket"<<endl;
}

void Socket::clear()
{
	this->socket_fd = -1;
}



InetAddress::InetAddress()
{
	this->address.sin_family = AF_INET;
	memset(this->address.sin_zero, 0, sizeof(this->address.sin_zero));
	this->address.sin_port = 0;
	this->address.sin_addr.s_addr = 0;
}

InetAddress::InetAddress(const char * ip_addr, unsigned short port, bool verbose) : InetAddress()
{
	this->set(ip_addr, port, verbose);
}

InetAddress::~InetAddress()
{
	this->clear();
}

/*
Monta a struct do endereco IP com o IP dado por ip_addr e a Porta port
*/
int InetAddress::set(const char * ip_addr, unsigned short port, bool verbose)
{
	this->address.sin_addr.s_addr = inet_addr(ip_addr);
	if (this->address.sin_addr.s_addr == 0){
		if (verbose) cout<<"Invalid address"<<endl;
		return -1;
	}
	this->address.sin_port = port;

	return 0;
}

void InetAddress::clear()
{
	this->address.sin_port = 0;
	this->address.sin_addr.s_addr = 0;
}



Client::Client()
{
	this->socket = Socket();
	this->server_addr = InetAddress();
}

Client::Client(const char * ip_addr, unsigned short port, bool verbose) : Client()
{
	this->connect_to_server(ip_addr, port, verbose);
}

Client::~Client()
{
	this->clear();
}

/*
Criar o socket e monta o endereco IP com os dados dado como argumento
Depois, conecta o cliente ao servidos especificado utilizando a funcao do sistema
*/
int Client::connect_to_server(const char * ip_addr, unsigned short port, bool verbose)
{
	//Inicializar atributos
	this->socket.initialize(verbose);
	this->server_addr.set(ip_addr, port, verbose);
	
	//Conectar no servidor
	if (connect(this->socket.socket_fd, (const struct sockaddr*) &(this->server_addr.address), sizeof(this->server_addr.address)) < 0){
		if (verbose) cout<<"Error connecting socket to server"<<endl;
		return -1;
	}

	return 0;
}

/*
Funcao bloqueante para enviar qualquer vetor de bytes para o servidor
*/
int Client::send(void * data, size_t nbytes, bool verbose)
{
	if (write(this->socket.socket_fd, data, nbytes) != nbytes){
		if (verbose) cout<<"Error sending data"<<endl;
		return -1;
	}

	return 0;
}

/*
Funcao bloqueante para ler qualquer vetor de bytes do servidor
*/
int Client::receive(void * data, size_t nbytes, bool verbose)
{
	if (read(this->socket.socket_fd, data, nbytes) != nbytes){
		if (verbose) cout<<"Error receiving data"<<endl;
		return -1;
	}

	return 0;
}

/*
Fecha o cliente
*/
void Client::close_client(bool verbose)
{
	this->socket.close_socket(verbose);
}

void Client::clear()
{
	this->socket.clear();
	this->server_addr.clear();
}



Server::Peer::Peer()
{
	this->socket = Socket();
	this->addr = InetAddress();
	this->addr_size = 0;
}

Server::Peer::~Peer()
{
	this->clear();
}

void Server::Peer::clear()
{
	this->socket.clear();
	this->addr.clear();
	this->addr_size = 0;
}



Server::Server()
{
	this->socket = Socket();
	this->addr = InetAddress();
	this->backlog_size = STD_BACKLOG_SIZE;
	this->peers = vector<Peer>();
}

Server::Server(const char * ip_address, unsigned short port, bool verbose)
{
	this->bind_to_server(ip_address, port, verbose);
}

Server::~Server()
{
	this->clear();
}

/*
Funcao que inicializa os atributos do objeto Servidor.
Depois liga o objeto Servidor a porta e ao IP dados nos argumetos
*/
int Server::bind_to_server(const char * ip_address, unsigned short port, bool verbose)
{
	//Incializar atributos
	this->socket.initialize(verbose);
	this->addr.set(ip_address, port, verbose);

	//Linkar servidor
	if(bind(this->socket.socket_fd, (const struct sockaddr*) &(this->addr.address), sizeof(this->addr.address)) < 0){
		if (verbose) cout<<"Error binding socket"<<endl;
		return -1;
	}

	return 0;
}

/*
Funcao bloqueante onde o servidor espera que client_count clientes
facam a conexao e aceita cada cliente
*/
int Server::accept_clients(size_t client_count, bool verbose)
{
	//Botar o servidor para escutar por conexoes
	if(listen(this->socket.socket_fd, this->backlog_size) < 0){
		if (verbose) cout<<"Error listening for connection"<<endl;
		return -1;
	}

	for (size_t i=0; i<client_count; i++){
		Peer new_peer = Peer();
		new_peer.socket.socket_fd = accept(this->socket.socket_fd, (struct sockaddr*) &(new_peer.addr.address), &(new_peer.addr_size));
		if (new_peer.socket.socket_fd < 0){
			if (verbose) cout<<"Error accepting connection"<<endl;
			return -1;
		}

		this->peers.push_back(new_peer);
	}

	return 0;
}

/*
Funcao bloqueante para enviar qualquer vetor de bytes para o cliente especificado por peer_od
*/
int Server::send(size_t peer_id, void * data, size_t nbytes, bool verbose)
{
	if (write(this->peers[peer_id].socket.socket_fd, data, nbytes) != nbytes){
		if (verbose) cout<<"Error sending data"<<endl;
		return -1;
	}

	return 0;
}

/*
Funcao bloqueante para receber qualquer vetor de bytes do cliente especificado por peer_od
*/
int Server::receive(size_t peer_id, void * data, size_t nbytes, bool verbose)
{
	if (read(this->peers[peer_id].socket.socket_fd, data, nbytes) != nbytes){
		if (verbose) cout<<"Error receiving data"<<endl;
		return -1;
	}

	return 0;
}

/*
Fecha o cliente
*/
void Server::close_server(bool verbose)
{
	this->socket.close_socket(verbose);
}

void Server::clear()
{
	this->socket.clear();
	this->addr.clear();
	this->backlog_size = 0;
	this->peers.clear();
}