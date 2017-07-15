all: main.c libsl.a
	gcc -o sl main.c libsl.a

sorted-list.o: sorted-list.c sorted-list.h
	gcc -c sorted-list.c

libsl.a: sorted-list.o
	ar r libsl.a sorted-list.o

clean:
	rm sl *.o