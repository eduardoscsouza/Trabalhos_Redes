#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/timeb.h>

#include "socketstream.hpp"

#define LOOPBACK_ADDR "127.0.0.1"
#define BASEPORT 24666

#define OBS_PER_SEC 4
#define SEC_IN_DAY 86400
#define SAMPLE_SIZE (SEC_IN_DAY * OBS_PER_SEC)

#define SENSOR_N 222
#define WAITING 1
#define SENDING 2
#define DEAD 0

using namespace std;



class PhysicalSensor
{
public:
	Client client;
	double * data;
	size_t data_count, sample_count;
	struct timeb dataload_time;
	char state;



	PhysicalSensor()
	{
		this->client = Client();
		this->data = NULL;
		this->sample_count = this->data_count = 0;
		this->dataload_time.time = 0;
		this->dataload_time.millitm = 0;
		this->dataload_time.timezone = 0;
		this->dataload_time.dstflag = 0;
		this->state = WAITING;
	}

	~PhysicalSensor()
	{
		this->client.clear();
		this->data = NULL;
		this->sample_count = this->data_count = 0;
		this->dataload_time.time = 0;
		this->dataload_time.millitm = 0;
		this->dataload_time.timezone = 0;
		this->dataload_time.dstflag = 0;
		this->state = WAITING;
	}



	void load_data(const char * data_filename, size_t data_count)
	{
		FILE * data_file = fopen(data_filename, "r+");
		this->data = new double[this->data_count=data_count];
		fread(this->data, this->data_count, sizeof(double), data_file);

		ftime(&(this->dataload_time));
		fclose(data_file);
	}

	void connect_to_virtsens(const char * ip_addr, unsigned short port)
	{
		this->client.connect_to_server(ip_addr, port);
	}

	void check_call()
	{
		bool activate = false;
		int cur_flags = fcntl(this->client.socket.socket_fd, F_GETFL);
		fcntl(this->client.socket.socket_fd, F_SETFL, cur_flags | O_NONBLOCK);
		if (this->client.receive(&activate, sizeof(bool), false)==-1) activate = false;
		fcntl(this->client.socket.socket_fd, F_SETFL, cur_flags);

		if (activate){
			size_t samples;
			this->client.receive(&samples, sizeof(size_t));
			if (samples==0) this->state = DEAD;
			else{
				this->sample_count = samples;
				this->state = SENDING;
			}
		}
		else this->state = WAITING;
	}

	void send_sample()
	{
		struct timeb cur_time;
		ftime(&cur_time);
		size_t cur_pos = OBS_PER_SEC*((cur_time.time - dataload_time.time) + ((cur_time.millitm-dataload_time.millitm)/1000.0));
		if (cur_pos<data_count && this->sample_count>0){
			this->client.send(data + cur_pos, sizeof(double));
			this->sample_count--;
			if (this->sample_count==0) this->state = WAITING;
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
	
	for (int i=0; i<16; i++){
		ps[i].connect_to_virtsens(LOOPBACK_ADDR, BASEPORT);
		ps[i].load_data("0.dat", SAMPLE_SIZE);
	}
	for (int i=16; i<19; i++){
		ps[i].connect_to_virtsens(LOOPBACK_ADDR, BASEPORT + 1);
		ps[i].load_data("1.dat", SAMPLE_SIZE);
	}
	for (int i=19; i<219; i++){
		ps[i].connect_to_virtsens(LOOPBACK_ADDR, BASEPORT + 2);
		ps[i].load_data("2.dat", SAMPLE_SIZE);
	}
	for (int i=219; i<222; i++){
		ps[i].connect_to_virtsens(LOOPBACK_ADDR, BASEPORT + 3);
		ps[i].load_data("0.dat", SAMPLE_SIZE);
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