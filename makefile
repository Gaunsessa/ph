CC = gcc
INCLUDES = -I src/ -I include/

build: src/*
	$(CC) -g src/*.c include/utf8proc/utf8proc.c $(INCLUDES) -fsanitize=address -o build/ph

run:
	make -B
	./build/ph run tests/test.ph