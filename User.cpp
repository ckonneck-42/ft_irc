#include "Server.hpp"
#include <cerrno>

std::vector<User*> g_mappa;
std::map<std::string, Chatroom*> g_chatrooms;

User::User(const std::string &nickname,const std::string &password) : _username(""), _nickname(nickname), _password(password), _hostname("")
{

    this->_isOP = false;
    std::cout << "User "<< this->_nickname <<" has been created" <<std::endl;
}

User::~User()
{
    std::cout << "User "<< this->_nickname <<" fucked off to somewhere else" <<std::endl;
}
void User::newclient(int server_fd, std::vector<pollfd> &fds)
{
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cout << "strangerdanger" << std::endl;
        }
    }
    else
    {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        pollfd client_pollfd = { client_fd, POLLIN, 0 };
        fds.push_back(client_pollfd);
        std::cout << "New client connected: FD nr " << client_fd << "\n";

        // Create a temporary user with default nickname
        // We'll update the nickname when we receive a NICK command
        User* newUser = new User("","");
        newUser->_FD = client_fd;
        newUser->setRegis(false);
        g_mappa.push_back(newUser);
        std::cout << "we done" <<std::endl;

        // newUser->HSwelcome(client_fd);
    }
    

}

#include <unistd.h>   // for write()
void User::sendMsg(const std::string& msg)
{
    int fd = _FD;
    write(fd, msg.c_str(), msg.size());
}

std::string parseNick(const std::string &msg)
{
    size_t pos = msg.find("NICK");
    if (pos == std::string::npos)
        return ""; // NICK not found

    // Move past "NICK" and any space
    pos += 4;

    // Skip any whitespace after "NICK"
    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Extract until end of line or space
    size_t end = msg.find_first_of("\r\n ", pos);
    std::string nickname = msg.substr(pos, end - pos);

    return nickname;
}

std::string parseUser(const std::string &msg)
{
    size_t pos = msg.find("USER");
    if (pos == std::string::npos)
        return ""; // USER not found

    pos += 4;
    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Extract the first word after USER (the username)
    size_t end = msg.find_first_of("\r\n ", pos);
    return msg.substr(pos, end - pos);
}

std::string parseHost(const std::string &msg)
{
    size_t pos = msg.find("USER");
    if (pos == std::string::npos)
        return ""; // USER not found

    pos += 4;
    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Skip username (first word)
    pos = msg.find_first_of(" \r\n", pos);
    if (pos == std::string::npos)
        return "";

    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Skip mode/unused (second word)
    pos = msg.find_first_of(" \r\n", pos);
    if (pos == std::string::npos)
        return "";

    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Now we're at the host (third word)
    size_t end = msg.find_first_of("\r\n ", pos);
    return msg.substr(pos, end - pos);
}


void User::HSwelcome(int &client_fd)
{
    std::string nick = this->_nickname; // from client
    std::string host = this->_hostname;
    std::string username = this->_username;
    std::string msg1 = ":localhost 001 " + nick + " :Welcome to the UWURC server " + nick + "!" + username + "@" + host + "\r\n";
    send(client_fd, msg1.c_str(), msg1.length(), 0);

    std::string msg2 = ":localhost 002 " + nick + " :Your host is UWUCHAN running version 1.0\r\n";
    send(client_fd, msg2.c_str(), msg2.length(), 0);

    std::string msg3 = ":localhost 003 " + nick + " :This server was created IMA DA NYA\r\n";
    send(client_fd, msg3.c_str(), msg3.length(), 0);

    std::string msg4 = ":localhost 004 " + nick + " owo please don't be mean\r\n";
    send(client_fd, msg4.c_str(), msg4.length(), 0);
}

void User::HSNick(const std::string &oldname, const std::string &newname)
{
    std::string oldnick = oldname;
    std::string newNick = newname;
    std::string nickMsg = ":" + oldnick + "!user@localhost NICK :" + newNick + "\r\n";
    for (std::map<std::string, Chatroom*>::iterator it = g_chatrooms.begin();
        it != g_chatrooms.end(); ++it)
    {
        it->second->broadcast(nickMsg, this);
    }
    send(this->_FD, nickMsg.c_str(), nickMsg.length(), 0);
}

