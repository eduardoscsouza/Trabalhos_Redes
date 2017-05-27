#include <cstdio>
#include <unistd.h>
#include <ctime>
#include <fcntl.h>

#include "socketstream.hpp"

#define LOOPBACK_ADDR "127.0.0.1"
#define BASEPORT 24666

#define OBS_PER_SEC 4
#define SEC_IN_DAY 86400
#define SAMPLE_SIZE (SEC_IN_DAY * OBS_PER_SEC)

#define SENSOR_N 6
#define WAITING 1
#define SENDING 2
#define DEAD 0

using namespace std;



class PhysicalSensor
{
public:
	Client client;
	double * data;
	size_t data_count, end_sample, cur_pos;
	time_t dataload_time;
	char state;



	PhysicalSensor()
	{
		this->client = Client();
		this->data = NULL;
		this->cur_pos = this->end_sample = this->data_count = 0;
		this->dataload_time = -1;
		this->state = WAITING;
	}

	~PhysicalSensor()
	{
		this->client.clear();
		this->data = NULL;
		this->cur_pos = this->end_sample = this->data_count = 0;
		this->dataload_time = -1;
		this->state = WAITING;
	}



	void load_data(const char * data_filename, size_t data_count)
	{
		FILE * data_file = fopen(data_filename, "r+");
		this->data = new double[this->data_count=data_count];
		fread(this->data, this->data_count, sizeof(double), data_file);

		this->dataload_time = time(NULL);
		fclose(data_file);
	}

	void connect_to_virtsens(const char * ip_addr, unsigned short port)
	{
		this->client.connect_to_server(ip_addr, port);
	}

	void check_call()
	{
		int cur_flags = fcntl(this->client.socket.socket_fd, F_GETFL);
		fcntl(this->client.socket.socket_fd, F_SETFL, cur_flags | O_NONBLOCK);

		bool activate = false;
		if (this->client.receive(&activate, sizeof(bool), false)!=-1 && activate){
			size_t samples;
			this->client.receive(&samples, sizeof(size_t));
			if (samples==0) this->state = DEAD;
			else{
				this->cur_pos = time(NULL) - this->dataload_time;
				this->end_sample = this->cur_pos + samples;
				this->state = SENDING;
			}
		}
		else this->state = WAITING;

		fcntl(this->client.socket.socket_fd, F_SETFL, cur_flags);
	}

	void send_sample()
	{
		if (this->cur_pos < data_count && this->cur_pos<this->end_sample){
			this->client.send(data + cur_pos, sizeof(double));
			this->cur_pos++;
			if (this->cur_pos == this->end_sample){
				this->state = WAITING;
				this->cur_pos = this->end_sample = 0;
			}
		}
	}

	void close_sensor()
	{
		delete this->data;
		this->client.close_client();
	}
};



int main(int argc, char * argv[])
{
	vector<PhysicalSensor> ps = vector<PhysicalSensor>(SENSOR_N);
	vector<bool> alive = vector<bool>(SENSOR_N);
	int alive_count = SENSOR_N;
	for (int i=0; i<alive.size(); i++) alive[i] = true;
	for (int i=0; i<ps.size(); i++) ps[i] = PhysicalSensor();
	
	for (int i=0; i<3; i++){
		ps[i].connect_to_virtsens(LOOPBACK_ADDR, BASEPORT);
		ps[i].load_data("0.dat", SAMPLE_SIZE);
	}
	for (int i=3; i<SENSOR_N; i++){
		ps[i].connect_to_virtsens(LOOPBACK_ADDR, BASEPORT + 1);
		ps[i].load_data("1.dat", SAMPLE_SIZE);
	}

	while (alive_count > 0){
		for (int i=0; i<ps.size() && alive[i]; i++){
			if (ps[i].state==WAITING) ps[i].check_call();
			else if (ps[i].state==SENDING) ps[i].send_sample();
			else if (ps[i].state==DEAD){
				alive[i] = false;
				alive_count--;
			}
		}

		usleep(1000000/OBS_PER_SEC);
	}
	
	for (int i=0; i<ps.size(); i++) ps[i].close_sensor();
	return 0;
}