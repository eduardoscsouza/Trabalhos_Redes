#include <cstdio>
#include <cmath>
#include <functional>
#include <vector>
#include <sstream>

#define RAND_PRECISION 1000000

#define OBS_PER_SEC 4
#define SEC_IN_DAY 86400
#define SAMPLE_SIZE (SEC_IN_DAY * OBS_PER_SEC)

using namespace std;



function<double(double)> linear(double a, double b) 
{
	return ([=](double x) -> double { return a*x + b; });
}

function<double(double)> quadratic(double a, double b, double c) 
{
	return ([=](double x) -> double { return a*x*x + b*x + c; });
}

function<double(double)> random(double min, double max) 
{
	long long int_min = (long long)(min * RAND_PRECISION), int_max = (long long)(max * RAND_PRECISION);
	return ([=](double x) -> double { return ((rand() % (int_max-int_min)) + int_min) / ((double)RAND_PRECISION); });
}

function<double(double)> sine(double freq, double ampl, double ang)
{
	return ([=](double x) -> double { return ampl*sin(freq*x + ang); });
}

function<double(double)> cosine(double freq, double ampl, double ang)
{
	return ([=](double x) -> double { return ampl*cos(freq*x + ang); });
}

function<double(double)> tangent(double freq, double ampl, double ang)
{
	return ([=](double x) -> double { return ampl*tan(freq*x + ang); });
}

function<double(double)> constant(double cons)
{
	return ([=](double x) -> double { return cons; });
}

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



void write_data(const char * filename, double * data, size_t data_count)
{
	FILE * data_file = fopen(filename, "w+");
	fwrite(data, data_count, sizeof(double), data_file);
	fclose(data_file);
}

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
	Funcoes que serão utilizadas nos sensores físicos
	Sao definidas semi-aleatoriamente
	
	write_data("x1", generate_data(), SAMPLE_SIZE);
	write_data("y1", data, SAMPLE_SIZE);
	write_data("z1", data, SAMPLE_SIZE);
	write_data("d1", data, SAMPLE_SIZE);
	write_data("", data, SAMPLE_SIZE);
	write_data("", data, SAMPLE_SIZE);
	write_data("", data, SAMPLE_SIZE);*/
	vector<function<double(double)> > funcs;
	vector<string> names;
	vector<function<double(double)> > com_funcs;
	com_funcs.push_back(constant(10));
	com_funcs.push_back(constant(10));
	funcs.push_back(comp_func(com_funcs));

	stringstream filename;
	double * data;
	for (int i=0; i<5; i++){
		data = generate_data(funcs[0], 0, SEC_IN_DAY, SAMPLE_SIZE);
		filename.str("");
		filename<<i<<".dat";

		write_data(filename.str().c_str(), data, SAMPLE_SIZE);
		free(data);
	}

	return 0;
}