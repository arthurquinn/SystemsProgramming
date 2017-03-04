all: client server	

client: client.c client.h
	gcc -o client client.c -pthread

server: server.c server.h sessioncommands.o sessioncommands.h
	gcc -o server server.c sessioncommands.o -pthread

sessioncommands.o: sessioncommands.c sessioncommands.h
	gcc -c sessioncommands.c

clean:
	rm client server *.o