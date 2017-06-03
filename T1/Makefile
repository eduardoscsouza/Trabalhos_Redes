#Eduardo Santos Carlos de Souza	9293481
#Fabrício Guedes Faria			9293522
#Gustavo Cabral					9293028

all:
	g++ socketstream.cpp -o socketstream.o -c -std=c++11
	g++ socketstream.o physicalsensor.cpp -o physicalsensor.out -std=c++11
	g++ socketstream.o virtualsensor.cpp -o virtualsensor.out -lm -std=c++11
	g++ datagenerator.cpp -o datagenerator.out -lm -std=c++11

clean:
	rm *.o *.out *.dat

run:
	./datagenerator.out
	sleep 1s && ./physicalsensor.out &
	./virtualsensor.out