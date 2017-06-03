#Eduardo Santos Carlos de Souza	9293481
#Fabrício Guedes Faria			9293522
#Gustavo Cabral					9293028

all:
	g++ -c -std=c++14 -O3 socketstream.cpp  -o socketstream.o
	g++ -std=c++14 -O3 socketstream.o physicalsensor.cpp -o physicalsensor.out
	g++ -lm -std=c++14 -O3 socketstream.o virtualsensor.cpp -o virtualsensor.out
	g++ -lm -std=c++14 -O3 datagenerator.cpp -o datagenerator.out

clean:
	rm -f *.o *.out *.dat ./data/*.dat

gen_data:
	mkdir -p data
	./datagenerator.out

run:
	sleep 1s && ./physicalsensor.out &
	./virtualsensor.out