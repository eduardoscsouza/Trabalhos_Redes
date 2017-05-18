#include <iostream>
#include <unistd.h>

#include "socketstream.hpp"

using namespace std;

int main(int argc, char * argv[])
{
	Client c[10];
	for (int i=0; i<10; i++) c[i] = Client("127.0.0.1", 54666);

	char message[] = "hello from x";
	for (int i=0; i<10; i++){
		message[11] = '0' + i;
		c[i].send(message, sizeof(message));
		//usleep(1000000);
	}

	char response[] = "          ";
	for (int i=0; i<10; i++){
		c[i].receive(response, sizeof("hi x"));
		cout<<response<<endl;
	}
	
	for (int i=0; i<10; i++) c[i].close_client();
	return 0;
}