#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string>
#include <iostream>
#include <chrono>
#include <future>
#include <poll.h>
#include <csignal>

// Settings
#define PORT 8080 
#define MESSAGE_SIZE 4096
#define LOOP_INTERVAL 100

class Client
{
private:
	int socket_fd;
	struct sockaddr_in server_address;
public:
	Client();
	~Client();
	int connectToServer(const char *domain, std::string nickname);
	int sendMessage(std::string message);
	int readMessage(void *buffer);
	int getFd();
};

// function to get user input asynchronously
std::string WaitInput() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void signalHandler( int signum ) {
	std::cout << "Interrupt signal (" << signum << ") received.\n";
	std::cout << "To exit the program, enter \"/quit\" or press CTRL+D\n";
}

int main(int argc, char const *argv[]) 
{ 
	// associate SIGINT with signalHandler function
	signal(SIGINT, signalHandler);

	// client welcome message
	std::cout << "*************************" << std::endl;
	std::cout << "* Welcome to IRC Client *" << std::endl;
	std::cout << "*************************" << std::endl;

	// create the client object
	Client c;

	// create the nickname string
	std::string nickname;

	// nickname loop
	while (true) {
		std::cout << std::endl << "Please use '/nickname [name]' to set your nickname" << std::endl << std::endl;

		// read a line		
		std::string line;
    	std::getline(std::cin, line);

    	// analyze the input
    	if (line.front() == '/') {
    		// check quit command
    		if (line.compare("/quit") == 0) {
    			std::cout << std::endl << "Closing client" << std::endl;
    			return 0;
    		}
    		// check nickname command
    		else if (line.compare(0, 10, "/nickname ") == 0 && line.length() > 10) {
    			// check if nickname lenght is valid
    			if (line.length() > 60) {
    				std::cout << std::endl << "Please use a nickname with 50 or less characters" << std::endl;
    			}
    			else {
	    			nickname = line;
	    			std::cout << std::endl << "Nickname defined: " << line.substr(10, -1) << std::endl;
	    			break;
    			}
    		}
    		else {
    			std::cout << std::endl << "Invalid command" << std::endl;
    		}
    	}
	}

	// connection loop
	while (true) {
		std::cout << std::endl << "Please use '/connect' or '/connect [server-address]' to start" << std::endl << std::endl;

		// read a line		
		std::string line;
    	std::getline(std::cin, line);

    	// analyze the input
    	if (line.front() == '/') {
    		// check quit command
    		if (line.compare("/quit") == 0) {
    			std::cout << std::endl << "Closing client" << std::endl;
    			return 0;
    		}
    		// check connect command
    		else if (line.compare("/connect") == 0) {
				if(c.connectToServer("127.0.0.1", nickname) == 0) 
					break;
    		}
    		// check connect [server-address] command
    		else if (line.compare(0, 9, "/connect ") == 0) {
    			if(c.connectToServer(line.substr(9, 16).c_str(), nickname) == 0) 
					break;
    		}
    		else {
    			std::cout << std::endl << "Invalid command" << std::endl;
    		}
    	}
	}

	// initialize socket poll struct
    struct pollfd fds[1];
    fds[0].fd = c.getFd();
    fds[0].events = 0;
    fds[0].events |= POLLIN;

    // run asynchronous function to get user input
    auto waitInput = std::async(std::launch::async, WaitInput);

    // begin socket loop
    while (!std::cin.eof()) {

        // poll during the LOOP_INTERVAL
        int pollResult = poll(fds, 1, LOOP_INTERVAL);

        // if the poll returns >0, there is a message to read
        if (pollResult > 0) {
            char buffer[MESSAGE_SIZE] = {0};
            if (c.readMessage(buffer) == 0){
				std::cout << "Server closed, closing application." << std::endl;
				exit(1);
			}
            std::cout << buffer << std::endl << std::endl;
        }

        // check if the asynchronous task has a result
        if (waitInput.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {

            // get the result and send it
            auto line = waitInput.get();

			// check for eof (ctrl+d)
			if (line.front() == EOF) break;

			// check for commands
			if (line.front() == '/') {
				// check for quit command
				if (line.compare("/quit") == 0) {
					c.sendMessage(line);
					std::cout << "Disconnecting..." << std::endl;
					break;
				}
				// check for ping command
				else if (line.compare("/ping") == 0) {
					c.sendMessage(line);
				}
				// check for join command
        		else if (line.compare(0, 6, "/join ") == 0 && line.length() > 6) {
        			c.sendMessage(line);
        		}
        		// check nickname command
	    		else if (line.compare(0, 10, "/nickname ") == 0 && line.length() > 10) {
	    			// check if nickname lenght is valid
	    			if (line.length() > 60) {
	    				std::cout << std::endl << "Please use a nickname with 50 or less characters" << std::endl;
	    			}
	    			else {
		    			nickname = line;
		    			c.sendMessage(line);
	    			}
	    		}
				else {
    				std::cout << "Invalid command" << std::endl << std::endl;
    			}
			} else {

	            // split the message in n parts using the MESSAGE_SIZE
	           	int offset = 0;
	           	while ( (int)line.length() - offset > 0) {
	            	c.sendMessage(line.substr(offset, MESSAGE_SIZE));
	            	offset += MESSAGE_SIZE;
	           	}

				//std::cout << std::endl << "Message sent:" << std::endl << line << std::endl << std::endl;
			}

            // run the asynchronous function again
            waitInput = std::async(std::launch::async, WaitInput);
        }
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

int Client::connectToServer(const char *domain, std::string nickname){
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

	// Sending nickname
	sendMessage(nickname);
	
	std::cout << "Connected to " << domain << std::endl << std::endl;
	return 0;
}

int Client::sendMessage(std::string message){
	if(socket_fd == 0){
		std::cerr << "Please connect to a server before sending messages." << std::endl;
		return -1;
	}
	return send(socket_fd, message.c_str(), message.length(), 0);
}

int Client::readMessage(void *buffer){
	return read(socket_fd, buffer, MESSAGE_SIZE);
}

int Client::getFd(){
	return socket_fd;
}
