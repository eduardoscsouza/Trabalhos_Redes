#include <unistd.h>
#include <cmath>

#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
#include <string>

#include "socketstream.hpp"

#define LOOPBACK_ADDR "127.0.0.1"
#define BASEPORT 24666

#define OBS_PER_SEC 4
#define SEC_IN_DAY 86400
#define SAMPLE_SIZE (SEC_IN_DAY * OBS_PER_SEC)

#define SENSOR_N 4

#define PLANE_MAX_WEIGHT 300000
#define PERSON_WEIGHT_MIN 200
#define PERSON_WEIGHT_MED 700

using namespace std;



class VirtualSensor
{
public:
	Server server;
	size_t physen_count;
	vector<double> physen_data;
	function<vector<double>(vector<double>)> func;



	VirtualSensor()
	{
		this->server = Server();
		this->physen_count = 0;
		this->physen_data = vector<double>();
		this->func = function<vector<double>(vector<double>)>();
	}

	VirtualSensor(const char * ip_addr, unsigned short port)
	{
		this->server = Server(ip_addr, port);
		this->physen_count = 0;
		this->physen_data = vector<double>();
		this->func = function<vector<double>(vector<double>)>();
	}

	~VirtualSensor()
	{
		this->server.clear();
		this->physen_count = 0;
		this->physen_data = vector<double>();
		this->func = function<vector<double>(vector<double>)>();
	}



	void set_func(function<vector<double>(vector<double>)> func)
	{
		this->func = func;
	}

	vector<double> calculate()
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


//recebe a aceleracao da aeronave nos tres eixos e retorna a aceleracao resultante
vector<double> vect_trans(vector<double> vect)
{
	//cria o vetor que recebera as coordenadas polares do vetor aceleracao resultante
	vector<double> v = vector<double>(3);

	//calculo da coordenada polar R = sqrt(x^2 + y^2 + z^2)
	v[0] = sqrt(vect[0]*vect[0] + vect[1]*vect[1] + vect[2]*vect[2]);
	//calculo da coordenada polar theta = arctg(y/x)
	v[1] = atan2(vect[1], vect[0]);
	//calculo da coordenada polar fi = arccos(z/R)
	v[2] = acos(vect[2]/v[0]);

	return v;
}

//recebe a pressao embaixo do assento de todos os passageiros e retorna quantos estão sentados
vector<double> sum_thresh(vector<double> vect)
{
	//variavel para armazenar numero de passageiros
	double pass = 0;

	for(int i = 0; i < vect.size(); i++){
		if(vect[i] >= PERSON_WEIGHT_MIN){
			pass++;
		}
	}
	
	vector<double> v = vector<double>(1);
	v[0] = pass;
	return v;
}

//recebe a quantidade de combustivel no tanque (em kg), a quantidade de carga no deck (em kg) e o numero de passageiros que entraram no aviao
vector<double> sum_perc(vector<double> vect)
{
	//variavel que armazena o peso total calculado ate agora
	double total = 0;

	//multiplica a quantidade de pessoas pelo peso medio de um adulto e adiciona ao peso total calculado ate agora
	for(int i = 0; i < vect.size(); i++) total += vect[2]*PERSON_WEIGHT_MED;

	vector<double> v = vector<double>(1);
	v[0] = total/PLANE_MAX_WEIGHT;
	return v;
}

vector<double> local(vector<double> vect)
{
	vector<double> v = vector<double>(3);
	for(int i = 0; i < 3; i++){
		v[i] = vect[i];
	}
	return v;
}



int main(int argc, char * argv[])
{
	if (argc!=1 && argc!=3){
		cout<<"---USO---"<<endl<<"/virtualsensor.out"<<"OU"<<endl<<"/virtualsensor.out <ip> <porta>"<<endl;
		return 0;
	}
	string add = string((argc==3) ? argv[1] : LOOPBACK_ADDR);
	short port = (argc==3) ? atoi(argv[2]) : BASEPORT;

	VirtualSensor * vs = new VirtualSensor[SENSOR_N];
	for (int i=0; i<SENSOR_N; i++) vs[i] = VirtualSensor(add.c_str(), port+i);
	
	vs[0].accept_physen(16);
	vs[1].accept_physen(3);
	vs[2].accept_physen(200);
	vs[3].accept_physen(3);

	vs[0].set_func(&local);
	vs[1].set_func(&vect_trans);
	vs[2].set_func(&sum_thresh);
	vs[3].set_func(&sum_perc);

	bool verbose;
	cout<<endl<<"Deseja ver os dados que os sensores virtuais recebem, ou apenas o resultado?"<<endl<<"(1/0)"<<endl;
	cin>>verbose;

	cout<<endl<<"---USO---"<<endl<<"<NUMERO_DO_SENSOR> <NUMERO_DE_LEITURAS>"<<endl
	<<"<NUMERO_DO_SENSOR>==0 -> Fechar o Programa"<<endl<<endl
	<<"Sensor 1: Posicao GPS"<<endl
	<<"Sensor 2: Aceleracao"<<endl
	<<"Sensor 3: Numero de Passageiros Sentados"<<endl
	<<"Sensor 4: Porcentagem da Carga Máxima"<<endl;
	
	cout<<setprecision(4);
	int op = -1;
	size_t samples = 0;
	while (op!=0){
		cin>>op;
		if (op>0 && op<SENSOR_N){
			cin>>samples;
			vs[op-1].call_physen(samples);
			for (int i=0; i<samples; i++){
				vs[op-1].receive_sample();
				if (verbose){
					cout<<"Dados: ";
					for (int j=0; j<vs[op-1].physen_data.size(); j++) printf("%.4lf ", vs[op-1].physen_data[j]);
					cout<<endl;
				}

				vector<double> result = vs[op-1].calculate();
				cout<<"Resultado: ";
				for(int j=0; j<result.size(); j++) printf("%.4lf ", result[j]);
				cout<<endl;
				usleep(1000000/OBS_PER_SEC);
			}
		}
		else{
			if (op==0) for (int i=0; i<SENSOR_N; i++) vs[i].call_physen(0);
			else cout<<"Invalido"<<endl;
		}
	}

	usleep(2000000);
	for (int i=0; i<SENSOR_N; i++) vs[i].close_sensor();
	delete[] vs;
	return 0;
} 