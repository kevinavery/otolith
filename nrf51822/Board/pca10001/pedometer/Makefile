
all: step_counter parser

step_counter: step_counter.c
	gcc -o step_counter step_counter.c

parser: parser.cpp step_counter.c step_counter.h
	g++ -g -o parser parser.cpp

ss_test: parser
	./parser ${csv} ${ss}

ss_all:
	for csv in *.txt; do \
		make -s ss_test csv=$$csv ss=${ss}; \
	done

ss_loop:
	for number in 25 50 75 100 150 200; do \
		echo $$number:; \
		make -s ss_all ss=$$number; \
	done


clean:
	rm step_counter parser
