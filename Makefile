CC = cc
CFLAGS = -Wall -Wextra -g

processflow: ./src/main.c
	$(CC) $(CFLAGS) src/main.c -o ./processflow


.PHONY: clean

clean:
	rm -f processflow


