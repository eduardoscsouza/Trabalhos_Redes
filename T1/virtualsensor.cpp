/*
Eduardo Santos Carlos de Souza	9293481
Fabrício Guedes Faria			9293522
Gustavo Cabral					9293028
*/

#include <unistd.h>
#include <cmath>
#include <cstring>

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

#define PLANE_MAX_WEIGHT 3000000
#define PERSON_WEIGHT_MIN 200
#define PERSON_WEIGHT_MED 700

#define SQR(x) ((x)*(x))
#define RAD_TO_DEG(rad) ((rad)*(180.0/M_PI))

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
	v[0] = sqrt(SQR(vect[0]) +  SQR(vect[1]) + SQR(vect[2]));
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
	for(int i = 0; i < vect.size(); i++) if(vect[i] >= PERSON_WEIGHT_MIN) pass++;
	
	vector<double> v = vector<double>(1);
	v[0] = pass;
	return v;
}

//recebe a quantidade de combustivel no tanque (em kg), a quantidade de carga no deck (em kg) e o numero de passageiros que entraram no aviao
vector<double> sum_perc(vector<double> vect)
{
	vector<double> v = vector<double>(1);
	v[0] = (vect[0] + vect[1] + vect[2]*PERSON_WEIGHT_MED)/PLANE_MAX_WEIGHT;
	return v;
}

double ** gps_matrix(double ** sat_data)
{
	size_t i, j;
	double ** gps_mat = (double**) malloc(4*sizeof(double*));
	for (i=0; i<4; i++) gps_mat[i] = (double*) malloc(3*sizeof(double));

	for (i=0; i<3; i++) for (j=0; j<3; j++) gps_mat[i][j] = sat_data[0][j] - sat_data[i+1][j];

	double cons = SQR(sat_data[0][3]) - SQR(sat_data[0][0]) - SQR(sat_data[0][1]) - SQR(sat_data[0][2]);
	for (i=0; i<3; i++){
		gps_mat[3][i] = SQR(sat_data[i+1][3]) - cons;
		for (j=0; j<3; j++) gps_mat[3][i]-=SQR(sat_data[i+1][j]);
		gps_mat[3][i]/=2.0;
	}

	return gps_mat;
}

double * gauss_elim(double ** mat, double * res, size_t order)
{
	size_t i, j, k;
	double ** aux_mat = (double**) malloc(order*sizeof(double*));
	for (i=0; i<order; i++){
		aux_mat[i] = (double*) malloc(order*sizeof(double));
		memcpy(aux_mat[i], mat[i], order*sizeof(mat));
	}
	double * aux_res = (double*) malloc(order*sizeof(double));
	memcpy(aux_res, res, order*sizeof(double));
	
	double coef;
	for (j=0; j<order-1; j++){
		for (i=j+1; i<order; i++){
			coef = -aux_mat[i][j]/aux_mat[j][j];
			
			for (k=0; k<order; k++) aux_mat[i][k]+=coef*aux_mat[j][k];
			aux_res[i]+=coef*aux_res[j];
		}
	}

	double sum;
	double * sol = (double*) malloc(order*sizeof(double));
	for (i=order-1; i<order; i--){
		sum = aux_res[i];
		for (j=i+1; j<order; j++) sum-=sol[j]*aux_mat[i][j];

		sol[i] = sum/aux_mat[i][i];
	}

	for (i=0; i<order; i++) free(aux_mat[i]);
	free(aux_mat);
	free(aux_res);
	return sol;
}

vector<double> local(vector<double> vect)
{
	double ** mat = new double*[4];
	for (int i=0; i<4; i++) mat[i] = new double[4];
	for (int i=0; i<vect.size(); i++) mat[i/4][i%4] = vect[i];

	double ** gps_mat = gps_matrix(mat);
	for (int i=0; i<4; i++) delete[] mat[i];
	delete[] mat;
	
	double * p = gauss_elim(gps_mat, gps_mat[3], 3);
	vector<double> cart = vector<double>(3);
	for(int i=0; i<3; i++) cart[i] = p[i];

	for (int i=0; i<4; i++) free(gps_mat[i]);
	free(gps_mat);
	free(p);
	return vect_trans(cart);
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
	
	cout<<setprecision(8);
	int op = -1;
	size_t samples = 0;
	while (op!=0){
		cin>>op;
		if (op>0 && op<=SENSOR_N){
			cin>>samples;
			vs[op-1].call_physen(samples);
			for (int i=0; i<samples; i++){
				vs[op-1].receive_sample();
				if (verbose){
					cout<<"Dados: ";
					for (int j=0; j<vs[op-1].physen_data.size(); j++) cout<<vs[op-1].physen_data[j]<<" ";
					cout<<endl;
				}

				vector<double> result = vs[op-1].calculate();
				switch (op){
					case 1:
						cout<<RAD_TO_DEG(((M_PI/2.0)-result[1]))<<" N, "<<RAD_TO_DEG(result[2])<<" E"<<endl;
						break;

					case 2:
						cout<<result[0]<<" m/s^2, "<<RAD_TO_DEG(((M_PI/2.0)-result[1]))<<" Inclinacao, "<<RAD_TO_DEG(result[2])<<" Direcao"<<endl;
						break;

					case 3:
						cout<<result[0]<<" Passageiros Sentados"<<endl;
						break;

					case 4:
						cout<<result[0]<<"\%"<<endl;
						break;

				}
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