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

#define C_REQUEST_SERVER_EVENTS 1
#define S_SEND_SERVER_EVENTS 1
#define C_REGISTER_CLIENT_EVENT 2
#define S_CLIENT_EVENT_ACKNOWLEDGEMENT 2


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

    PlayerList::registerServerEvent("invaders:popVirus",502);

    //std::cout << PlayerList::registerClientEvent("convex:test") << std::endl;
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

void PlayerList::registerServerEvent( const char* eventName, sf::Uint16 id )
{
    if ( serverEvents.find(eventName) == serverEvents.end() ){
        serverEvents[eventName] = id;

        std::cout << "Registered server event " << eventName << ' ' << id << std::endl;

        sf::Packet newEvent;
        sf::Uint16 packetCode = 1;
        newEvent << packetCode << eventName << id;
        PlayerList::broadcastTCPMessage(newEvent);
    }
    else{
        std::cout << "Event already registered " << eventName << std::endl;
    }
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
        if ( (*it)->id == playerID )
            return (*it);
    }
    return 0;
}

/*
unsigned int test1;
char test2[333];
std::size_t test3;
*/
void PlayerList::process(){
    if ( selector.wait() ){
        acceptNewConnection();
        receiveMessage();
    }
}

//char testBuffer[] = { 1, 12, 1, 1, 1, 1, 2, 0,0,0,0, 0,0,0,0 };

void PlayerList::sendPeerData( PlayerData* connection ){

    //Sending peer-spawning packets
    //unsigned current = 0;
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

sf::Packet packet;

//sf::Socket::Status PlayerList::receiveMessage(unsigned int& id, char* buffer, std::size_t & received ){
void PlayerList::receiveMessage(){
    sf::Socket::Status status;

    // TCP
    for ( std::vector<PlayerData*>::iterator it = playerData.begin(); it != playerData.end();){
        if ( (*it)->isOn && selector.isReady( (*it)->tcpSocket ) ){
            //status = (*it)->tcpSocket.receive(receiveBuffer,256,received);
            status = (*it)->tcpSocket.receive(packet);
            if ( status == sf::Socket::Status::Done){
                std::cout << "[TCP] ";
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
            //status = udpSocket.receive(receiveBuffer,256,received,(*it)->ipAddress,p);
            status = udpSocket.receive(packet,(*it)->ipAddress,p);
            if ( status == sf::Socket::Status::Done){
                std::cout << "[UDP] ";
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

sf::Uint16 clientEvent = 50; // start allocation at 50

sf::Uint16 PlayerList::registerClientEvent(const char* eventName){
    std::map<std::string,sf::Uint16,strless>::const_iterator it = clientEvents.find(eventName);

    if ( it == clientEvents.end() ){
        sf::Uint16 id = clientEvent;
        clientEvent++;
        clientEvents[eventName] = id;
        std::cout << "Registered client event " << eventName << ' ' << id << std::endl;
        return id;
    }
    else{
        std::cout << "Client Event already registered " << eventName << ' ' << it->second << std::endl;
        return it->second;
    }
}

void PlayerList::sendServerEvents( unsigned int player ){
    packet.clear();
    sf::Uint16 packetCode = 1;
    packet << packetCode;

    for ( std::map<std::string,sf::Uint16,strless>::const_iterator i = serverEvents.begin(); i != serverEvents.end(); ++i ) {
        //cout << (*i).first << " " << (*i).second << endl;
        packet << (*i).first << (*i).second;
        std::cout << (*i).first << (*i).second << std::endl;
        // packet << STRING_NAME << INT_CODE
    }

    sf::Socket::Status stat = getConnection(player)->tcpSocket.send(packet);
    std::cout << stat << ' ' << sf::Socket::Status::Done << std::endl;
}

void PlayerList::parseBuffer(unsigned int player){
    sf::Uint16 packetCode;

    packet >> packetCode;

    std::cout << "Player " << player << " sent : " << packetCode << std::endl;
    if ( !packet.endOfPacket() ){
        std::cout << " with some extra data" << std::endl;
    }

    switch ( packetCode ){
        case 1: // Request Server Events
            PlayerList::sendServerEvents( player );
            std::cout << "Sent server events to player " << player << std::endl;
            break;
        case 2:{ // Reguest Client Event Code
            sf::Uint16 clientEventCode;
            std::string eventName;
            packet >> eventName;
            clientEventCode = PlayerList::registerClientEvent(eventName.c_str());

            packet.clear();
            packet << (sf::Uint16)(2);
            packet << eventName;
            packet << clientEventCode;
            getConnection(player)->tcpSocket.send(packet);
            break;
        }
        case 502: // debug purpose
            registerServerEvent("vrp:bankrobbery",5010);
            break;
    }
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

void PlayerList::broadcastTCPMessage(sf::Packet packetToSend ){
    for ( std::vector<PlayerData*>::iterator it = playerData.begin(); it != playerData.end(); ++it ){
        if ( (*it)->isOn ){
            //What if the socket disconnected in the meantime?
            (*it)->tcpSocket.send(packetToSend);
            //std::cout << "BROADCASTED PACKET" << std::endl;
        }
    }
}

void PlayerList::broadcastUDPMessage(sf::Packet packetToSend ){

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