User* findUserByFD(int fd)
{
    std::cout << "Starting search for user with FD: " << fd << std::endl;
    for (size_t i = 0; i < g_mappa.size(); ++i)
    {
        User* user = g_mappa[i];

        // Debug print of nickname and FD being checked
        std::cout << "Checking user: " << user->getNickname()
                  << " (FD: " << user->getFD() << ")" << std::endl;

        if (user->getFD() == fd) {
            std::cout << "Match found: " << user->getNickname() << std::endl;
            return user;
        }
    }
    std::cout << "No user found for FD: " << fd << std::endl;
    return NULL;
}


User* findUserByNickname(const std::string& nick)
{
    for (size_t i = 0; i < g_mappa.size(); ++i)
    {
        User* user = g_mappa[i];
        if (user->getNickname() == nick)
            return user;
    }
    return NULL;
}

void User::HSKick(const std::string &target)//templates for later parsing
{   
    if (this->_isOP != true)
        return;
    std::string reason = "guy is racist";
    std::string channel = "yeanoidea";
    std::string msg = ":" + this->_nickname + "!user@localhost KICK " 
        + channel + " " + target + " :" + reason + "\r\n";
    send(this->_FD, msg.c_str(), msg.length(), 0);
}

void User::HSInvite(const std::string &whotoinv)
{
    //check if user has invite rights i guess
    //server sends response back to the client 
    std::string channel = "yeanoidea";//parsing plss
    std::string msg = ":localhost 341 " + this->_nickname + " " + whotoinv + " " + channel + "\r\n";
    send(this->_FD, msg.c_str(), msg.length(), 0);

    //server sends message to the person being invited
    std::string msg2 = ":" + this->_nickname + "!user@localhost INVITE " + whotoinv + " :" + channel + "\r\n";
    User *targetuser = findUserByNickname(whotoinv);
    send(targetuser->_FD, msg2.c_str(), msg2.length(), 0);
}

void User::HSTopicQuery(Chatroom &chatroom)//this is for when client
//sends: /Topic #channelname
{
    if (chatroom.hasTopic() == true) {
        std::string topic = chatroom.getTopic();
        std::string setter = chatroom.getLastTopicSetter();
        std::ostringstream oss;
        oss << chatroom.getTopicTime();
        std::string timestamp = oss.str();

        std::string msg332 = ":localhost 332 " + this->_nickname + " " + chatroom.getName() + " :" + topic + "\r\n";
        send(this->_FD, msg332.c_str(), msg332.length(), 0);

        std::string msg333 = ":localhost 333 " + this->_nickname + " " + chatroom.getName() + " " + setter + " " + timestamp + "\r\n";
        send(this->_FD, msg333.c_str(), msg333.length(), 0);
    } else {
        std::string msg = ":localhost 331 " + this->_nickname + " " + chatroom.getName() + " :No topic is set\r\n";
        send(this->_FD, msg.c_str(), msg.length(), 0);
    }
}

void User::HSSetTopic(const std::string &topicstring, Chatroom &chatroom)
{
    //if user has rights
    chatroom.setTopic(topicstring, chatroom.getLastTopicSetter());
    std::string topicChangeMsg = ":" + this->_nickname + "!user@localhost TOPIC " + chatroom.getName() + " :" + topicstring + "\r\n";
    chatroom.broadcast(topicChangeMsg, this);
}

void User::setNickname(const std::string &nick)
{
    this->_nickname = nick;
}
 void User::setHostname(const std::string &host)
 {
    this->_hostname = host;
 }
void User::setUser(const std::string &user_str)
{
    this->_username = user_str;
}

std::string User::getHostname()
{
    return this->_hostname;
}

std::string User::getUsername()
{
    return this->_username;
}