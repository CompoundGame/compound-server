#include "PlayerList.h"
#include <iostream>

#include <Mechanics.h>
#include <math.h>

/*
struct PlayerData{
    unsigned int id;
    //int crystalType;
    //int crystalHP;
    bool isOn;
    sf::TcpSocket tcpSocket;
    //sf::Vector2f position;
    sf::IpAddress ipAddress;
};
*/

PlayerList::PlayerList()
{
}

void PlayerList::init(){
    for ( unsigned int i = 0; i < MAX_PLAYERS; i++ )
        takenIDS[i] = false;

    playerCount = 0;

    takenIDS[0]=true;
    nextPlayerData = createPlayerData();

    if (udpSocket.bind(4474) != sf::Socket::Done)
        std::cout << "Problem binding UDP socket to port " << 4474 << std::endl;
    else
        std::cout << "Successfuly bound UDP socket to port" << 4474 << std::endl;
    selector.add(udpSocket);

    listener.listen(4474);
    selector.add(listener);
    std::cout << "Playerlist init!" << std::endl;
}

PlayerList::~PlayerList()
{
    //dtor
}

unsigned int PlayerList::getFreshID(){
    for ( unsigned int i = 0; i < MAX_PLAYERS; i++ ){
        if ( takenIDS[i] == false ){
            takenIDS[i] = true;
            std::cout << "FRESH ID : " << i << std::endl;
            return i;
        }
    }
    return -1;
}


PlayerData* PlayerList::createPlayerData(){
    PlayerData* newPlayerData = new PlayerData();
    playerData.push_back(newPlayerData);
    //newPlayerData->crystalHP = 100;
    //newPlayerData->crystalType = 0;
    newPlayerData->id = getFreshID();
    newPlayerData->ipAddress = sf::IpAddress::None;
    newPlayerData->isOn = false;
    //newPlayerData->position.x = 0;
    //newPlayerData->position.y = 0;
    return newPlayerData;
}

PlayerData* PlayerList::getNextPlayerData(){
    return nextPlayerData;
}

PlayerData* PlayerList::getConnection( unsigned int playerID ){
    for ( std::vector<PlayerData*>::iterator it = playerData.begin(); it != playerData.end();it++){
        if ( (*it)->id = playerID )
            return (*it);
    }
    return 0;
}

unsigned int test1;
char test2[333];
std::size_t test3;

void PlayerList::process(){
    if ( selector.wait() ){
        acceptNewConnection();
        receiveMessage();
    }
}

//char testBuffer[] = { 1, 12, 1, 1, 1, 1, 2, 0,0,0,0, 0,0,0,0 };

void PlayerList::sendPeerData( PlayerData* connection ){

    //Sending peer-spawning packets
    unsigned current = 0;
/*
    for ( std::vector<PlayerData*>::iterator it = playerData.begin(); it != playerData.end(); ++it ){
        if ( ( (*it)->isOn ) && ( (*it)->crystalType != 0 ) ){
            if ( (*it) != connection ){// If different than current player
                generalBuffer[current] = (*it)->id;
                generalBuffer[current+1] = 1; // packet code - crystal type
                generalBuffer[current+2] = (*it)->crystalType;
                current+=3;
            }
        }
    }

    //Sending player's id
    generalBuffer[current] = connection->id; // player's id
    current++;
    generalBuffer[current] = 12; // packet code ( player id )
    current++;


    //unsigned int current = 5;
    //memcpy(generalBuffer,testBuffer,current);

    connection->tcpSocket.send(generalBuffer, current);
    */
}


void PlayerList::acceptNewConnection(){
    if ( selector.isReady(listener) ){
        PlayerData * newConnection = getNextPlayerData();

        sf::Socket::Status status = listener.accept(newConnection->tcpSocket);
        if ( status == sf::Socket::Status::Done ){
            std::cout << "Somebody connected! " << playerCount+1 << " players now online!" << std::endl;

            selector.add( newConnection->tcpSocket );

            //sendPeerData( newConnection );
            //std::cout << "Sent new player his players id and the others' crystal types" << std::endl;

            newConnection->isOn = true;
            newConnection->ipAddress = newConnection->tcpSocket.getRemoteAddress();
            playerCount++;

            nextPlayerData = createPlayerData(); // relocate next connection etc etc
        }
        else if ( status != sf::Socket::Status::NotReady ){
            std::cout << "Error / Partial / Disconnected socket" << std::endl;
        }
    }
}

char receiveBuffer[256];
std::size_t received;

