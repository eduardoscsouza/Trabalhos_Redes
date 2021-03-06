/*
Eduardo Santos Carlos de Souza	9293481
Fabrício Guedes Faria			9293522
Guilherme Hideo Tubone			9019403
Lucas de Oliveira Pacheco		9293182
*/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <functional>
#include <vector>
#include <string>
#include <sstream>

#define RAND_PRECISION 1000000.0
#define RAND_RANGE(MIN, MAX) ((MIN) + (((MAX)-(MIN))*((rand() % (int)RAND_PRECISION)/(RAND_PRECISION-1.0))))

#define OBS_PER_SEC 4
#define SEC_IN_DAY 86400
#define SAMPLE_SIZE (SEC_IN_DAY * OBS_PER_SEC)

using namespace std;



/*
Funcao que gera uma funcao linear com os atributos dados
*/
function<double(double)> linear(double a, double b) 
{
	return ([=](double x) -> double { return a*x + b; });
}

/*
Funcao que gera uma funcao seno com os atributos dados
*/
function<double(double)> sine(double freq, double ampl, double ang)
{
	return ([=](double x) -> double { return ampl*sin(freq*x + ang); });
}

/*
Funcao que gera uma funcao constante com os atributos dados
*/
function<double(double)> constant(double cons)
{
	return ([=](double x) -> double { return cons; });
}

/*
Funcao que gera uma funcao composta pela somatoria das funcoes do vetor funcs
*/
function<double(double)> comp_func(vector<function<double(double)> > funcs)
{
	return
	([=](double x) -> double
	{
		double out = 0;
		for (int i=0; i<funcs.size(); i++) out+=funcs[i](x);

		return out; 
	});
}



/*
Funcao que armazena o vetor de doubles em um arquivo
*/
void write_data(const char * filename, double * data, size_t data_count)
{
	FILE * data_file = fopen(filename, "w");
	fwrite(data, data_count, sizeof(double), data_file);
	fclose(data_file);
}

/*
Funcao que aplica a funcao func em samples pontos durante os tempos t0 e tf, e retorna
um vetor com os resultados
*/
double * generate_data(function<double(double)> func, double t0, double tf, size_t samples)
{
	double * data = new double[samples];

	double x=t0, step = (tf - t0) / (samples - 1);
	for (size_t i=0; i<samples; i++){
		data[i] = func(x);
		x+=step;
	}

	return data;
}



int main(int argc, char * argv[])
{
	/*
	Funcoes que serão utilizadas para os dados dos sensores físicos
	Sao definidas previamente e semi-aleatoriamente
	*/
	vector<function<double(double)> > funcs;
	vector<string> names;

	//Funcoes para o GPS
	stringstream filename;
	for (int i=0; i<16; i++){
		filename.str("");
		filename<<((i%4==3) ? ('d') : (char)('x'+(i%4)))<<(i/4)+1;
		names.push_back(filename.str());

		if (i%4==3) funcs.push_back(sine(RAND_RANGE(0.000001, 0.000050), RAND_RANGE(500000, 1000000), 0));
		else funcs.push_back(constant(RAND_RANGE(6380000, 6400000)));
	}

	//Funcoes de aceleracao no plano xy
	for (int i=0; i<2; i++){
		filename.str("");
		filename<<"a"<<((char)('x'+i));
		names.push_back(filename.str());
		funcs.push_back(sine(RAND_RANGE(0.005, 0.0010), RAND_RANGE(1.0, 30.0), 0));
	}
	//Funcao de aceleracao do eixo z
	names.push_back("az");
	vector<function<double(double)> > com_funcs;
	com_funcs.push_back(constant(-9.8));
	com_funcs.push_back(sine(RAND_RANGE(0.005, 0.0010), RAND_RANGE(1.0, 30.0), 0));
	funcs.push_back(comp_func(com_funcs));

	//Funcoes de pressao dos passageiros que se mantem sentados durante todo o voo
	for (int i=0; i<150; i++){
		filename.str("");
		filename<<"pvar"<<i;
		names.push_back(filename.str());

		com_funcs.clear();
		com_funcs.push_back(constant(RAND_RANGE(600, 800)));
		com_funcs.push_back(sine(RAND_RANGE(3000000.0, 3500000.0), RAND_RANGE(1, 50), 0));
		funcs.push_back(comp_func(com_funcs));
	}

	//Funcao de pressai para os passageiros que se levantam
	for (int i=0; i<50; i++){
		filename.str("");
		filename<<"psin"<<i;
		names.push_back(filename.str());

		com_funcs.clear();
		int val = RAND_RANGE(400, 900);
		com_funcs.push_back(constant(val));
		com_funcs.push_back(sine(RAND_RANGE(0.02, 0.2), val, 0));
		funcs.push_back(comp_func(com_funcs));
	}
	
	//Funcao da carga total do aviao
	names.push_back("fuel");
	funcs.push_back(linear(RAND_RANGE(-0.1, -0.5), 100000));
	names.push_back("pass");
	funcs.push_back(constant((int)RAND_RANGE(100, 200)));
	names.push_back("bagg");
	funcs.push_back(constant(RAND_RANGE(300000, 380000)));

	//Funcoes do sinal emitido pelos celulares
	for (int i=0; i<10; i++){
		filename.str("");
		filename<<"cel"<<i;
		names.push_back(filename.str());

		com_funcs.clear();
		double val = RAND_RANGE(0.01, 0.1);
		com_funcs.push_back(constant(1.1*val));
		com_funcs.push_back(sine(RAND_RANGE(3000.0, 5000.0), val, 0));
		com_funcs.push_back(sine(RAND_RANGE(100000.0, 300000.0), 0.1*val, 0));
		funcs.push_back(comp_func(com_funcs));
	}

	//Aplicar as funcoes e guardar o resultado
	double * data;
	for (int i=0; i<funcs.size(); i++){
		data = generate_data(funcs[i], 0, SEC_IN_DAY, SAMPLE_SIZE);

		filename.str("");
		filename<<"./data/"<<names[i]<<".dat";
		write_data(filename.str().c_str(), data, SAMPLE_SIZE);
		delete[] data;
	}

	return 0;
}