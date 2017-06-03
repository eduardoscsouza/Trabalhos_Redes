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

#define SENSOR_N 5

#define PLANE_MAX_WEIGHT 3000000
#define PERSON_WEIGHT_MIN 200
#define PERSON_WEIGHT_MED 700

#define SQR(x) ((x)*(x))
#define RAD_TO_DEG(rad) ((rad)*(180.0/M_PI))

using namespace std;



/*
Classe do sensor virtual do trabalho
*/
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



	/*
	Define a funcao que o sensor virtual aplicara nos dados recebidos
	*/
	void set_func(function<vector<double>(vector<double>)> func)
	{
		this->func = func;
	}

	/*
	Aplica a funcao definida
	*/
	vector<double> calculate()
	{
		return this->func(this->physen_data);
	}

	/*
	Linka o sensor a um servidor
	*/
	void bind_to_server(const char * ip_addr, unsigned short port)
	{
		this->server.bind_to_server(ip_addr, port);
	}

	/*
	Conecta o sensor a physen_count sensores virtuais
	*/
	void accept_physen(size_t physen_count)
	{
		this->server.accept_clients(physen_count);
		this->physen_count = physen_count;
		this->physen_data = vector<double>(physen_count);
	}

	/*
	Envia para os sensores fisicos um bool=true e um inteiro que diz quantas leituras
	o sensor virtual quer que sejam enviadas a ele. Caso esse numero seja 0, significa
	que o sensor fisico deve fechar.
	*/
	void call_physen(size_t samples)
	{
		bool call = true;
		for (int i=0; i<this->physen_count; i++){
			this->server.send(i, &call, sizeof(bool));
			this->server.send(i, &samples, sizeof(size_t));
		}
	}

	/*
	Armazena as leituras enviadas pelos sensores fisicos
	*/
	void receive_sample()
	{
		for (int i=0; i<this->physen_count; i++)
			this->server.receive(i, &(this->physen_data[i]), sizeof(double));
	}

	/*
	Fecha o sensor virtual
	*/
	void close_sensor()
	{
		this->server.close_server();
	}
};


/*
Recebe um vetor em coordenadas cartesianas e retorna
em coordenadas polares
*/
vector<double> vect_trans(vector<double> vect)
{
	//cCia o vetor que recebera as coordenadas polares
	vector<double> v = vector<double>(3);

	//Calculo da coordenada polar R = sqrt(x^2 + y^2 + z^2)
	v[0] = sqrt(SQR(vect[0]) +  SQR(vect[1]) + SQR(vect[2]));
	//Calculo da coordenada polar theta = arctg(y/x)
	v[1] = atan2(vect[1], vect[0]);
	//Calculo da coordenada polar phi = arccos(z/R)
	v[2] = acos(vect[2]/v[0]);

	return v;
}

/*
Recebe o sinal emitido pelos celulares da tripulacao e retorna o total do ruido
*/
vector<double> sum(vector<double> vect)
{
	double noise = 0;
	for(int i=0; i<vect.size(); i++) noise+=vect[i];

	vector<double> v = vector<double>(1);
	v[0] = noise;
	return v;
}

/*
Recebe a pressao embaixo do assento de todos os passageiros e retorna quantos estão sentados
*/
vector<double> sum_thresh(vector<double> vect)
{
	//variavel para armazenar numero de passageiros
	double pass = 0;
	for(int i=0; i<vect.size(); i++) if(vect[i] >= PERSON_WEIGHT_MIN) pass++;
	
	vector<double> v = vector<double>(1);
	v[0] = pass;
	return v;
}

/*
Recebe a quantidade de combustivel no tanque, a quantidade de carga no deck
e o numero de passageiros que entraram no aviao, e retorna a porcentagem da carga maxima do
aviao
*/
vector<double> sum_perc(vector<double> vect)
{
	vector<double> v = vector<double>(1);
	v[0] = (vect[0] + vect[1] + (vect[2]*PERSON_WEIGHT_MED))/PLANE_MAX_WEIGHT;
	return v;
}

/*
sat_data
{
	{x1, y1, z1, d1},
	{x2, y2, z2, d2},
	{x3, y3, z3, d3},
	{x4, y4, z4, d4}
}

gps_mat
{
	{A1, A2, A3},
	{A4, A5, A6},
	{A7, A8, A9},
	{b1, b2, b3}

	do sistema Ax = b, onde x=(Px, Py, Pz); P => a posicao que quero encontrar
}

*/

