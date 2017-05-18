#include <iostream>
#include <unistd.h>

#include "socketstream.hpp"

using namespace std;

int main(int argc, char * argv[])
{
	Server s = Server("127.0.0.1", 54666);
	s.accept_clients(10);

	char message[] = "                     ";
	for (int i=0; i<10; i++){
		s.receive(i, message, sizeof("hello from x"));
		cout<<message<<endl;
	}

	char response[] = "hi x";
	for (int i=0; i<10; i++){
		response[3] = '0' + i;
		s.send(i, response, sizeof("hi x"));
		//usleep(1000000);
	}

	s.close_server();
	return 0;
}