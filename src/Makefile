cc := g++ -Wall -Werror -Wpedantic -std=c++20 -g -fsanitize=address

# OBJECT ARTIFACTS #
obj := tokenizer.o parser.o regex.o main.o

default: regexer

run: regexer
	@./$< "(a|b)*c" "abbabac"

%.o: %.cpp regexer.h
	${cc} -c $<

regexer: ${obj}
	${cc} $^ -o $@

clean: 
	@rm -rf *.o

purge: 
	@rm -rf *.o regexer