all: compile

compile:
	gcc -c lightserver.c
	gcc -o server lightserver.o

run: compile
	./server -l Logfile -p 8001
