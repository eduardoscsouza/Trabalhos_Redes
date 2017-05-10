#include <iostream>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#define SERVER_PORT 54666
#define SERVER_ADDRESS "127.0.0.1"

using namespace std;



int main(int argc, char * argv[])
{
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0) {
		cout<<"Error creating socket"<<endl;
		return -1;
	}

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = (in_port_t) SERVER_PORT;
	server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
	if (connect(client_socket, (const struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
		cout<<"Error connecting socket"<<endl;
		return -1;
	}
	
	write(client_socket, "Connection Succesful", sizeof("Connection Succesful"));

	close(client_socket);

	return 0;
}