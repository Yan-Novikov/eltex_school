all: chat_server chat_client

chat_server: server_main.c server.c common.c server.h common.h
	gcc -o chat_server server_main.c server.c common.c

chat_client: client_main.c client.c common.c client.h common.h
	gcc -o chat_client client_main.c client.c common.c

clean:
	rm -f chat_server chat_client *.o

distclean: clean
	rm -f chat_server