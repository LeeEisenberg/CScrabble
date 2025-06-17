.PHONY: setup compile run server client clean

all: compile run

setup: 
	test -d "./output" || mkdir output 

compile: setup client.out server.out main.out

client.out: client/client.c client/client.h
	gcc -o output/client.out client/client.c -lncurses

server.out: server/server.c server/server.h
	gcc -o output/server.out server/server.c -lncurses

main.out: main.c
	gcc -o output/main.out main.c -lncurses

server: server.out
	./output/server.out 

client: client.out
	./output/client.out 127.0.0.1 25595

run: main.out
	./output/main.out

clean: 
	test -d "./output" && rm -r ./output