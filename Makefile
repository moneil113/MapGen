all: src/main.o src/Color.o src/Queue.o src/open-simplex-noise.o
	gcc -o MapGen src/main.c src/Color.o src/Queue.o src/open-simplex-noise.o

src/main.o: src/Color.o src/Queue.o src/open-simplex-noise.o
	cd src/ ; gcc -c main.c

src/Color.o: src/Color.c src/Color.h
	cd src/ ; gcc -c Color.c

src/Queue.o: src/Queue.c src/Queue.h
	cd src/ ; gcc -c Queue.c

src/open-simplex-noise.o: src/open-simplex-noise.c src/open-simplex-noise.h
	cd src/ ; gcc -c open-simplex-noise.c

clean:
	rm MapGen
	cd src/ ; rm main.o Color.o Queue.o open-simplex-noise.o
