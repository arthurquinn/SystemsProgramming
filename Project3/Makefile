all: test0 test1 test2 test3 test4 test5 test6 test7 test8

test0: test0.c libmm.a
	gcc -o test0 test0.c libmm.a

test1: test1.c libmm.a
	gcc -o test1 test1.c libmm.a

test2: test2.c libmm.a
	gcc -o test2 test2.c libmm.a

test3: test3.c libmm.a
	gcc -o test3 test3.c libmm.a

test4: test4.c libmm.a
	gcc -o test4 test4.c libmm.a

test5: test5.c libmm.a
	gcc -o test5 test5.c libmm.a

test6: test6.c libmm.a
	gcc -o test6 test6.c libmm.a

test7: test7.c libmm.a
	gcc -o test7 test7.c libmm.a

test8: test8.c libmm.a
	gcc -o test8 test8.c libmm.a

mymalloc.o: mymalloc.c mymalloc.h
	gcc -c mymalloc.c

libmm.a: mymalloc.o
	ar r libmm.a mymalloc.o

clean:
	rm *.o *.a ./test0 ./test1 ./test2 ./test3 ./test4 ./test5 ./test6 ./test7 ./test8
