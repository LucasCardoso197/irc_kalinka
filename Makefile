all:
	g++ socket_server.cpp -o server.out
	g++ socket_client.cpp -o client.out
run_server:
	./server.out
run_client:
	./client.out
clean:
	rm *.out