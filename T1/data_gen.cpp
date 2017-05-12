#include <iostream>
#include <cstdlib>
#include <cstdio>

using namespace std;

int main(int argc, char * argv[])
{
	double * data = (double *) malloc(20 * sizeof(double));
	for (int i=0; i<20; i++)data[i] = i/20.0;
	
	FILE * data_file = fopen("linear.dat", "w+");
	fwrite(data, 20, sizeof(double), data_file);
	fclose(data_file);
	free(data);

	return 0;
}