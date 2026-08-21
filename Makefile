CC = cc
CFLAGS = -Wall -Wextra -g
TARGET = processflow
SRC = src/main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

.PHONY: clean run debug

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug:
	$(CC) $(CFLAGS) -fsanitize=address $(SRC) -o $(TARGET)
	./$(TARGET)