//sf::Socket::Status PlayerList::receiveMessage(unsigned int& id, char* buffer, std::size_t & received ){
void PlayerList::receiveMessage(){
    sf::Socket::Status status;

    // TCP
    for ( std::vector<PlayerData*>::iterator it = playerData.begin(); it != playerData.end();){
        if ( (*it)->isOn && selector.isReady( (*it)->tcpSocket ) ){
            status = (*it)->tcpSocket.receive(receiveBuffer,256,received);
            if ( status == sf::Socket::Status::Done){
                std::cout << "[TCP]";
                parseBuffer( (*it)->id );
            } else if ( status == sf::Socket::Status::Disconnected ){
                std::cout << "Player Disconnected " << (*it)->id << std::endl;
                (*it)->isOn = false;
                takenIDS[(*it)->id] = false;
                playerCount--;
                selector.remove( (*it)->tcpSocket );

                //Broadcast disconnect to players
                generalBuffer[0] = (char)((*it)->id);
                generalBuffer[1] = 11;
                broadcastTCPMessage(generalBuffer,2);

                delete(*it);//deallocate
                it = playerData.erase(it);//remove from vector

                std::cout << playerCount << " players left ingame!" << std::endl;
                continue;
            }

        }
        ++it;
    }

    // UDP
    for ( std::vector<PlayerData*>::iterator it = playerData.begin(); it != playerData.end();){
        if ( (*it)->isOn && selector.isReady( udpSocket ) ){
            unsigned short int p = 30125;
            status = udpSocket.receive(receiveBuffer,256,received,(*it)->ipAddress,p);
            if ( status == sf::Socket::Status::Done){
                std::cout << "[UDP]";
                parseBuffer( (*it)->id );
            }
            else{
                if ( status == sf::Socket::Status::Error )
                    std::cout << "ERORR ON PACKAGE RECEIVEING (UDP) !!!!!!!!!!!!!!!!" << std::endl;
                else if ( status == sf::Socket::Status::Partial )
                    std::cout << "Partial receive might cause bugs!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
                // TODO : in case of other crashes, insert disconnected status error code and inspect further
            }
        }
        ++it;
    }

}

void PlayerList::parseBuffer(unsigned int player){
    /*
        std::cout << "Received : " << received << " " <<
        ((int*)SyncManager::packageBuffer)[0] << " " <<
        ((int*)SyncManager::packageBuffer)[1] << std::endl;
    */

    //unsigned int streampos = 0;
    char * cursor = receiveBuffer;

    cursor[received] = 0;

    printf ( "Received message from %u\n    %s\n", player, cursor );


    //std::cout << "Received " << received << " bytes." << std::endl;
    /*
    while ( cursor - receiveBuffer < received )
    {
        std::cout<<"BOOM"<<std::endl;
        switch( *cursor ){ // Packet Code
            case 1:{ // Crystal Type
                std::cout << "Type 1 - crystal type" << std::endl;
                //redirectPacket( player, cursor, 2, false );
                proxyTCPMessage( player, cursor, 2, false );
                cursor++;
                //Stage * stage = StageManager::getStage();
                switch( *cursor ){ // Crystal Type Number
                    case 1:{
                        //Emeraldo * emeraldo = new Emeraldo();
                        //stage->addEntity( emeraldo );
                        //SyncManager::crystals[player] = emeraldo;
                        std::cout << "Emeraldo spawned!" << std::endl;
                        break;
                    }
                    case 2:{
                        //Rubie * rubie = new Rubie();
                        //stage->addEntity( rubie );
                        //SyncManager::crystals[player] = rubie;
                        std::cout << "Rubie spawned!" << std::endl;
                        break;
                    }
                    case 3:{
                        //Sapheer * sapheer = new Sapheer();
                        //stage->addEntity( sapheer );
                        //SyncManager::crystals[player] = sapheer;
                        std::cout << "Sapheer spawned!" << std::endl;
                        break;
                    }
                }
                getConnection(player)->crystalType = *cursor;
                cursor++;
                break;
            }
            case 2:{ // Position package
                //std::cout << "Type 2 - position type" << std::endl;
                proxyTCPMessage( player, cursor, 9, true );
                cursor++;
                sf::Int32 x,y;
                memcpy(&x,cursor,4);
                memcpy(&y,cursor+4,4);
                PlayerData * connection = getConnection(player);
                connection->position.x = x;
                connection->position.y = y;
                cursor += 8;
                //PlayerList::detectCollisions();
                break;
            }
            case 3:{ // Basic attack package
                std::cout << "Type 3 - basic attack" << std::endl;
                //redirectPacket( player, cursor, 9, true );
                proxyTCPMessage( player, cursor, 9, true );
                cursor++;

                //memcpy(SyncManager::options, cursor, 8 );
                //float x,y;
                //memcpy(&x, SyncManager::options, 4);
                //memcpy(&y, SyncManager::options+4, 4);
                //std::cout << "RECEIVED COORDS " << x << " " << y << std::endl;

                cursor += 8;
                break;
            }
            case 4:{ // Ultimate package ( no options )
                std::cout << "Type 4 - ultimate ( no options )" << std::endl;
                //redirectPacket( player, cursor, 1, true );
                proxyTCPMessage( player, cursor, 1, true );
                cursor++;
                //memcpy(SyncManager::options,cursor,1);
                //SyncManager::crystals[player]->ultimate();
                break;
            }
            case 5:{ // Ultimate package ( 1 byte )
                std::cout << "Type 5 - ultimate ( 1 byte )" << std::endl;
                //redirectPacket( player, cursor, 2, true );
                proxyTCPMessage( player, cursor, 2, true );
                cursor++;
                //memcpy(SyncManager::options,cursor,1);
                //SyncManager::crystals[player]->ultimate();
                cursor++;
                break;
            }
            case 6:{ // Ultimate package ( 4 bytes )
                std::cout << "Type 6 - ultimate ( 4 bytes )" << std::endl;
                //redirectPacket( player, cursor, 5, true );
                proxyTCPMessage( player, cursor, 5, true );
                cursor++;
                //memcpy(SyncManager::options,cursor,4);
                //SyncManager::crystals[player]->ultimate();
                cursor+=4;
                break;
            }
            case 7:{ // client to server only ( ctso )
                std::cout << "Type 7 - Laser Collision" << std::endl;
                //no redirection
                cursor++;
                int affectedPlayer;
                memcpy(&affectedPlayer,cursor,4);
                damageCrystal( affectedPlayer );
                cursor+=4;
                break;
            }
            case 8:{ // client to server only ( ctso )
                std::cout << "Type 8 - Shield Collision" << std::endl;
                //no redirection
                cursor++;
                int affectedPlayer;
                memcpy(&affectedPlayer,cursor,4);
                //damageCrystal( affectedPlayer );
                pushCrystal(affectedPlayer,player,30.f);
                cursor+=4;
                break;
            }
            case 9:{ // client to server only ( ctso )
                std::cout << "Type 9 - Particle Collision" << std::endl;
                //no redirection
                cursor++;
                int affectedPlayer;
                memcpy(&affectedPlayer,cursor,4);
                damageCrystal( affectedPlayer );
                cursor+=4;
                break;
            }
            //case 8 - sapheer shield collision ( ctso )
            //case 9 - basic attack collision ( ctso )
            //case 10- hp ( stco )
            //case 11- disconnected ( stco )
            //case 12- player ID ( stco )
            //case 13- impulse ( stco )
            default:{
                cursor++;
            }
            //...
        }
    }*/
}


