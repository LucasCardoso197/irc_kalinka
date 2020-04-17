#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string>
#include <iostream> 

// Settings
#define PORT 8080 
#define MESSAGE_SIZE 4096

class Server
{
private:
	int server_fd, listen_socket_fd;
	struct sockaddr_in server_address;
public:
    Server();
    ~Server();
    int setup();
    int listenForConnections();
	int sendResponse(std::string message);
	int readMessage(void *buffer);
    void closeConnection();
};

int main(int argc, char const *argv[]){
    Server s;
    char buffer[MESSAGE_SIZE] = {0}; 
    std::string response = "Hello from server"; 

    if(s.setup() == -1) return -1;
    while(true){
        s.listenForConnections();
        s.readMessage(buffer);
        std::cout << "Client says: " << buffer << std::endl;
        s.sendResponse(response);
        s.closeConnection();
    }

    return 0; 
}

// Methods definitions
Server::Server(){
    server_fd = 0;
    listen_socket_fd = 0;
}

Server::~Server(){
	if(server_fd != 0)
		close(server_fd);
	if(listen_socket_fd != 0)
		close(listen_socket_fd);
}

int Server::setup(){
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		std::cerr << "Socket creation error" << std::endl; 
		return -1; 
    }

    // Setting some socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        std::cerr << "Error configuring socket." << std::endl;
        close(server_fd);
        server_fd = 0; 
        return -1;
    } 
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = INADDR_ANY; 
    server_address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address))<0){ 
        std::cerr << "Error binding to port." << std::endl;
        close(server_fd);
        server_fd = 0;
        return -1;
    }
    return 0;
}

int Server::listenForConnections(){
    if(server_fd == 0){
        std::cerr << "Please setup the server before trying to listen for connections." << std::endl;
        return -1;
    }

    if(listen(server_fd, 3) < 0){
        std::cerr << "Error during listen." << std::endl;
        return -1;
    }

    int addrlen = sizeof(server_address); 
    if ((listen_socket_fd = accept(server_fd, (struct sockaddr *)&server_address, (socklen_t*)&addrlen))<0){
        std::cerr << "Error accepting connection." << std::endl; 
        return -1;
    }
    std::cout << "Conection established, ready to send and receive messages." << std::endl;
}

int Server::sendResponse(std::string message){
	if(listen_socket_fd == 0){
		std::cerr << "Please listen for connections before sending messages." << std::endl;
		return -1;
	}
	return send(listen_socket_fd, message.c_str(), message.length(), 0);
}

int Server::readMessage(void *buffer){
	return read(listen_socket_fd, buffer, MESSAGE_SIZE);
}

void Server::closeConnection(){
    if(listen_socket_fd != 0)
        close(listen_socket_fd);
    listen_socket_fd = 0;
}