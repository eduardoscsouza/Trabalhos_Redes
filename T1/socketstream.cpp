#include "socketstream.hpp"

#include <iostream>

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

using namespace std;



Socket::Socket()
{
	this->socket_fd = -1;
}

Socket::~Socket()
{
	this->clear();
}

void Socket::initialize()
{
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd < 0) cout<<"Error creating socket"<<endl;
}

void Socket::close_socket()
{
	if (this->socket_fd>0 && close(this->socket_fd)<0) cout<<"Error closing socket"<<endl;
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

InetAddress::InetAddress(const char * ip_addr, unsigned short port) : InetAddress()
{
	this->set(ip_addr, port);
}

InetAddress::~InetAddress()
{
	this->clear();
}

int InetAddress::set(const char * ip_addr, unsigned short port)
{
	this->address.sin_addr.s_addr = inet_addr(ip_addr);
	if (this->address.sin_addr.s_addr == 0){
		cout<<"Invalid address"<<endl;
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

Client::Client(const char * ip_addr, unsigned short port) : Client()
{
	this->connect_to_server(ip_addr, port);
}

Client::~Client()
{
	this->clear();
}

int Client::connect_to_server(const char * ip_addr, unsigned short port)
{
	this->socket.initialize();
	this->server_addr.set(ip_addr, port);
	
	if (connect(this->socket.socket_fd, (const struct sockaddr*) &(this->server_addr.address), sizeof(this->server_addr.address)) < 0){
		cout<<"Error connecting socket to server"<<endl;
		return -1;
	}

	return 0;
}

int Client::send(void * data, size_t nbytes)
{
	if (write(this->socket.socket_fd, data, nbytes) != nbytes){
		cout<<"Error sending data"<<endl;
		return -1;
	}

	return 0;
}

int Client::receive(void * data, size_t nbytes)
{
	if (read(this->socket.socket_fd, data, nbytes) != nbytes){
		cout<<"Error receiving data"<<endl;
		return -1;
	}

	return 0;
}

void Client::close_client()
{
	this->socket.close_socket();
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
	this->backlog_size = 1024;
	this->peers = vector<Peer>();
}

Server::Server(const char * ip_address, unsigned short port)
{
	this->bind_to_server(ip_address, port);
}

Server::~Server()
{
	this->clear();
}

int Server::bind_to_server(const char * ip_address, unsigned short port)
{
	this->socket.initialize();
	this->addr.set(ip_address, port);

	if(bind(this->socket.socket_fd, (const struct sockaddr*) &(this->addr.address), sizeof(this->addr.address)) < 0){
		cout<<"Error binding socket"<<endl;
		return -1;
	}

	return 0;
}

int Server::accept_clients(size_t client_count)
{
	if(listen(this->socket.socket_fd, this->backlog_size) < 0){
		cout<<"Error listening for connection"<<endl;
		return -1;
	}

	for (size_t i=0; i<client_count; i++){
		Peer new_peer = Peer();
		new_peer.socket.socket_fd = accept(this->socket.socket_fd, (struct sockaddr*) &(new_peer.addr.address), &(new_peer.addr_size));
		if (new_peer.socket.socket_fd < 0){
			cout<<"Error accepting connection"<<endl;
			return -1;
		}

		this->peers.push_back(new_peer);
	}
}

int Server::send(size_t peer_id, void * data, size_t nbytes)
{
	if (write(this->peers[peer_id].socket.socket_fd, data, nbytes) != nbytes){
		cout<<"Error sending data"<<endl;
		return -1;
	}

	return 0;
}

int Server::receive(size_t peer_id, void * data, size_t nbytes)
{
	if (read(this->peers[peer_id].socket.socket_fd, data, nbytes) != nbytes){
		cout<<"Error receiving data"<<endl;
		return -1;
	}

	return 0;
}

void Server::close_server()
{
	this->socket.close_socket();
}

void Server::clear()
{
	this->socket.clear();
	this->addr.clear();
	this->backlog_size = 0;
	this->peers.clear();
}