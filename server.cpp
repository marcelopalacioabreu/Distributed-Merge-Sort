#include "server.h"
#include <iostream>
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include <algorithm>

#define PORT "2018"
#define MAXDATASIZE 100
int main(int argc, char *argv[]) {
	std::vector<int> data;

	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	int sockfd;
	int getaddrinfo_result;
	int yes = 1;
	socklen_t sin_size;
	char buf[MAXDATASIZE];
	
	if (argc != 2) {
		std::cout << "Enter data filename" << std::endl;
		return 1;
	}

	std::cout << "Open " << argv[1] << std::endl;
	std::ifstream datafile (argv[1]);
	if (datafile.is_open()) {
		int tmp;
		while (datafile >> tmp) {
			data.push_back(tmp);
		}
		datafile.close();
	}
	std::cout << data.size() << " integers loaded." << std::endl;

	fd_set readfds;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((getaddrinfo_result = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		std::cout << "getaddrinfo error: " << gai_strerror(getaddrinfo_result) << std::endl;
		return 1;
	}
	
	for (p = servinfo; servinfo != NULL; servinfo = servinfo->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			std::cerr << "server: socket " << strerror(errno) << std::endl;
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			std::cerr << "setsockopt " << strerror(errno) << std::endl;
			return 1;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			std::cerr << "setsockopt " << strerror(errno) << std::endl;
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL) {
		std::cerr << "server failed to bind " << std::endl;
		return 1;
	}

	if (listen(sockfd, 10) == -1) {
		std::cerr << "listen failed " << strerror(errno) << std::endl;
		return 1;
	}
	
	int max_sd = sockfd;
	std::vector<int> clients;

	while (true) {
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		max_sd = sockfd;
		
		for (int i = 0; i < clients.size(); i++) {
			int sd = clients[i];
			FD_SET(sd, &readfds);
			
			if (sd > max_sd) {
				max_sd = sd;
			}
		}
		
		int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if (activity < 0) {
			std::cerr << strerror(errno) << std::endl;
		}

		if (FD_ISSET(sockfd, &readfds)) {
			socklen_t sin_size = sizeof their_addr;
			int new_socket;
			if ((new_socket = accept(sockfd, (struct sockaddr*) &their_addr, &sin_size)) < 0) {
				std::cerr << "Accept: " << strerror(errno) << std::endl;
				return 1;
			}
			clients.push_back(new_socket);

			std::string input;
			std::cout << "Run (y/n)" << std::endl;
			std::cin >> input;

			if (input == "y") {
				std::cout << "SEND" << std::endl;
				int num_per_client = data.size()/clients.size();
				
				std::cout << "Num clients: " << clients.size() << std::endl;		
				for (int i = 0; i < clients.size(); i++) {
					if (i == clients.size()-1) {
						num_per_client += data.size()%clients.size();
					}
					
					std::cout<<"SIZE: " << num_per_client << std::endl;
					int size = htons(num_per_client);
					send(clients[i], &size, sizeof(int), 0); 
					
					int begin = i * (data.size()/clients.size());
					for (int j = begin; j < begin + num_per_client; j++) {
						int value = htons(data[j]);
						send(clients[i], &value, sizeof(int), 0);
					}
				}
			}
		}	
	}		

	return 0;
}

