CC=gcc
CFLAGS=-I.

main: main.c crc_hash.c packet_encode.c
	$(CC) -o main main.c crc_hash.c packet_encode.c -I.

clean:
	rm -f ./*.o ./*~ 