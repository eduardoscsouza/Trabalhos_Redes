#include <iostream>
//#include <cstdio>

#include "socketstream.hpp"

using namespace std;



class PhysicalSensor
{
public:
	Client client;
	double * data;
	size_t data_count, cur_pos;



	PhysicalSensor()
	{
		this->client = Client();
		this->data = NULL;
		this->data_count = this->cur_pos = 0;
	}

	~PhysicalSensor()
	{
		this->client.clear();
		this->data = NULL;
		this->cur_pos = this->data_count = 0;
	}



	void load_data(const char * data_filename, size_t data_count)
	{
		FILE * data_file = fopen(data_filename, "r+");
		this->data = new double[this->data_count=data_count];
		fread(this->data, this->data_count, sizeof(double), data_file);
		fclose(data_file);
	}

	void connect_to_server(const char * ip_addr, unsigned short port)
	{
		this->client.connect_to_server(ip_addr, port);
	}

	void send_sample()
	{
		if (cur_pos < data_count) this->client.send(data + cur_pos++, sizeof(double));
	}

	void close_sensor()
	{
		delete this->data;
		this->client.close_client();
	}
};



int main(int argc, char * argv[])
{
	PhysicalSensor ps;
	ps.connect_to_server("127.0.0.1", 54666);
	
	Client c[10];
	for (int i=0; i<10; i++) c[i] = Client("127.0.0.1", 54666);

	char message[] = "hello from x";
	for (int i=0; i<10; i++){
		message[11] = '0' + i;
		c[i].send(message, sizeof(message));
		//usleep(1000000);
	}

	char response[] = "          ";
	for (int i=0; i<10; i++){
		c[i].receive(response, sizeof("hi x"));
		cout<<response<<endl;
	}
	
	for (int i=0; i<10; i++) c[i].close_client();
	return 0;
}