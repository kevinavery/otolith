
all: step_counter parser

step_counter: step_counter.c
	gcc -o step_counter step_counter.c

parser: parser.cpp step_counter.c step_counter.h
	g++ -g -o parser parser.cpp

clean:
	rm step_counter parser