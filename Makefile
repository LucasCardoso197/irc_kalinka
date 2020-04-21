all:
	g++ socket_server.cpp -lpthread -o server.out
	g++ socket_client.cpp -lpthread -o client.out
run_server:
	./server.out
run_client:
	./client.out
clean:
	rm *.out