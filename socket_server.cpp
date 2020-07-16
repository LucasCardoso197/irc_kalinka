#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <poll.h>
#include <string.h>

// Settings
#define PORT 8080
#define MESSAGE_SIZE 4096
#define LOOP_INTERVAL 100
#define MAX_CLIENTS 30

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
	int connectToNewClient();
	int sendMessage(std::string message, int socket_fd);
	void broadcastMessage(std::string message);
	int sendMessageUser(std::string message, int user);
	int readMessage(void *buffer, int *clientIndex);
};

class User
{
public:
	std::string nickname;
	int channel;
};

class Channel
{
public:
	std::string name;
	int admin;
	int muted[MAX_CLIENTS];
};

int main(int argc, char const *argv[])
{

	// create work variables
	int activityType, messageType, i = 0;
	char msg_buffer[MESSAGE_SIZE];

	// users array
	User users[MAX_CLIENTS];

	// channels vector
	std::vector<Channel> channels;

	// create and setup the server
	Server s;
	if (s.setup() == -1)
		return -1;

	// server loop
	while (true)
	{

		// select a socket and get the activity type
		activityType = s.selectSocket();

		// check for new client connection
		if (activityType == S_CONNECTION)
		{
			// Receive connection
			s.connectToNewClient();
		}
		// check for message
		else if (activityType == S_MESSAGE)
		{

			// Receive message
			messageType = s.readMessage(msg_buffer, &i);
			if (messageType != DC_MESSAGE)
			{

				// convert to string and log
				std::string line(msg_buffer);
				if (!users[i].nickname.empty())
					std::cout << users[i].nickname << ": " << line << std::endl;

				// check for commands
				if (line.front() == '/')
				{

					// check for ping command
					if (line.compare("/ping") == 0)
					{
						s.sendMessageUser("Server: pong", i);
						std::cout << "Pong sent to user '" << users[i].nickname << "'" << std::endl
								  << std::endl;
					}
					// check for join command
					else if (line.compare(0, 6, "/join ") == 0)
					{

						// reset this user channel
						users[i].channel = -1;

						// get the channel name
						std::string channelName = line.substr(6, line.length());

						// check if channel already exists
						for (int j = 0; j < channels.size(); j++)
						{
							if (channels[j].name.compare(channelName) == 0)
							{
								users[i].channel = j;
								std::cout << "User '" << users[i].nickname << "' joined the channel '" << channelName << "'" << std::endl
										  << std::endl;
								s.sendMessageUser("You joined the channel '" + channelName + "'", i);
							}
						}

						// if could not find the channel, create a new one
						if (users[i].channel < 0)
						{

							// create and add the new channel
							Channel newChannel;
							newChannel.name = channelName;
							newChannel.admin = i;
							for (int j=0; j<MAX_CLIENTS; j++) 
								newChannel.muted[j] = 0;
							channels.push_back(newChannel);

							// link the user to the channel
							users[i].channel = channels.size() - 1;
							std::cout << "User '" << users[i].nickname << "' created and joined the channel '" << channelName << "'" << std::endl
									  << std::endl;
							s.sendMessageUser("You created and joined the channel '" + channelName + "'", i);
						}
					}
					// check nickname command
					else if (line.compare(0, 10, "/nickname ") == 0)
					{
						// new connection nickname
						if (users[i].nickname.empty())
						{
							users[i].nickname = line.substr(10, -1);
							users[i].channel = -1;
							std::cout << "New connection nickname: " << users[i].nickname << std::endl
									  << std::endl;
						}
						// nickname changing
						else
						{
							std::cout << "User '" << users[i].nickname << "' changed his nickname to '" << line.substr(10, -1) << "'" << std::endl
									  << std::endl;
							users[i].nickname = line.substr(10, -1);
							s.sendMessageUser("Nickname changed to '" + users[i].nickname + "'", i);
						}
					}
					// ********************************************************* ADMIN COMMANDS ********************************************************* //
					// check kick command
					else if (line.compare(0, 6, "/kick ") == 0)
					{
						// check if user has joined a channel
						if (users[i].channel >= 0)
						{
							//check if user is admin on said channel
							if (channels[users[i].channel].admin == i)
							{
								// get the user name
								std::string userName = line.substr(6, line.length());
								// search for user in the channel
								int found = 0;
								for (int j=0; j<MAX_CLIENTS; j++) {
									if (users[j].nickname.compare(userName) == 0 && users[j].channel == users[i].channel) {
										found = 1;
										users[j].channel = -1;
										s.sendMessageUser("You have been kicked from the channel", j);
										s.sendMessageUser("User " + userName + " has been kicked from your channel", i);
										break;
									}
								}
								if (!found) s.sendMessageUser("User not found in your channel", i);
							}
							else {
								s.sendMessageUser("You are not the admin of this channel", i);
							}
						}
						else {
							s.sendMessageUser("You must create your own channel before using admin commands ", i);
						}
					}
					// check mute command
					else if (line.compare(0, 6, "/mute ") == 0)
					{
						// check if user has joined a channel
						if (users[i].channel >= 0)
						{
							//check if user is admin on said channel
							if (channels[users[i].channel].admin == i) {
								// get the user name
								std::string userName = line.substr(6, line.length());
								// search for user in the channel
								int found = 0;
								for (int j=0; j<MAX_CLIENTS; j++) {
									if (users[j].nickname.compare(userName) == 0 && users[j].channel == users[i].channel) {
										found = 1;
										channels[users[i].channel].muted[j] = 1;
										s.sendMessageUser("You have been muted on this channel", j);
										s.sendMessageUser("User " + userName + " has been muted in your channel", i);
										break;
									}
								}
								if (!found) s.sendMessageUser("User not found in your channel", i);
							}
							else {
								s.sendMessageUser("You are not the admin of this channel", i);
							}
						}
						else {
							s.sendMessageUser("You must create your own channel before using admin commands ", i);
						}
					}
					// check unmute command
					else if (line.compare(0, 8, "/unmute ") == 0)
					{
						// check if user has joined a channel
						if (users[i].channel >= 0)
						{
							//check if user is admin on said channel
							if (channels[users[i].channel].admin == i) {
								// get the user name
								std::string userName = line.substr(8, line.length());
								// search for user in the channel
								int found = 0;
								for (int j=0; j<MAX_CLIENTS; j++) {
									if (users[j].nickname.compare(userName) == 0 && users[j].channel == users[i].channel) {
										found = 1;
										channels[users[i].channel].muted[j] = 0;
										s.sendMessageUser("You have been unmuted on this channel", j);
										s.sendMessageUser("User " + userName + " has been unmuted in your channel", i);
										break;
									}
								}
								if (!found) s.sendMessageUser("User not found in your channel", i);
							}
							else {
								s.sendMessageUser("You are not the admin of this channel", i);
							}
						}
					}
					// check whois command
					else if (line.compare(0, 7, "/whois ") == 0)
					{
						// check if user has joined a channel
						if (users[i].channel >= 0)
						{
							//check if user is admin on said channel
							if (channels[users[i].channel].admin == i)
							{
								// whois structure (TODO)
							}
							else {
								s.sendMessageUser("You are not the admin of this channel", i);
							}
						}
						else {
							s.sendMessageUser("You must create your own channel before using admin commands ", i);
						}
					}
				}
				// if it is not a command, it is a common text message
				else
				{
					// check if user has joined a channel and is not muted
					if (users[i].channel >= 0 && !channels[users[i].channel].muted[i])
					{
						// send message to all users in the same channel
						int userChannel = users[i].channel;
						for (int j = 0; j < MAX_CLIENTS; j++)
						{
							if (!users[j].nickname.empty() && users[j].channel == userChannel)
							{
								s.sendMessageUser(users[i].nickname + ": " + line, j);
							}
						}
					}
					// if the user is not in a channel, just explain it
					else
					{
						std::cout << "User '" << users[i].nickname << "' has not joined a channel and can't send messages" << std::endl
								  << std::endl;
						s.sendMessageUser("Please join a channel before sending messages", i);
					}
				}
			}
			// a user is disconnecting
			else
			{
				// forget his nickname
				users[i].nickname.clear();

				// revove his admin rights from any channels
				for (int j = 0; j < channels.size(); j++)
				{
					if (channels[j].admin = i)
					{
						channels[j].admin = -1;
					}
				}
			}
		}
	}

	return 0;
}