void PlayerList::proxyTCPMessage(unsigned int _id, char* buffer, size_t size, bool skipSender){
    generalBuffer[0] = (char)_id;
    memcpy(generalBuffer+1,buffer,size);

    for ( std::vector<PlayerData*>::iterator it = playerData.begin(); it != playerData.end(); ++it ){
        if ( (*it)->isOn ){

            if ( skipSender && (*it)->id == _id )
                continue;

            (*it)->tcpSocket.send(generalBuffer,size+1);
            //std::cout << "REDIRECTED PACKET" << std::endl;
        }
    }
}

void PlayerList::proxyUDPMessage(unsigned int id, char* buffer, size_t size, bool skipSender){

}

void PlayerList::broadcastTCPMessage(char* buffer, size_t size){
    for ( std::vector<PlayerData*>::iterator it = playerData.begin(); it != playerData.end(); ++it ){
        if ( (*it)->isOn ){
            (*it)->tcpSocket.send(buffer,size);
            //std::cout << "BROADCASTED PACKET" << std::endl;
        }
    }
}

void PlayerList::broadcastUDPMessage(char* buffer, size_t size){

}
/*
void PlayerList::damageCrystal( unsigned int player ){

    PlayerData* connection = getConnection(player);

    if ( connection->crystalHP > 1 ){
        connection->crystalHP--;
        generalBuffer[0] = (char)player;
        generalBuffer[1] = 10;
        memcpy(generalBuffer+2,&connection->crystalHP,4);

        broadcastTCPMessage(generalBuffer,6);
    }
    else{//if the hp is 0
        //TODO - kill
    }
}

void PlayerList::detectCollisions(){
    for ( std::vector<PlayerData*>::iterator i = playerData.begin(); i != playerData.end();i++){
        for ( std::vector<PlayerData*>::iterator j = playerData.begin(); j != playerData.end();j++){
            if ( (*i) != (*j) && (*i)->isOn && (*j)->isOn ){
                sf::Vector2f posI = (*i)->position;
                sf::Vector2f posF = (*j)->position;
                float dist = sqrt( (posI.x-posF.x)*(posI.x-posF.x) + (posI.y-posF.y)*(posI.y-posF.y) );
                if ( dist < 32.f ){
                    pushCrystal((*i)->id,(*j)->id,5.f);
                    pushCrystal((*j)->id,(*i)->id,5.f);
                }
            }
        }
    }
}


void PlayerList::pushCrystal( int affectedPlayer, int player, float force ){
    PlayerData* connectionI = getConnection(affectedPlayer);
    PlayerData* connectionJ = getConnection(player);

    sf::Vector2f velocity; // velocity vector;

    velocity = (connectionI->position) - (connectionJ->position);

    //Multiply it then truncate it ( so you get the same* magnitude each time )
    velocity *= 10000.f;

    Mechanics::applyMaxSpeed( velocity, force );

    generalBuffer[0] = (char)player;
    generalBuffer[1] =  13;// packet code ( impulse 13 )
    memcpy(generalBuffer+2,&(velocity.x),4);
    memcpy(generalBuffer+6,&(velocity.y),4);

    //SyncManager::sockets[affectedPlayer].setBlocking(true);
    connectionI->tcpSocket.send(generalBuffer,10);
    //SyncManager::sockets[affectedPlayer].setBlocking(false);
}*/
