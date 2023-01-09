all: compile

compile:
	gcc -c server.c
	gcc -o server server.o

run: compile
	./server -l Logfile -p 8001
