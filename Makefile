CC      = cc
CFLAGS  = -Wall -Wextra -std=c11 -g
TARGET  = processflow
SRC     = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

test: $(TARGET)
	@echo "sem testes ainda — issue 21"

clean:
	rm -f $(TARGET) *.o

.PHONY: all test clean
