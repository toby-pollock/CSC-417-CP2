interpreter: main2.o
	gcc main2.o -o interpreter -lcjson

main2.o: main2.c
	gcc -c main2.c -o main2.o