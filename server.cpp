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
#include <chrono>
#include <netdb.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>

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
	std::chrono::time_point<std::chrono::high_resolution_clock> start;

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
	
	std::vector<std::queue<int>> unmerged;
	
	int num_sorted = 0;
	while (num_sorted < data.size()) {
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		max_sd = sockfd;
		
		for (int i = 0; i < clients.size(); i++) {
			int sd = clients[i];
			FD_SET(sd, &readfds);
			
			if (sd > max_sd) {
				max_sd = sd;
			}
			if (unmerged.size() <= i) {
				std::queue<int> tmp;
				unmerged.push_back(tmp);
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
				start = std::chrono::high_resolution_clock::now();
				int num_per_client = data.size()/clients.size();
				
				std::cout << "Num clients: " << clients.size() << std::endl;		
				for (int i = 0; i < clients.size(); i++) {
					if (i == clients.size()-1) {
						num_per_client += data.size()%clients.size();
					}
					
					std::cout<<"SIZE: " << num_per_client << std::endl;
					int size = htonl(num_per_client);
					send(clients[i], &size, sizeof(int), 0); 
					
					int begin = i * (data.size()/clients.size());
					for (int j = begin; j < begin + num_per_client; j++) {
						int value = htonl(data[j]);
						send(clients[i], &value, sizeof(int), 0);
					}
				}
			}
		}	
		
		for (int i = 0; i < clients.size(); i++) {
			if (clients[i] >= 0 && FD_ISSET(clients[i], &readfds)) {
				int value;
				int numbytes = recv(clients[i], &value, sizeof(int), 0);
				if (numbytes == 0) {
					std::cout << "client disconnected" << std::endl;
					clients[i] = -1;
					continue;
				}
				value = ntohl(value);
				unmerged[i].push(value);
				num_sorted++;
			}
		}
	}	
	
	std::vector<int> merged = kMerge(unmerged);

	auto finish = std::chrono::high_resolution_clock::now();
	
  std::cout << std::endl << std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count() << std::endl;
	
	std::cout << std::endl << "Is sorted: " << isSorted(merged) << std::endl;

	return 0;
}

std::vector<int> kMerge(std::vector<std::queue<int>> &unmerged) {
	std::vector<int> merged;
	std::priority_queue<std::pair<int,int>, std::vector<std::pair<int,int>>, std::greater<std::pair<int,int>> > min_heap;

	for (int i = 0; i < unmerged.size(); i++) {
		std::pair<int,int> val_index (unmerged[i].front(), i); unmerged[i].pop();
		min_heap.push(val_index);
	}

	//k-way merge
	while (!min_heap.empty()) {
		std::pair<int,int> val_index = min_heap.top(); min_heap.pop();
		merged.push_back(val_index.first);
		if (!unmerged[val_index.second].empty()) {
			std::pair<int,int> new_val_index (unmerged[val_index.second].front(), val_index.second); 
			unmerged[val_index.second].pop();
			min_heap.push(new_val_index);
		}
	}
	return merged;
}

bool isSorted(const std::vector<int> &vec) {
	std::cout << "SIZE: " << vec.size() << std::endl;
	for (int i = 0; i < vec.size()-1; i++) {
		if (vec[i] > vec[i+1]) {
			std::cout << i << "," << vec[i] << "," << vec[i+1] << std::endl;
			return false;
		}
	}
	return true;
}
