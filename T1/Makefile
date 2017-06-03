#Eduardo Santos Carlos de Souza	9293481
#Fabrício Guedes Faria			9293522
#Guilherme Hideo Tubone			9019403
#Lucas de Oliveira Pacheco		9293182

all:
	g++ -c -std=c++11 -O3 socketstream.cpp -o socketstream.o
	g++ -std=c++11 -O3 socketstream.o physicalsensor.cpp -o physicalsensor.out
	g++ -lm -std=c++11 -O3 socketstream.o virtualsensor.cpp -o virtualsensor.out
	g++ -lm -std=c++11 -O3 datagenerator.cpp -o datagenerator.out

clean:
	rm -f *.o *.out *.dat ./data/*.dat

gen_data:
	mkdir -p data
	./datagenerator.out

run:
	sleep 1s && ./physicalsensor.out &
	./virtualsensor.out