// Methods definitions
Server::Server()
{
	connection_fd = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
		client_socket[i] = 0;
}

Server::~Server()
{
	if (connection_fd != 0)
		close(connection_fd);
	for (int i = 0; i < MAX_CLIENTS; i++)
		if (client_socket[i] != 0)
			close(client_socket[i]);
}

int Server::setup()
{
	// Creating socket file descriptor
	if ((connection_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		std::cerr << "Socket creation error" << std::endl;
		return -1;
	}

	// Setting some socket options
	int opt = true;
	if (setsockopt(connection_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *)&opt, sizeof(opt)) < 0)
	{
		std::cerr << "Error configuring socket." << std::endl;
		close(connection_fd);
		connection_fd = 0;
		return -1;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(connection_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "Error binding to port." << std::endl;
		close(connection_fd);
		connection_fd = 0;
		return -1;
	}
	std::cout << "Listening on PORT " << PORT << std::endl;

	// Setting connection_fd as a socket that listen for connections
	if (listen(connection_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	std::cout << "Waiting for connections..." << std::endl
			  << std::endl;

	return 0;
}

int Server::selectSocket()
{
	int sd, max_sd, activity;

	if (connection_fd == 0)
	{
		std::cerr << "Please first run the setup method." << std::endl;
		return -1;
	}

	// Preparing fdset
	FD_ZERO(&readfds);
	FD_SET(connection_fd, &readfds);
	max_sd = connection_fd;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		sd = client_socket[i];
		if (sd > 0)
			FD_SET(sd, &readfds);

		if (sd > max_sd)
			max_sd = sd;
	}

	// Running select
	activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);
	if ((activity < 0) && (errno != EINTR))
	{
		std::cerr << "Error during select." << std::endl;
		return -2;
	}

	if (FD_ISSET(connection_fd, &readfds))
		return S_CONNECTION;
	else
		return S_MESSAGE;
}

int Server::connectToNewClient()
{
	int new_socket;
	int addr_len = sizeof(address);
	if ((new_socket = accept(connection_fd, (struct sockaddr *)&address, (socklen_t *)&addr_len)) < 0)
	{
		std::cerr << "Error accepting connection." << std::endl;
		return -1;
	}

	std::cout << "New connection at IP: " << inet_ntoa(address.sin_addr) << std::endl;
	std::cout << "At port " << ntohs(address.sin_port) << " and socket " << new_socket << std::endl;

	// Sending welcome message
	if (this->sendMessage("Welcome to IRC Kalinka!!", new_socket) <= 0)
		std::cerr << "Error sending welcome message." << std::endl;

	// Adding new connected socket to the list
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_socket[i] == 0)
		{
			client_socket[i] = new_socket;
			std::cout << "Added new connection at " << i << std::endl;
			break;
		}
	}

	return 0;
}

