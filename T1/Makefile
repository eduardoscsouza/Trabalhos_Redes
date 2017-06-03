#Eduardo Santos Carlos de Souza	9293481
#Fabrício Guedes Faria			9293522
#Gustavo Cabral					9293028

all:
	g++ socketstream.cpp -o socketstream.o -c -std=c++11 -O3
	g++ socketstream.o physicalsensor.cpp -o physicalsensor.out -std=c++11 -O3
	g++ socketstream.o virtualsensor.cpp -o virtualsensor.out -lm -std=c++11 -O3
	g++ datagenerator.cpp -o datagenerator.out -lm -std=c++11 -O3

clean:
	rm -f *.o *.out *.dat ./data/*.dat

gen_data:
	mkdir -p data
	./datagenerator.out

run:
	sleep 1s && ./physicalsensor.out &
	./virtualsensor.out