#include <iostream>
#include "SyncManager.h"

#include <Server.h>
#include "LuaConsole.h"

int main(){
    Server server;
    server.init();
    //SyncManager::init();

    LuaConsole::init();
    LuaConsole::execute("resources/crystal/server.lua");

    while ( true ){
        //if ( SyncManager::selector.wait(sf::seconds(0.1f)) ){
            //SyncManager::acceptConnections();
            //SyncManager::receivePackets();
        //}
        //SyncManager::sceneProcess();
        server.process();
    }
    return 0;
}
