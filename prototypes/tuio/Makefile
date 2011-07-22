all: clutterspat

clutterspat: main.cpp
	g++ -Wall -g `pkg-config --cflags --libs spatosc-0.2 clutter-1.0 ` main.cpp -o clutterspat
clean:
	rm -f clutterspat *.o

