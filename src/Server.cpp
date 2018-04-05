#include "Server.h"

Server::Server()
{
    //ctor
}

Server::~Server()
{
    //dtor
}

void Server::init(){
    playerList.init();
}

void Server::process(){
    playerList.process();
}
