all: compile

compile:
	gcc -c lightclient.c
	gcc -o client lightclient.o

run: compile
	./client -s 34.123.21.190 -l Logfile -p 8001
