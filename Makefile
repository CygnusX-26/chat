CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC = src/chat.c src/user.c src/utils.c
OUT = out/chat

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf out

.PHONY: all clean
