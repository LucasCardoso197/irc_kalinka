#ifndef _SERVER_HPP_
#define _SERVER_HPP_

// Codes
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
	int connectToNewClient(int *j, std::string& ip);
	int sendMessage(std::string message, int socket_fd);
	void sendMessageUser(std::string message, int user);
	int readMessage(void *buffer, int *clientIndex);
};

#endif