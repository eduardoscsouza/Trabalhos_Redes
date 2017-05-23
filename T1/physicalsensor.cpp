#include <unistd.h>
#include <sstream>

#include "socketstream.hpp"

#define LOOPBACK_ADDR "127.0.0.1"
#define BASEPORT 24666
#define SAMPLE_SIZE 1000000
#define MICROSEC_IN_DAY 86400000000

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

	void connect_to_virtsens(const char * ip_addr, unsigned short port)
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
	stringstream filename;
	PhysicalSensor ps[2][3];
	for (int i=0; i<2; i++){
		for (int j=0; j<3; j++){
			filename.str("");
			filename<<j<<".dat";

			ps[i][j] = PhysicalSensor();
			ps[i][j].load_data(filename.str().c_str(), SAMPLE_SIZE);
			ps[i][j].connect_to_virtsens(LOOPBACK_ADDR, BASEPORT + i);
		}
	}

	for (int k=0; k<SAMPLE_SIZE; k++){
		for (int i=0; i<2; i++)
			for (int j=0; j<3; j++)
				ps[i][j].send_sample();

		usleep(MICROSEC_IN_DAY/SAMPLE_SIZE);
	}
	
	for (int i=0; i<2; i++) for (int j=0; j<3; j++) ps[i][j].close_sensor();
	return 0;
}