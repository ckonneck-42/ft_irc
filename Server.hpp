#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <map>
#include <ctime>
#include <algorithm>
#include "Chatroom.hpp"
#include "User.hpp"
class Chatroom;



extern std::map<std::string, Chatroom*> g_chatrooms;
extern std::vector<User*> g_mappa;
bool serverexit();
void cleanup(std::vector<pollfd> &fds);
void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd);
void welcomemessage();
void messagehandling(std::vector<pollfd> &fds, size_t i);
void validatePort(char *argv);
bool isDigit(char *strnum);
void commandParsing(char *messagebuffer, std::vector<pollfd> &fds, size_t i);
std::vector<std::string> split(const std::string &input, char delimiter);
User* findUserByFD(int fd);
User* findUserByNickname(const std::string& nick);
std::string intToString(int value);
void removeUser(User* target);
void send_to_client(int client_fd, const std::string& message);
std::string sanitize(const std::string& str);
std::string parseNick(const std::string &msg);
std::string parseUser(const std::string &msg);
std::string parseHost(const std::string &msg);
void join_channel(int client_fd, const std::string& nickname, const std::string& channel);