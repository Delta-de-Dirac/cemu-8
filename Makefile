CC = clang
CCOPS = -std=gnu23 -O3 -o
CWARN = -Werror -Wall -Wextra -pedantic

INCLUDE =
INCLUDELIB =
LIBS = -lm -lraylib

all: cemu

cemu: cemu.c
	$(CC) $(CWARN) $(CCOPS) $@ $^ $(INCLUDE) $(INCLUDELIB) $(LIBS)

clean:
	rm -rf cemu