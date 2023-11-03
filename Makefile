cc := g++ -Wall -Werror -Wpedantic -std=c++20 -g -fsanitize=address

# OBJECT ARTIFACTS #
sources := $(shell find src/ -name *.cpp)
headers := $(shell find src/ -name *.h)
objects := $(patsubst src/%.cpp, target/obj/%.o, $(sources))

default: regexer

run: target/regexer
	@./$< "(a|b)*c" "abbabac"

$(objects): target/obj/%.o: src/%.cpp
	${cc} -c $< -o $@

regexer: $(objects)
	${cc} $^ -o target/$@

clean: 
	@rm -rf target/obj/*.o

purge: 
	@rm -rf target/