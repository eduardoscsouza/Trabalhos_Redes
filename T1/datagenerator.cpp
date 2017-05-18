#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <functional>
#include <vector>
#include <string>

#define RAND_PRECISION 1000000

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



void write_data(const char * filename, double * data, size_t data_count)
{
	FILE * data_file = fopen(filename, "w+");
	fwrite(data, data_count, sizeof(double), data_file);
	fclose(data_file);
}

double * generate_data(function<double(double)> func, double t0, double tf, size_t samples)
{
	double * data = (double*) malloc(samples * sizeof(double));

	double x=t0, step = (tf - t0) / (samples - 1);
	for (size_t i=0; i<samples; i++){
		data[i] = func(x);
		x+=step;
	}

	return data;
}



int main(int argc, char * argv[])
{
	vector<function<double(double)> > funcs;
	funcs.push_back(linear(2, 10));
	funcs.push_back(quadratic(1, -6, 6));
	funcs.push_back(random(0, 10000));
	funcs.push_back(sine(2, 10, 0));
	funcs.push_back(cosine(2, 10, 0));
	funcs.push_back(tangent(2, 10, 0));

	double * data;
	string filename = "";
	for (int i=0; i<funcs.size(); i++){
		data = generate_data(funcs[i], 0, 1, 10000);
		write_data((filename + ((char)('0'+i)) + ".dat").c_str(), data, 10000);
		free(data);
	}

	return 0;
}