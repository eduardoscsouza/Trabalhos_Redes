#include <unistd.h>

#include <iostream>
#include <vector>
#include <functional>

#include "socketstream.hpp"

#define LOOPBACK_ADDR "127.0.0.1"
#define BASEPORT 24666

#define OBS_PER_SEC 4
#define SEC_IN_DAY 86400
#define SAMPLE_SIZE (SEC_IN_DAY * OBS_PER_SEC)

using namespace std;



class VirtualSensor
{
public:
	Server server;
	size_t physen_count;
	vector<double> physen_data;
	function<double(vector<double>)> func;



	VirtualSensor()
	{
		this->server = Server();
		this->physen_count = 0;
		this->physen_data = vector<double>();
		this->func = function<double(vector<double>)>();
	}

	VirtualSensor(const char * ip_addr, unsigned short port)
	{
		this->server = Server(ip_addr, port);
		this->physen_count = 0;
		this->physen_data = vector<double>();
		this->func = function<double(vector<double>)>();
	}

	~VirtualSensor()
	{
		this->server.clear();
		this->physen_count = 0;
		this->physen_data = vector<double>();
		this->func = function<double(vector<double>)>();
	}



	void set_func(function<double(vector<double>)> func)
	{
		this->func = func;
	}

	double calculate()
	{
		return this->func(this->physen_data);
	}

	void bind_to_server(const char * ip_addr, unsigned short port)
	{
		this->server.bind_to_server(ip_addr, port);
	}

	void accept_physen(size_t physen_count)
	{
		this->server.accept_clients(physen_count);
		this->physen_count = physen_count;
		this->physen_data = vector<double>(physen_count);
	}

	void call_physen(size_t samples)
	{
		bool call = true;
		for (int i=0; i<this->physen_count; i++){
			this->server.send(i, &call, sizeof(bool));
			this->server.send(i, &samples, sizeof(size_t));
		}
	}

	void receive_sample()
	{
		for (int i=0; i<this->physen_count; i++)
			this->server.receive(i, &(this->physen_data[i]), sizeof(double));
	}

	void close_sensor()
	{
		this->server.close_server();
	}
};



double mean(vector<double> vect)
{
	size_t size = vect.size();
	double m = 0;
	for (int i=0; i<size; i++) m+=vect[i]/size;

	return m;
}

double sum(vector<double> vect)
{
	double s = 0;
	for (int i=0; i<vect.size(); i++) s+=vect[i];

	return s;
}



int main(int argc, char * argv[])
{
	VirtualSensor vs[2];
	for (int i=0; i<2; i++) vs[i] = VirtualSensor(LOOPBACK_ADDR, BASEPORT + i);
	for (int i=0; i<2; i++) vs[i].accept_physen(3);

	vs[0].set_func(&mean);
	vs[1].set_func(&sum);

	int op = -1;
	size_t samples = 0;
	while (op!=0){
		cin>>op;
		if (op!=0){
			cin>>samples;
			vs[op-1].call_physen(samples);
			for (int i=0; i<samples; i++){
				vs[op-1].receive_sample();
				for (int j=0; j<vs[op-1].physen_data.size(); j++) cout<<vs[op-1].physen_data[j]<<" ";
				cout<<endl;
				usleep(1000000/OBS_PER_SEC);
			}
		}
		else{
			for (int i=0; i<2; i++){
				vs[i].call_physen(0);
				vs[i].close_sensor();
			}
		}
	}

	return 0;
}