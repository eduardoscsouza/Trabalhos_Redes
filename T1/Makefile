all:
	g++ socketstream.cpp physicalsensor.cpp -o physicalsensor.out -std=c++11
	g++ socketstream.cpp virtualsensor.cpp -o virtualsensor.out -std=c++11
	g++ datagenerator.cpp -o datagenerator.out -lm -std=c++11

clean:
	rm *.out *.dat