int Server::sendMessage(std::string message, int socket_fd)
{
	return send(socket_fd, message.c_str(), message.length(), 0);
}

void Server::broadcastMessage(std::string message)
{
	int error_flag;

	// loop that goes through every client
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_socket[i] != 0)
		{
			error_flag = -1;

			// loop that attempts to send message to client i
			for (int j = 0; j < 5 && error_flag == -1; j++)
			{
				error_flag = this->sendMessage(message, client_socket[i]);

				if (error_flag != -1)
					std::cout << "Message sent successfully to socket " << client_socket[i] << " after " << j + 1 << " try(ies)\n";
				else
				{
					std::cout << "Message failed to reach socket " << client_socket[i] << " after " << j + 1 << "try(ies)\n";
				}
			}
			// closes socket after 5 failed attempts
			if (error_flag == -1)
			{
				std::cout << "Closing socket " << client_socket[i] << "\n";
				close(client_socket[i]);
				client_socket[i] = 0;
			}
		}
	}
}

int Server::sendMessageUser(std::string message, int user)
{
	return send(client_socket[user], message.c_str(), message.length(), 0);
}

int Server::readMessage(void *buffer, int *clientIndex)
{
	int sd, valread;
	struct sockaddr addr;
	int addrlen = sizeof(addr);
	char *aux = (char *)buffer;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		sd = client_socket[i];
		if (FD_ISSET(sd, &readfds))
		{
			*clientIndex = i;
			// Check if is closing message
			if ((valread = read(sd, buffer, MESSAGE_SIZE)) == 0)
			{
				getpeername(sd, &addr, (socklen_t *)&addrlen);
				std::cout << std::endl
						  << "Host disconnected at IP: " << inet_ntoa(address.sin_addr) << std::endl;
				std::cout << "At port " << ntohs(address.sin_port) << " and socket " << sd << std::endl
						  << std::endl;

				// Closing disconnected port
				close(sd);
				client_socket[i] = 0;
				return DC_MESSAGE;
			}
			else
			{
				aux[valread] = '\0';
				return TXT_MESSAGE;
			}
		}
	}
	return 0;
}