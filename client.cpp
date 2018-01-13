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
	chunk_size = ntohs(chunk_size);
	std::cout << "Chunk size: " << chunk_size << std::endl;

	for (int i = 0; i < chunk_size; i++) {
		int tmp;
		if ((recv_result = recv(sockfd, &tmp, sizeof(int), 0)) == 0) {
			std::cout << "connection close" << std::endl;
			return 0;
		}
		tmp = ntohs(tmp);
		std::cout << tmp << std::endl;
		data.push_back(tmp);
	}	

	close(sockfd);
	return 0;
}

void mergesort(vector<int> arr) {

}

void merge(vector<int> arr) {

}
