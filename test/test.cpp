#include <string>
#include <iostream>

int isValidChannelName(std::string name);

int main(){
    std::string line;
    while(1){
        std::getline(std::cin, line);
        if (line.compare(0, 8, "/invite ") == 0 && line.length() > 8){
            std::string channelName, targetUserName;
            if(line.find(' ', 8) != std::string::npos){
                channelName = line.substr(8, line.find(' ', 8)-8);
                targetUserName = line.substr(line.find(' ', 8)+1, std::string::npos);
                if(isValidChannelName(channelName) < 0) {
                    std::cout << "Invalid channel name." << std::endl;
                    continue;
                }
                std::cout << "Sending command\n";
            }
            else
                std::cout << "Not enough arguments.\n";
        }
        else if (line.compare(0, 6, "/mode ") == 0 && line.length() > 6){
            std::string channelName, arguments;
            if(line.find(' ', 6) != std::string::npos){
                channelName = line.substr(6, line.find(' ', 6)-6);
                arguments = line.substr(line.find(' ', 6), std::string::npos);
                if(isValidChannelName(channelName) == 1){
					if(arguments.find("+i") == std::string::npos && line.find("-i") == std::string::npos)
						std::cout << "Please add +i or -i to configure channel to invite only." << std::endl;
                    else
                        std::cout << "sending message\n";
                }
            }
            else
                std::cout << "Not enough arguments, remember channel names\n";
        }
        else
            std::cout << "Invalid command\n";
    }

    return 0;
}

int isValidChannelName(std::string name){
    if (name.at(0) != '&' && name.at(0) != '#')
        return -1;
    else if (name.find(' ') != std::string::npos || name.find(',') != std::string::npos)
        return -2;
    else if (name.length() > 200)
        return -3;
    else
        return 1;
}