#include "client.h"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include <algorithm>

#define PORT "2018"

#define MAXDATASIZE 100

int main(int argc, char *argv[]) {
	struct addrinfo hints, *servinfo, *p;
	int sockfd;
	int getaddrinfo_result;
	
	if (argc != 2) {
		std::cerr << "pass in only server ip" << std::endl;
		return 1;
	}
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((getaddrinfo_result = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		std::cerr << "getaddrinfo error: " << gai_strerror(getaddrinfo_result) << std::endl;
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			std::cerr << "socket create: " << strerror(errno) << std::endl;
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			std::cerr << "socket connect: " << strerror(errno) << std::endl;
			continue;
		}
		break;
	}
	
	if (p == NULL) {
		std::cerr << "client connect failed" << std::endl;
		return 2;
	}
	
	freeaddrinfo(servinfo);
	
	std::vector<int> data;
	
	int recv_result;
	int chunk_size;

	if ((recv_result = recv(sockfd, &chunk_size, sizeof(int), 0)) == 0) {
		std::cout << "connection closed" << std::endl;
		return 0;
	}  
	chunk_size = ntohl(chunk_size);
	std::cout << "Chunk size: " << chunk_size << std::endl;

	for (int i = 0; i < chunk_size; i++) {
		int tmp;
		if ((recv_result = recv(sockfd, &tmp, sizeof(int), 0)) == 0) {
			std::cout << "connection close" << std::endl;
			return 0;
		}
		tmp = ntohl(tmp);
		data.push_back(tmp);
	}

	data = mergesort(data);
	std::cout << "SORTED: " << std::is_sorted(data.begin(), data.end()) << std::endl;
	for (int i = 0; i < data.size(); i++) {
		int tmp = htonl(data[i]);
		send(sockfd, &tmp, sizeof(int), 0);
		std::cout << data[i] << std::endl;
	}

	close(sockfd);
	return 0;
}

std::vector<int> mergesort(std::vector<int> arr) {
	if(arr.size() == 1) {
		return arr;
	}
	std::vector<int> left(arr.begin(), arr.begin() + arr.size()/2);
	std::vector<int> right(arr.begin() + arr.size()/2, arr.end());
	left = mergesort(left);
	right = mergesort(right);
	return merge(left, right);
}

std::vector<int> merge(std::vector<int> left, std::vector<int> right) {
	std::vector<int> merged;
	int j = 0;
	for (int i = 0; i < left.size(); i++) {
		while (j < right.size() && right[j] < left[i]) {
			merged.push_back(right[j]);
			j++;
		}
		merged.push_back(left[i]);
	}

	while (j < right.size()) {
		merged.push_back(right[j]);
		j++;
	}
	return merged;
}
