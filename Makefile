CC = clang
CWARN = -Werror -Wall -Wextra -pedantic

INCLUDE =
INCLUDELIB =
LIBS = -lm -lraylib

all: cemu

run: cemu
	./cemu ./c8games/GUESS

cemu: cemu.c
	$(CC) $(CWARN) -O3 -o $@ $^ $(INCLUDE) $(INCLUDELIB) $(LIBS)

clean:
	rm -rf cemu