/*
Recebe os dados da matriz dos satelites, com a posicao dos 4 satelites
e a distancia do ponto aos satelites, e gera uma matriz de um sistema linear
de 3 equacoes cuja solucao e a posição do ponto. 
*/
double ** gps_matrix(double ** sat_data)
{
	size_t i, j;
	double ** gps_mat = new double*[4];
	for (i=0; i<4; i++) gps_mat[i] = new double[3];

	for (i=0; i<3; i++) for (j=0; j<3; j++) gps_mat[i][j] = sat_data[0][j] - sat_data[i+1][j];

	double cons = SQR(sat_data[0][3]) - SQR(sat_data[0][0]) - SQR(sat_data[0][1]) - SQR(sat_data[0][2]);
	for (i=0; i<3; i++){
		gps_mat[3][i] = SQR(sat_data[i+1][3]) - cons;
		for (j=0; j<3; j++) gps_mat[3][i]-=SQR(sat_data[i+1][j]);
		gps_mat[3][i]/=2.0;
	}

	return gps_mat;
}

/*
Resolve um sistema linear por eliminacao gaussiana e retorna a solucao
*/
double * gauss_elim(double ** mat, double * res, size_t order)
{
	size_t i, j, k;
	double ** aux_mat = new double*[order];
	for (i=0; i<order; i++){
		aux_mat[i] = new double[order];
		memcpy(aux_mat[i], mat[i], order*sizeof(mat));
	}
	double * aux_res = new double[order];
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
	double * sol = new double[order];
	for (i=order-1; i<order; i--){
		sum = aux_res[i];
		for (j=i+1; j<order; j++) sum-=sol[j]*aux_mat[i][j];

		sol[i] = sum/aux_mat[i][i];
	}

	for (i=0; i<order; i++) delete[] aux_mat[i];
	delete[] aux_mat;
	delete[] aux_res;
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
	
	double * point = gauss_elim(gps_mat, gps_mat[3], 3);
	vector<double> cart = vector<double>(3);
	for(int i=0; i<3; i++) cart[i] = point[i];
	delete[] point;

	for (int i=0; i<4; i++) delete[] gps_mat[i];
	delete[] gps_mat;
	//Transforma em polares para dar em graus N e graus E
	return vect_trans(cart);
}



int main(int argc, char * argv[])
{
	VirtualSensor * vs = new VirtualSensor[SENSOR_N];
	for (int i=0; i<SENSOR_N; i++) vs[i] = VirtualSensor(LOOPBACK_ADDR, BASEPORT+i);
	
	vs[0].accept_physen(16);
	vs[1].accept_physen(3);
	vs[2].accept_physen(200);
	vs[3].accept_physen(3);
	vs[4].accept_physen(10);

	vs[0].set_func(&local);
	vs[1].set_func(&vect_trans);
	vs[2].set_func(&sum_thresh);
	vs[3].set_func(&sum_perc);
	vs[4].set_func(&sum);

	bool verbose;
	cout<<endl<<"Deseja ver os dados que os sensores virtuais recebem, ou apenas o resultado?"<<endl<<"(1/0)"<<endl;
	cin>>verbose;

	cout<<endl<<"---USO---"<<endl<<"<NUMERO_DO_SENSOR> <NUMERO_DE_LEITURAS>"<<endl
	<<"<NUMERO_DO_SENSOR>==0 -> Fechar o Programa"<<endl<<endl
	<<"Sensor 1: Posicao GPS"<<endl
	<<"Sensor 2: Aceleracao"<<endl
	<<"Sensor 3: Numero de Passageiros Sentados"<<endl
	<<"Sensor 4: Porcentagem da Carga Máxima"<<endl
	<<"Sensor 5: Ruído gerado pelos celulares"<<endl;
	
	cout<<setprecision(8);
	int op = -1;
	size_t samples = 0;
	while (op!=0){
		cin>>op;
		if (op>0 && op<=SENSOR_N){
			cin>>samples;
			//Pede que os sensores fisicos mandem os dados
			if(samples>0) vs[op-1].call_physen(samples);
			for (int i=0; i<samples; i++){
				//Le os dados
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

					case 5:
						cout<<result[0]<<" dB"<<endl;
						break;

				}

				//Espera para sincronia com o tempo de leitura
				usleep(1000000/OBS_PER_SEC);
			}
		}
		else{
			//Fecha os sensores fisicos
			if (op==0) for (int i=0; i<SENSOR_N; i++) vs[i].call_physen(0);
			else cout<<"Invalido"<<endl;
		}
	}

	//Espera para que os sensores fisicos ja tenham fechado, antes de fechar o virtual
	usleep(2000000);
	for (int i=0; i<SENSOR_N; i++) vs[i].close_sensor();
	delete[] vs;
	return 0;
} 