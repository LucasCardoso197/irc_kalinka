#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string>
#include <iostream>
#include <chrono>
#include <future>

// Settings
#define PORT 8080 
#define MESSAGE_SIZE 4096

class Client
{
private:
	int socket_fd;
	struct sockaddr_in server_address;
public:
	Client();
	~Client();
	int connectToServer(const char *domain);
	int sendMessage(std::string message);
	int readResponse(void *buffer);
};

std::string WaitInput() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::string ReadResponse(Client c){
	char buffer[MESSAGE_SIZE] = {0};
	c.readResponse(buffer);
	std::string line(buffer);
	//fprintf(stderr, "socket() failed: %m\n");
	return line;
}

int main(int argc, char const *argv[]) 
{ 
	Client c;

	if(c.connectToServer("127.0.0.1") == -1) return -1;

	auto inputFuture = std::async(std::launch::async, WaitInput);
	//auto readFuture = std::async(std::launch::async, ReadResponse, c);

    while (true) {
    	
        if (inputFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto line = inputFuture.get();
            std::cout << "you wrote " << line << std::endl;
            c.sendMessage(line);
            inputFuture = std::async(std::launch::async, WaitInput);
        }

        /*if (readFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto line = readFuture.get();
            std::cout << "recebeu " << line << std::endl;
            readFuture = std::async(std::launch::async, ReadResponse, c);
        }*/

        std::cout << "waiting..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
	
	return 0; 
}



// Methods definitions
Client::Client() {
	socket_fd = 0;
}

Client::~Client() {
	if(socket_fd != 0)
		close(socket_fd);
}

int Client::connectToServer(const char *domain){
	// Socket creation
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "Socket creation error" << std::endl; 
		return -1; 
	}

	// Creating address info
	server_address.sin_family = AF_INET; 
	server_address.sin_port = htons(PORT); 
	if(inet_pton(AF_INET, domain, &server_address.sin_addr)<=0) {
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		return -1; 
	}

	// Connecting to server
	if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { 
		std::cerr << "Connection Failed" << std::endl;
		return -1; 
	}
	
	std::cout << "Connected to " << domain << std::endl;
	return 0;
}

int Client::sendMessage(std::string message){
	if(socket_fd == 0){
		std::cerr << "Please connect to a server before sending messages." << std::endl;
		return -1;
	}
	return send(socket_fd, message.c_str(), message.length(), 0);
}

int Client::readResponse(void *buffer){
	return recv(socket_fd, buffer, MESSAGE_SIZE, MSG_DONTWAIT|MSG_PEEK);
}