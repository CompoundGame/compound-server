#include "Server.h"

PlayerList Server::playerList;

void Server::init(){
    playerList.init();
}

void Server::process(){
    playerList.process();
}
