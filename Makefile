all:
	g++ socket_server.cpp -o server
	g++ socket_client.cpp -o client
run_server:
	./server
run_client:
	./client