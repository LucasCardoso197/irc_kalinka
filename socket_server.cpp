#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string>
#include <iostream>
#include <chrono>
#include <future>
#include <poll.h>
#include <string.h>

// Settings
#define PORT 8080 
#define MESSAGE_SIZE 4096
#define LOOP_INTERVAL 100
#define MAX_CLIENTS 30
// Códigos
#define S_CONNECTION 1
#define S_MESSAGE 2

#define DC_MESSAGE 1
#define TXT_MESSAGE 2

class Server
{
private:
	int connection_fd, client_socket[MAX_CLIENTS];
	struct sockaddr_in address;
	fd_set readfds;
public:
	Server();
	~Server();
	int setup();
	int selectSocket();
	int connectToNewClient();
	int sendMessage(std::string message, int socket_fd);
	void broadcastMessage(std::string message);
	int readMessage(void *buffer, int *clientIndex);
};

// function to get user input asynchronously
std::string WaitInput() {
	std::string line;
	std::getline(std::cin, line);
	return line;
}

int main(int argc, char const *argv[]){
	int max, activityType, messageType, i=0;
	char msg_buffer[MESSAGE_SIZE];

	// create and setup the server
	Server s;
	if(s.setup() == -1) return -1;
	
	
	while (true){
		activityType = s.selectSocket();
		if(activityType == S_CONNECTION){
			// Receive connection
			s.connectToNewClient();
		}
		else if(activityType == S_MESSAGE){
			// Receive message
			messageType = s.readMessage(msg_buffer, &i);
            if(messageType != DC_MESSAGE) {
				std::cout << "User " << i << ": " << msg_buffer << std::endl;
				s.broadcastMessage("User " + std::to_string(i) + ": " + msg_buffer);
            }
		}
	}

	return 0; 
}

// Methods definitions
Server::Server(){
	connection_fd = 0;

	for(int i=0; i<MAX_CLIENTS; i++)
		client_socket[i] = 0;
}

Server::~Server(){
	if(connection_fd != 0)
		close(connection_fd);
	for(int i=0; i<MAX_CLIENTS; i++)
		if(client_socket[i] != 0)
			close(client_socket[i]);
}

int Server::setup(){
	// Creating socket file descriptor 
	if ((connection_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		std::cerr << "Socket creation error" << std::endl; 
		return -1; 
	}

	// Setting some socket options
	int opt = true;
	if (setsockopt(connection_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *)&opt, sizeof(opt)) < 0){
		std::cerr << "Error configuring socket." << std::endl;
		close(connection_fd);
		connection_fd = 0; 
		return -1;
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	   
	// Forcefully attaching socket to the port 8080 
	if (bind(connection_fd, (struct sockaddr *)&address, sizeof(address))<0){ 
		std::cerr << "Error binding to port." << std::endl;
		close(connection_fd);
		connection_fd = 0;
		return -1;
	}
	std::cout << "Listening on PORT " << PORT << std::endl;

	// Setting connection_fd as a socket that listen for connections
	if(listen(connection_fd, 3) < 0){   
		perror("listen");
		exit(EXIT_FAILURE);
	} 
	std::cout << "Waiting for connections..." << std::endl << std::endl;

	return 0;
}

int Server::selectSocket(){
	int sd, max_sd, activity;

	if(connection_fd == 0){
		std::cerr << "Please first run the setup method." << std::endl;
		return -1;
	}

	// Preparing fdset
	FD_ZERO(&readfds);
	FD_SET(connection_fd, &readfds);
	max_sd = connection_fd;

	for(int i=0; i<MAX_CLIENTS; i++){
		sd = client_socket[i];
		if(sd > 0)
			FD_SET(sd, &readfds);
		
		if(sd > max_sd)
			max_sd = sd;
	}

	// Running select
	activity = select(max_sd+1, &readfds, nullptr, nullptr, nullptr);
	if((activity < 0) && (errno!=EINTR)){
		std::cerr << "Error during select." << std::endl;
		return -2;
	}
	
	if(FD_ISSET(connection_fd, &readfds))
		return S_CONNECTION;
	else
		return S_MESSAGE;
}

int Server::connectToNewClient(){
	int new_socket;
	int addr_len = sizeof(address);
	if((new_socket = accept(connection_fd, (struct sockaddr *)&address, (socklen_t *)&addr_len)) < 0){
		std::cerr << "Error accepting connection." << std::endl;
		return -1;
	}

	std::cout << "New connection at IP: " << inet_ntoa(address.sin_addr) << std::endl;
	std::cout << "At port " << ntohs(address.sin_port) << " and socket " << new_socket << std::endl;

	// Sending welcome message
	if(this->sendMessage("Welcome to IRC Kalinka!!", new_socket) <= 0)
		std::cerr << "Error sending welcome message." << std::endl;

	// Adding new connected socket to the list
	for(int i=0; i<MAX_CLIENTS; i++){
		if(client_socket[i] == 0){
			client_socket[i] = new_socket;
			std::cout << "Added new connection at " << i << std::endl << std::endl;
			break;
		}
	}

	return 0;
}

int Server::sendMessage(std::string message, int socket_fd){
	return send(socket_fd, message.c_str(), message.length(), 0);
}

void Server::broadcastMessage(std::string message) {
	for(int i=0; i<MAX_CLIENTS; i++){
		if(client_socket[i] != 0){
			this->sendMessage(message, client_socket[i]);
		}
	}
}

int Server::readMessage(void *buffer, int *clientIndex){
	int sd, valread;
	struct sockaddr addr;
	int addrlen = sizeof(addr);
	char *aux = (char *)buffer;

	for(int i=0; i<MAX_CLIENTS; i++){
		sd = client_socket[i];
		if(FD_ISSET(sd, &readfds)){
			*clientIndex = i;
			// Check if is closing message
			if((valread = read(sd, buffer, MESSAGE_SIZE)) == 0){
				getpeername(sd , &addr, (socklen_t *)&addrlen);
				std::cout << std::endl << "Host disconnected at IP: " << inet_ntoa(address.sin_addr) << std::endl;
				std::cout << "At port " << ntohs(address.sin_port) << " and socket " << sd << std::endl << std::endl;

				// Closing disconnected port
				close(sd);
				client_socket[i] = 0;
				return DC_MESSAGE;
			}
			else {
				aux[valread] = '\0';
				return TXT_MESSAGE;
			}
		}
	}
	return 0;
}