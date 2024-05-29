cc := g++ 
debug_flags := -Wall -Werror -Wpedantic -Wextra -std=c++20 -g -fsanitize=address
release_flags := -O2 -Wall -Werror -Wpedantic -Wextra -Werror -std=c++20 -fsanitize=address


# KCONFIG #
CONFIG := .config
include $(CONFIG)

ifeq ($(CONFIG_DEBUG), y)
cflags := $(debug_flags)
endif

ifeq ($(CONFIG_RELEASE), y)
cflags := $(release_flags)
endif

ifneq ($(CONFIG_LOG_LEVEL),LOG_LEVEL_NONE)
cflags += -D$(CONFIG_LOG_LEVEL)
endif

src := $(shell find src/ -name *.cpp)
obj := $(patsubst src/%.cpp, target/obj/%.o, $(src))


exe := target/regexer

run: $(exe)
	@./$< "(a|b)*c" "abbabac"

$(obj): target/obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(cc) $(cflags) -c $< -o $@

$(exe): $(obj)
	$(cc) $(cflags) $^ -o $@

.PHONY: clean
clean: 
	rm -rf target

.PHONY: menuconfig
menuconfig: Kconfig
	kconfig-mconf $<
