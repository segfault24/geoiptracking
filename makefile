main.out: main.o dict.o
	gcc main.o dict.o -lpcap -lGeoIP -o main.out

main.o: main.c
	gcc -c main.c -o main.o

dict.o: dict.c
	gcc -c dict.c -o dict.o

clean:
	rm main *.o
