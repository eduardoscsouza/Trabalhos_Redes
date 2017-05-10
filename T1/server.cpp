#include <iostream>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#define SERVER_PORT 54666
#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_BUFFER_SIZE 1024

using namespace std;



int main(int argc, char * argv[])
{
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		cout<<"Error creating socket"<<endl;
		return -1;
	}

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = (in_port_t) SERVER_PORT;
	server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
	if(bind(server_socket, (const struct sockaddr*) &server_address, sizeof(server_address))) {
		cout<<"Error binding socket"<<endl;
		return -1;
	}

	if(listen(server_socket, SERVER_BUFFER_SIZE) < 0){
		cout<<"Error listening to connection"<<endl;
		return -1;
	}

	struct sockaddr_in peer_address;
	socklen_t peer_address_size;
	int peer_socket = accept(server_socket, (struct sockaddr*) &peer_address, &peer_address_size);
	if (peer_socket < 0){
		cout<<"Error accepting connection"<<endl;
		return -1;
	}

	char message[SERVER_BUFFER_SIZE];
	read(peer_socket, message, sizeof("Connection Succesful"));

	cout<<message<<endl;

	close(server_socket);
	close(peer_socket);

	return 0;
}