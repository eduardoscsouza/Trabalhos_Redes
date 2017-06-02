#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <functional>
#include <vector>

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
	double * data = (double*) malloc(sizeof(double)*samples);

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
	Sao definidas semi-aleatoriamente
	*/
	vector<function<double(double)> > funcs;
	vector<string> names;
/*
	stringstream filename;
	for (int i=0; i<)*/

	funcs.push_back(constant(237688713.213));
	names.push_back("x1");
	funcs.push_back(constant(6356576.324));
	names.push_back("y1");
	funcs.push_back(constant(37132141.123));
	names.push_back("z1");
	funcs.push_back(constant(14365465.341));
	names.push_back("x2");
	funcs.push_back(constant(18367847.145));
	names.push_back("y2");
	funcs.push_back(constant(9344214.644));
	names.push_back("z2");
	funcs.push_back(constant(84321312.500));
	names.push_back("x3");
	funcs.push_back(constant(233219485.64));
	names.push_back("y3");
	funcs.push_back(constant(9865445.245));
	names.push_back("z3");
	funcs.push_back(constant(84432645245.234));
	names.push_back("x4");
	funcs.push_back(constant(06456395.235));
	names.push_back("y4");
	funcs.push_back(constant(9854335653.845));
	names.push_back("z4");
	funcs.push_back(sine(0.0000000003, 412321321.321, 0));
	names.push_back("d1");
	funcs.push_back(sine(0.0000000012, 2123321.311, 0));
	names.push_back("d2");
	funcs.push_back(sine(0.0000000043, 512376521.421, 0));
	names.push_back("d3");
	funcs.push_back(sine(0.0000000103, 5421321321.321, 0));
	names.push_back("d4");

	funcs.push_back(sine(0.00763, 11.820, 0));
	names.push_back("ax");
	funcs.push_back(sine(0.103, 2.32, 0));
	names.push_back("ay");
	funcs.push_back(constant(9.8));
	names.push_back("az");

	vector<function<double(double)> > com_funcs;
	com_funcs.push_back(constant(200));
	com_funcs.push_back(sine(3212645.65, 0.5, 0));
	funcs.push_back(comp_func(com_funcs));
	names.push_back("pvar");
	com_funcs.clear();
	com_funcs.push_back(constant(700));
	com_funcs.push_back(sine(0.0000005, 700, 0));
	funcs.push_back(comp_func(com_funcs));
	names.push_back("psin");

	funcs.push_back(linear(-0.02, 20000));
	names.push_back("fuel");
	funcs.push_back(constant(180));
	names.push_back("pass");
	funcs.push_back(constant(100000));
	names.push_back("bagg");

	double * data;
	for (int i=0; i<funcs.size(); i++){
		data = generate_data(funcs[i], 0, SEC_IN_DAY, SAMPLE_SIZE);

		write_data((names[i] + ".dat").c_str(), data, SAMPLE_SIZE);
		free (data);
	}

	return 0;
}