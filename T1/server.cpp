#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#define SERVER_PORT 54666
#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_CONNECTIONS_SIZE 1024

using namespace std;



class Peer
{
public:
	int peer_socket;
	struct sockaddr_in address;
	socklen_t address_size;

	Peer()
	{
		this->peer_socket = -1;
		this->address.sin_family = AF_INET;
		this->address.sin_port = 0;
		this->address.sin_addr.s_addr = 0;
		memset(this->address.sin_zero, 0, sizeof(this->address.sin_zero));
		this->address_size = 0;
	}

	~Peer()
	{
		this->peer_socket = -1;
		this->address.sin_port = 0;
		this->address.sin_addr.s_addr = 0;
		this->address_size = 0;
	}

	void close_socket()
	{
		close(this->peer_socket);
		this->peer_socket = -1;
	}
};

class Server
{
public:
	int server_socket, backlog_max_size;
	struct sockaddr_in address;
	vector<Peer> peers;

	Server()
	{
		this->address.sin_family = AF_INET;
		this->address.sin_port = 0;
		this->address.sin_addr.s_addr = 0;
		memset(this->address.sin_zero, 0, sizeof(this->address.sin_zero));
		this->backlog_max_size = SERVER_CONNECTIONS_SIZE;
		this->peers = vector<Peer>();

		this->server_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (this->server_socket < 0) cout<<"Error creating socket"<<endl;
	}

	~Server()
	{
		this->server_socket = -1;
		this->address.sin_port = 0;
		this->address.sin_addr.s_addr = 0;
	}

	int bind_to_server(const char * ip_address, unsigned short port)
	{
		this->address.sin_addr.s_addr = inet_addr(ip_address);
		if (this->address.sin_addr.s_addr == 0){
			cout<<"Invalid address"<<endl;
			return -1;
		}
		this->address.sin_port = port;

		if(bind(this->server_socket, (const struct sockaddr*) &(this->address), sizeof(this->address)) < 0){
			cout<<"Error binding socket"<<endl;
			return -1;
		}

		return 0;
	}

	int connect_to_clients(int n_clients, bool verbose = 0)
	{
		if (verbose) cout<<"Listening for connection..."<<endl;
		if(listen(this->server_socket, this->backlog_max_size) < 0){
			cout<<"Error listening for connection"<<endl;
			return -1;
		}

		for (int i=0; i<n_clients; i++){
			Peer new_peer = Peer();
			new_peer.peer_socket = accept(this->server_socket, (struct sockaddr*) &(new_peer.address), &(new_peer.address_size));
			if (new_peer.peer_socket < 0){
				cout<<"Error accepting connection"<<endl;
				return -1;
			}

			cout<<"Socket: "<<new_peer.peer_socket<<"; Server Addr: "<<new_peer.address.sin_addr.s_addr<<"; Port: "<<new_peer.address.sin_port<<"; Lenght: "<<sizeof(new_peer.address)<<endl;
			if(verbose) cout<<"Connected!"<<endl;
			peers.push_back(new_peer);
		}
	}

	void close_socket()
	{
		close(this->server_socket);
		this->server_socket = -1;
	}
};


int main(int argc, char * argv[])
{
	Server server;
	server.bind_to_server(SERVER_ADDRESS, SERVER_PORT);	
	server.connect_to_clients(20, 1);
	/*
	char message[SERVER_BUFFER_SIZE];
	bool stop = false;
	while(!stop){
		if (read(client_socket, message, SERVER_BUFFER_SIZE)==0) usleep(1000);
		else{
			if (message=="quit") stop=true;
			else cout<<message<<endl;
		}
	}*/

	//close(server_socket);
	//close(peer_socket);

	return 0;
}