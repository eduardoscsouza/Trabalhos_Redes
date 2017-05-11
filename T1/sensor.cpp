#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#define SERVER_PORT 54666
#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_BUFFER_SIZE 1024

using namespace std;



class ClientSocket
{	
public:
	int client_socket;
	struct sockaddr_in server_address;

	ClientSocket()
	{
		this->server_address.sin_family = AF_INET;
		this->server_address.sin_port = 0;
		this->server_address.sin_addr.s_addr = 0;
		memset(this->server_address.sin_zero, 0, 8*sizeof(char));

		this->client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (this->client_socket < 0) cout<<"Error creating socket"<<endl;
	}

	~ClientSocket()
	{
		if (this->client_socket >= 0){
			close(this->client_socket);
			this->client_socket = -1;
		}
	}

	int connect_to_server(const char * ip_address, unsigned short port)
	{
		this->server_address.sin_addr.s_addr = inet_addr(ip_address);
		if (this->server_address.sin_addr.s_addr == 0){
			cout<<"Invalid address"<<endl;
			return -1;
		}
		this->server_address.sin_port = port;

		if (connect(this->client_socket, (const struct sockaddr*)&(this->server_address), sizeof(this->server_address)) < 0){
			cout<<"Error connecting socket"<<endl;
			//return -1;
		}

		return 0;
	}

	int send_to_server(void * data, size_t data_size)
	{
		if (client_socket<0){
			cout<<"Invalid socket"<<endl;
			return -1;
		}

		if (write(this->client_socket, data, data_size) != data_size){
			cout<<"Error writting"<<endl;
			return -1;
		}

		return 0;
	}

};


class Sensor
{
public:
	ClientSocket client_socket;
	double * data;
	size_t data_count, cur_pos;

	Sensor()
	{
		this->client_socket = ClientSocket();
		this->data = NULL;
		this->data_count = 0;
		this->cur_pos = 0;
	}

	~Sensor()
	{
		if (this->data!=NULL){
			free(this->data);
			this->data = NULL;
		}
	}

	int load_data(FILE * data_file, size_t data_count)
	{
		this->data = (double*) malloc(sizeof(double) * data_count);
		this->data_count = data_count;

		if (fread(this->data, sizeof(double), this->data_count, data_file) != this->data_count){
			cout<<"Error reading data"<<endl;
			free(this->data);
			this->data = NULL;
			this->data_count = 0;
			return -1;
		}

		return 0;
	}

	int send_sample()
	{
		return(this->client_socket.send_to_server(data + cur_pos++, sizeof(double)));
	}

	int connect_to_server()
	{
		return this->client_socket.connect_to_server(SERVER_ADDRESS, SERVER_PORT);
	}
};



int main(int argc, char * argv[])
{
	Sensor * s = new Sensor[15];
	for (int i=0; i<15; i++) s[i] = Sensor();

	for (int i=0; i<15; i++){
		s[i].connect_to_server();
		usleep(1000);
	}

	return 0;
}