all: indexer.c

indexer.c: libindexer.a
	gcc -o indexer indexer.c libindexer.a

tokenizer.o: tokenizer.c tokenizer.h
	gcc -c tokenizer.c

sorted-list.o: sorted-list.c sorted-list.h
	gcc -c sorted-list.c

libindexer.a: tokenizer.o sorted-list.o
	ar r libindexer.a tokenizer.o sorted-list.o

clean: 
	rm indexer *.o *.a