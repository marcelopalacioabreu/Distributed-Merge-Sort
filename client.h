#ifndef CLIENT_H
#define CLIENT_H

#include <utility>
#include <string>

std::pair<int,std::string> receive_message(int sockfd);

void user_input(int sockfd);

#endif
