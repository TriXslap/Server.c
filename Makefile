CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic
CPPFLAGS = -Iinclude
OBJS = server.o request.o response.o

server: $(OBJS)
	$(CC) $(OBJS) -o server

server.o: src/server.c include/http.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

request.o: src/request.c include/http.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

response.o: src/response.c include/http.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJS)
