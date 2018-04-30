#include <iostream>
#include "SyncManager.h"

#include <Server.h>
#include "LuaConsole.h"
#include "FileManager.h"

int main(){
    Server::init();
    //SyncManager::init();

    LuaConsole::init();
    LuaConsole::execute("resources/crystal/__resource.lua");
    FileManager::readFilesFromLuaState();
    FileManager::loadFiles();

    while ( true ){
        //if ( SyncManager::selector.wait(sf::seconds(0.1f)) ){
            //SyncManager::acceptConnections();
            //SyncManager::receivePackets();
        //}
        //SyncManager::sceneProcess();
        Server::process();
    }
    return 0;
}
