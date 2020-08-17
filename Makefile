.PHONY: clean

CC = gcc
RM = rm
CFLAGS += -fsanitize=address -fno-omit-frame-pointer -O1 -g -I./ -lpthread

EXE = try_lru
OBJS =$(patsubst %.c, %.o, $(wildcard *.c))

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) -rf $(EXE) $(OBJS)