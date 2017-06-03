/*
Eduardo Santos Carlos de Souza	9293481
Fabrício Guedes Faria			9293522
Gustavo Cabral					9293028
*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/timeb.h>
#include <cstring>

#include <sstream>
#include <string>
#include <iostream>

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



/*
Classe do sensor físico do trabalho
*/
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



	/*
	Carrega o vetor de leituras que o sensor fisico ira mandar para o virtual
	*/
	void load_data(const char * data_filename, size_t data_count)
	{
		FILE * data_file = fopen(data_filename, "r");
		if (data_file == NULL){
			cout<<"Error opening data file"<<endl<<"Have you generated the data files?"<<endl;
			return;
		}

		this->data = new double[this->data_count=data_count];//double[this->data_count=data_count];
		if(fread(this->data, sizeof(double), this->data_count, data_file) != this->data_count){
			cout<<"Error loading data"<<endl;
			return;
		}

		ftime(&(this->dataload_time));
		fclose(data_file);
	}

	/*
	Conecta o sensor fisico ao virtual
	*/
	void connect_to_virtsens(const char * ip_addr, unsigned short port)
	{
		this->client.connect_to_server(ip_addr, port);
	}

	/*
	Verifica se o sensor virtual pediu que o fisico enviasse leituras
	*/
	void check_call()
	{
		bool activate = false;
		//Faz a leitura ser nao-bloqueante
		int cur_flags = fcntl(this->client.socket.socket_fd, F_GETFL);
		fcntl(this->client.socket.socket_fd, F_SETFL, cur_flags | O_NONBLOCK);
		if (this->client.receive(&activate, sizeof(bool), false)==-1) activate = false;
		//Faz a leitura ser bloqueante novamente
		fcntl(this->client.socket.socket_fd, F_SETFL, cur_flags);

		//Se foi pedido para ele enviar leituras, le quantas leituras ele deve enviar
		if (activate){
			size_t samples;
			this->client.receive(&samples, sizeof(size_t));
			//Se foram 0 leituras, significa que o sensor fisico deve fechar
			if (samples==0) this->state = DEAD;
			else{
				this->sample_count = samples;
				this->state = SENDING;
			}
		}
		else this->state = WAITING;
	}

	/*
	Envia a leitura para o sensor virtual
	*/
	void send_sample()
	{
		struct timeb cur_time;
		ftime(&cur_time);
		//Define a posicao da leitura baseada no tempo atual e no tempo inicial.
		size_t cur_pos = OBS_PER_SEC*((cur_time.time - dataload_time.time) + ((cur_time.millitm-dataload_time.millitm)/1000.0));
		if (cur_pos<data_count && this->sample_count>0){
			this->client.send(data + cur_pos, sizeof(double));
			//Decrementa o numero de leituras restantes para enviar
			this->sample_count--;
			if (this->sample_count==0) this->state = WAITING;
		}
	}

	/*
	Fecha o sensor
	*/
	void close_sensor()
	{
		delete[] this->data;
		this->client.close_client();
	}
};



int main(int argc, char * argv[])
{
	if (argc!=1 && argc!=3){
		cout<<"---USO---"<<endl<<"/virtualsensor.out"<<"OU"<<endl<<"/virtualsensor.out <ip> <porta>"<<endl;
		return 0;
	}
	string add = string((argc==3) ? argv[1] : LOOPBACK_ADDR);
	short port = (argc==3) ? atoi(argv[2]) : BASEPORT;

	PhysicalSensor * ps = new PhysicalSensor[SENSOR_N];
	bool * alive = new bool[SENSOR_N];
	for (int i=0; i<SENSOR_N; i++) ps[i] = PhysicalSensor();
	memset(alive, true, sizeof(bool)*SENSOR_N);
	int alive_count = SENSOR_N;
	
	stringstream filename;
	for (int i=0; i<SENSOR_N; i++){
		//Define o nome correto do arquivo de dados para aquele sensor
		filename.str("");
		filename<<"./data/";
		if (i<16) filename<<((i%4==3) ? ('d') : (char)('x'+(i%4)))<<(i/4)+1;
		else if (i<19) filename<<"a"<<(char)('x'+(i-16));
		else if (i<169) filename<<"pvar"<<(i-19);
		else if (i<219) filename<<"psin"<<(i-169);
		else if (i==219) filename<<"fuel";
		else if (i==220) filename<<"pass";
		else filename<<"bagg";
		filename<<".dat";

		//Carrega os dados
		ps[i].load_data(filename.str().c_str(), SAMPLE_SIZE);
		//Liga o sensor ao sensor virtual certo
		if (i<16) ps[i].connect_to_virtsens(add.c_str(), port);
		else if (i<19) ps[i].connect_to_virtsens(add.c_str(), port+1);
		else if (i<219) ps[i].connect_to_virtsens(add.c_str(), port+2);
		else ps[i].connect_to_virtsens(add.c_str(), port+3);
	}

	/*
	Loop que itera por todos os sensores virtuais ate que todos
	tenham sido fechados
	*/
	while (alive_count > 0){
		for (int i=0; i<SENSOR_N && alive[i]; i++){
			if (ps[i].state==WAITING) ps[i].check_call();
			else if (ps[i].state==SENDING) ps[i].send_sample();
			else if (ps[i].state==DEAD){
				//Marca como fechado e decrementa a contagem de restantes
				alive[i] = false;
				alive_count--;
			}
		}

		//Espera para sincronizar com o numero de leituras por segundo
		usleep(1000000/OBS_PER_SEC);
	}
	
	for (int i=0; i<SENSOR_N; i++) ps[i].close_sensor();
	delete[] ps;
	delete[] alive;
	return 0;
}