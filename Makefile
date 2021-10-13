all:
	gcc server.c -o server -lrt -lpthread
	gcc client.c -o client -lrt -lpthread
	gcc chat.c -o chat -lrt -lpthread
debug:
	gcc server.c -o server -lrt -lpthread -g
	gcc client.c -o client -lrt -lpthread -g
	gcc chat.c -o chat -lrt -lpthread -g
clean:
	rm server
	rm client
	rm chat
