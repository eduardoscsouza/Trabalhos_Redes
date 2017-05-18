all:
	g++ socketstream.cpp client.cpp -o client.out -std=c++11
	g++ socketstream.cpp server.cpp -o server.out -std=c++11
	g++ datagenerator.cpp -o datagenerator.out -lm -std=c++11

clean:
	rm *.out