
all: step_counter

step_counter: step_counter.c
	gcc -o step_counter step_counter.c

parser:
	g++ -o parser parser.cpp
	#gcc -c step_counter.c
	#g++ -o parser parser.o step_counter.o 

clean:
	rm step_counter parser