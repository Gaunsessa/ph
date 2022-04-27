CC = gcc
INCLUDES = -I src/ -I include/

build: src/*
	$(CC) -g src/*.c $(INCLUDES) -fsanitize=address -o build/ph

run:
	make
	./build/ph