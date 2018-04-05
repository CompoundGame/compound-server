#include "SyncManager.h"

#include <string.h>
#include <iostream>

#include "Mechanics.h"
#include "math.h"

/*
struct PlayerData{
    int id;
    int crystalType;
    int crystalHP;
    bool isOn;
    sf::TcpSocket tcpSocket;
    sf::Vector2f position;
};
*/

/*

void GameState::addEntity( Entity * entity )
{
    std::cout << "Added object to gamestate!" << std::endl;
    ps_entities.push_back( entity );
}

void GameState::update(float dt)
{
    for ( std::vector<Entity*>::iterator it = m_entities.begin(); it != m_entities.end();)
    {
        (*it)->update(dt);
        if ( (*it)->isDead() ){
            std::cout<<"ERASED ENTITY"<<std::endl;
            delete (*it);
            it = m_entities.erase(it);
        }
        else
            ++it;
    }

    while ( ps_entities.size() )
    {
        m_entities.push_back ( ps_entities.back() );
        ps_entities.pop_back();
    }
}

void GameState::input( const sf::Event & event )
{
    SyncManager::input(event);
}

void GameState::draw()
{
    if ( SyncManager::myPlayerID != -1 )
        Display::focusOn(SyncManager::crystals[SyncManager::myPlayerID]);
    Display::window->draw(arena);
    Display::window->draw(circle);
    for ( std::vector<Entity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it )
    {
        (*it)->draw();
    }
}

*/

sf::UdpSocket SyncManager::udpSocket;


PlayerList SyncManager::playerList;

int SyncManager::playerCount = 0;

sf::TcpSocket * SyncManager::sockets;

sf::TcpListener SyncManager::listener;
//Crystal * SyncManager::crystals[MAX_PLAYERS];
char * SyncManager::packageBuffer;
char * SyncManager::options;
//int SyncManager::crystalTypes[MAX_PLAYERS];
//int SyncManager::crystalHP[MAX_PLAYERS];
bool SyncManager::playersOn[MAX_PLAYERS];
//bool SyncManager::isAlive[MAX_PLAYERS];
//sf::Vector2f SyncManager::playerPositions[MAX_PLAYERS];
int SyncManager::nextSocket;

sf::SocketSelector SyncManager::selector;

char redirectionBuffer[128];

void SyncManager::init(){
    SyncManager::packageBuffer = new char[128];
    SyncManager::options = new char[128];

    SyncManager::nextSocket = 0;

    SyncManager::sockets = new sf::TcpSocket[MAX_PLAYERS];
    for ( int i = 0; i < MAX_PLAYERS; i++ ){
        //SyncManager::sockets[i] = new sf::TcpSocket;
        //SyncManager::sockets[i].setBlocking(false);
        SyncManager::packageBuffer[i] = 0;
        SyncManager::crystalTypes[i] = 0;
        SyncManager::playersOn[i] = false;
        SyncManager::selector.add( SyncManager::sockets[i] );
    }
    listener.listen(4474);
    SyncManager::selector.add(listener);
    //listener.setBlocking(false);

    std::cout << "SyncManager Initialized!" << std::endl;
}

void SyncManager::acceptConnections(){
    if ( !SyncManager::selector.isReady(listener) )
        return;
    sf::Socket::Status status = listener.accept(SyncManager::sockets[SyncManager::nextSocket]);
    if ( status == sf::Socket::Status::Done ){
        std::cout << "Somebody connected! " << SyncManager::playerCount+1 << " players online!" << std::endl;
        SyncManager::crystalHP[nextSocket] = 100;
        SyncManager::isAlive[nextSocket] = false;

        //Sending peer-spawning packets
        int current = 0;
        for ( int i = 0; i < MAX_PLAYERS; i++ ){
            if ( SyncManager::playersOn[i] ){
                if ( SyncManager::crystalTypes[i] != 0 && SyncManager::isAlive[i] ){
                    if ( i != SyncManager::nextSocket ) // If different than current player
                    {
                        redirectionBuffer[current] = i;
                        redirectionBuffer[current+1] = 1; // packet code - crystal type
                        redirectionBuffer[current+2] = SyncManager::crystalTypes[i];
                        current+=3;
                    }
                }
            }
        }
        //Sending player's id
        redirectionBuffer[current] = SyncManager::nextSocket; // player's id
        current++;
        redirectionBuffer[current] = 12; // packet code ( player id )
        current++;
        //Flush all info to stream
        //SyncManager::sockets[SyncManager::nextSocket].setBlocking(true);
        SyncManager::sockets[SyncManager::nextSocket].send(redirectionBuffer,current);
        std::cout << "SyncManager::playerCount = " << SyncManager::playerCount << "     SyncManager::nextSocket = " << SyncManager::nextSocket << std::endl;
        //SyncManager::sockets[SyncManager::nextSocket].setBlocking(false);

        SyncManager::selector.add( SyncManager::sockets[SyncManager::nextSocket] );

        std::cout << "Sent new player his players id and the others' crystal types" << std::endl;

        SyncManager::playersOn[SyncManager::nextSocket] = true;
        SyncManager::playerCount++;

        for ( int j = 0; j < MAX_PLAYERS; j++ ){ // Relocate nextSocket
            if ( !SyncManager::playersOn[j] ){
                SyncManager::nextSocket = j;
                break;
            }
        }
    }else if ( status != sf::Socket::Status::NotReady )
    {
        std::cout << "Error / Partial / Disconnected socket" << std::endl;
    }
}

void SyncManager::receivePackets(){
    sf::Socket::Status status;
    std::size_t received;
    for ( int i = 0; i < MAX_PLAYERS; i++ ){
        if ( SyncManager::playersOn[i] && SyncManager::selector.isReady(SyncManager::sockets[i]) ){
            status = SyncManager::sockets[i].receive(SyncManager::packageBuffer,128,received);
            if ( status == sf::Socket::Status::Done )
                SyncManager::parseBuffer( i, received );
            else if ( status == sf::Socket::Status::Disconnected ){
                SyncManager::playersOn[i] = false;
                SyncManager::playerCount--;
                SyncManager::isAlive[i] = false;
                SyncManager::crystalHP[i] = 0;
                SyncManager::crystalTypes[i] = 0;
                SyncManager::playerPositions[i] = sf::Vector2f(0.f,0.f);
                for ( int j = 0; j < MAX_PLAYERS; j++ ){ // Relocate nextSocket
                    if ( !SyncManager::playersOn[j] ){
                        SyncManager::nextSocket = j;
                        break;
                    }
                }
                SyncManager::selector.remove( SyncManager::sockets[i] );
                SyncManager::sendDisconnected( i );
                std::cout << "Player Disconnected " << i << std::endl;
            }
        }
    }
}

sf::Clock processClock;
int radius = 1000;

int roundCooldown = 0;

bool RoundStarted = false;

void SyncManager::startRound(){

    //Kill any remainings
    for ( int player = 0; player < MAX_PLAYERS; player++ ){
        if ( SyncManager::isAlive[player] && SyncManager::playersOn[player] ){
            SyncManager::crystalHP[player] = 0;
            //Send message to clients..
            redirectionBuffer[0] = (char)player;
            redirectionBuffer[1] = 15; // packet code 15 dead

            for ( int i = 0; i < MAX_PLAYERS; i++ ){
                if ( SyncManager::playersOn[i] ){
                    //SyncManager::sockets[i].setBlocking(true);
                    SyncManager::sockets[i].send( redirectionBuffer, 2 );
                    //SyncManager::sockets[i].setBlocking(false);
                }
            }
            SyncManager::isAlive[player] = false;
        }
    }

    radius = 500;
    int index = 0;
    for ( int i = 0; i < MAX_PLAYERS; i++ ){
        if ( SyncManager::playersOn[i] && SyncManager::crystalTypes[i] ){

            redirectionBuffer[0] = i; // player
            redirectionBuffer[1] = 1; // packet code = 1 crystaltype
            redirectionBuffer[2] = SyncManager::crystalTypes[i]; // the data

            std::cout << "Composed packet " << i << ' ' << 1 << ' ' << SyncManager::crystalTypes[i] << std::endl;

            SyncManager::crystalHP[i] = 100;
            SyncManager::isAlive[i] = true;

            for ( int j = 0; j < MAX_PLAYERS; j++){
                if ( SyncManager::playersOn[j] ){
                    SyncManager::sockets[j].send( redirectionBuffer, 3 );
                    std::cout << "Packet sent to player " << j << std::endl;
                }
            }
/*
            redirectionBuffer[0] = i;
            redirectionBuffer[1] = 2;
            sf::Int32 x,y;

            x = cos( M_PI * float(index) / float(SyncManager::playerCount) );
            y = sin( M_PI * float(index) / float(SyncManager::playerCount) );
            index++;

            x *= 50;
            y *= 50;

            memcpy( &x, redirectionBuffer+2, 4 );
            memcpy( &y, redirectionBuffer+6, 4 );

            for ( int j = 0; j < MAX_PLAYERS; j++){
                if ( SyncManager::playersOn[j] ){
                    SyncManager::sockets[j].send( redirectionBuffer, 10 );
                    //std::cout << "Packet sent to player " << j << std::endl;
                }
            }
            //Also send some packets regarding positions ( put them in a nice circle, yada yada )
*/
        }
    }
}

void SyncManager::checkRoundEnd(){
    if ( !RoundStarted )
        return;
    int alive = 0;
    for ( int i = 0; i < MAX_PLAYERS; i++ ){
        if ( SyncManager::playersOn[i] && SyncManager::crystalTypes[i] ){ // necessary?
            if ( SyncManager::isAlive[i] ){
                    alive++;
            }
        }
    }
    if ( alive <= 1 ){
            std::cout << "Round end, cooldown activated for respawning" << std::endl;

            RoundStarted = false;
            roundCooldown = 30;
    }
}

void SyncManager::sceneProcess(){
    if( processClock.getElapsedTime() >= sf::seconds(0.09f) ){
        if ( radius -3 >= 8 && RoundStarted ){
            radius -= 3;
            redirectionBuffer[0] = 0;
            redirectionBuffer[1] = 14;
            memcpy(redirectionBuffer+2,&radius,4);
            for ( int i = 0; i < MAX_PLAYERS; i++ ){
                if ( SyncManager::playersOn[i] ){
                    //SyncManager::sockets[i].setBlocking(true);
                    SyncManager::sockets[i].send( redirectionBuffer, 6 );
                    //std::cout << "REDIRECTED PACKET" << std::endl;
                    //SyncManager::sockets[i].setBlocking(false);
                }
            }
        }
        damageOutsideCircle();
        processClock.restart();

        if ( roundCooldown )
            roundCooldown--;

        if ( RoundStarted == false && SyncManager::playerCount >= 2 && roundCooldown == 0 )
        {
            int validCount = 0;
            for ( int i = 0; i < MAX_PLAYERS; i++ ){
                if ( SyncManager::playersOn[i] && SyncManager::crystalTypes[i] ){
                    validCount++;
                }
            }
            if ( validCount == SyncManager::playerCount ){
                std::cout << "Round valid and can start!" << std::endl;
                RoundStarted = true;
                SyncManager::startRound();
            }
        }


        //during round ... yada yada... prevent trigger every time etc
        SyncManager::checkRoundEnd();
    }
}

void SyncManager::damageOutsideCircle(){
    for ( int i = 0; i < MAX_PLAYERS; i++ ){
        if ( SyncManager::playersOn[i] ){
            sf::Vector2f posI = SyncManager::playerPositions[i];
            float dist = sqrt( (posI.x)*(posI.x) + (posI.y)*(posI.y) );
            //std::cout << "Dist | Radius " << posI.x << " " << posI.y << "     " <<dist << " " << radius << std::endl;
            if ( dist > radius ){
                damageCrystal(i);
            }
        }
    }
}

void SyncManager::parseBuffer( int player, std::size_t received ){
    /*
        std::cout << "Received : " << received << " " <<
        ((int*)SyncManager::packageBuffer)[0] << " " <<
        ((int*)SyncManager::packageBuffer)[1] << std::endl;
    */

    //unsigned int streampos = 0;
    char * cursor = SyncManager::packageBuffer;

    //std::cout << "Received " << received << " bytes." << std::endl;

    while ( cursor - SyncManager::packageBuffer < received )
    {
        switch( *cursor ){ // Packet Code
            case 1:{ // Crystal Type
                std::cout << "Type 1 - crystal type" << std::endl;
                //redirectPacket( player, cursor, 2, false );
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
                SyncManager::crystalTypes[player] = *cursor;
                //radius = 1000;
                cursor++;
                break;
            }
            case 2:{ // Position package
                //std::cout << "Type 2 - position type" << std::endl;
                redirectPacket( player, cursor, 9, true );
                cursor++;
                sf::Int32 x,y;
                memcpy(&x,cursor,4);
                memcpy(&y,cursor+4,4);
                SyncManager::playerPositions[player].x = x;
                SyncManager::playerPositions[player].y = y;
                //SyncManager::crystals[player]->setPosition(sf::Vector2f(x,y));
                cursor += 8;
                SyncManager::detectCollisions();
                break;
            }
            case 3:{ // Basic attack package
                std::cout << "Type 3 - basic attack" << std::endl;
                redirectPacket( player, cursor, 9, true );
                cursor++;
                //memcpy(SyncManager::options, cursor, 8 );
                /*float x,y;
                memcpy(&x, SyncManager::options, 4);
                memcpy(&y, SyncManager::options+4, 4);
                std::cout << "RECEIVED COORDS " << x << " " << y << std::endl;*/
                //SyncManager::crystals[player]->attack();
                cursor += 8;
                break;
            }
            case 4:{ // Ultimate package ( no options )
                std::cout << "Type 4 - ultimate ( no options )" << std::endl;
                redirectPacket( player, cursor, 1, true );
                cursor++;
                //memcpy(SyncManager::options,cursor,1);
                //SyncManager::crystals[player]->ultimate();
                break;
            }
            case 5:{ // Ultimate package ( 1 byte )
                std::cout << "Type 5 - ultimate ( 1 byte )" << std::endl;
                redirectPacket( player, cursor, 2, true );
                cursor++;
                //memcpy(SyncManager::options,cursor,1);
                //SyncManager::crystals[player]->ultimate();
                cursor++;
                break;
            }
            case 6:{ // Ultimate package ( 4 bytes )
                std::cout << "Type 6 - ultimate ( 4 bytes )" << std::endl;
                redirectPacket( player, cursor, 5, true );
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
            //case 14:{ // Circle Radius
            //case 15: // dead crystal
            default:{
                cursor++;
            }
            //...
        }
    }
}

/** @brief Redirects Packet from a client to peers ( including packet type ) */
void SyncManager::redirectPacket( int receivedFrom, char * buffer, int size, bool skipSender ){
    redirectionBuffer[0] = (char)receivedFrom;
    memcpy(redirectionBuffer+1,buffer,size);
    for ( int i = 0; i < MAX_PLAYERS; i++ ){
        if ( SyncManager::playersOn[i] ){
            if ( skipSender && i == receivedFrom )
                continue;
            //SyncManager::sockets[i].setBlocking(true);
            SyncManager::sockets[i].send( redirectionBuffer, size+1 );
            //std::cout << "REDIRECTED PACKET" << std::endl;
            //SyncManager::sockets[i].setBlocking(false);
        }
    }
}

void SyncManager::damageCrystal( int player ){
    if ( SyncManager::crystalHP[player] > 1 ){
        SyncManager::crystalHP[player]--;
        //Send message to clients..
        redirectionBuffer[0] = (char)player;
        redirectionBuffer[1] = 10; // packet code 10 hp
        memcpy(redirectionBuffer+2,SyncManager::crystalHP+player,4);

        for ( int i = 0; i < MAX_PLAYERS; i++ ){
            if ( SyncManager::playersOn[i] ){
                //SyncManager::sockets[i].setBlocking(true);
                SyncManager::sockets[i].send( redirectionBuffer, 6 );
                //SyncManager::sockets[i].setBlocking(false);
            }
        }
    }
    else{//if the hp is 0
        // KILL
        if ( SyncManager::isAlive[player] ){
            SyncManager::crystalHP[player] = 0;
            //Send message to clients..
            redirectionBuffer[0] = (char)player;
            redirectionBuffer[1] = 15; // packet code 15 dead

            for ( int i = 0; i < MAX_PLAYERS; i++ ){
                if ( SyncManager::playersOn[i] ){
                    //SyncManager::sockets[i].setBlocking(true);
                    SyncManager::sockets[i].send( redirectionBuffer, 2 );
                    //SyncManager::sockets[i].setBlocking(false);
                }
            }
            SyncManager::isAlive[player] = false;
        }
    }
}

void SyncManager::pushCrystal( int affectedPlayer, int player, float force ){
    sf::Vector2f velocity; // velocity vector;

    velocity = SyncManager::playerPositions[affectedPlayer]-
               SyncManager::playerPositions[player];

    //Multiply it then truncate it ( so you get the same* magnitude each time )
    velocity *= 10000.f;

    Mechanics::applyMaxSpeed( velocity, force );

    redirectionBuffer[0] = (char)player;
    redirectionBuffer[1] =  13;// packet code ( impulse 13 )
    memcpy(redirectionBuffer+2,&(velocity.x),4);
    memcpy(redirectionBuffer+6,&(velocity.y),4);

    //SyncManager::sockets[affectedPlayer].setBlocking(true);
    SyncManager::sockets[affectedPlayer].send(redirectionBuffer,10);
    //SyncManager::sockets[affectedPlayer].setBlocking(false);
}

void SyncManager::sendDisconnected( int player ){
    SyncManager::packageBuffer[0] = (char)player;
    SyncManager::packageBuffer[1] = 11; // packet code 11 disconnected
    for ( int i = 0; i < MAX_PLAYERS; i++ ){
        if ( SyncManager::playersOn[i] ){
            //SyncManager::sockets[i].setBlocking(true);
            SyncManager::sockets[i].send( SyncManager::packageBuffer, 2 );
            //SyncManager::sockets[i].setBlocking(false);
        }
    }
}


//Hardcoded but does the job
void SyncManager::detectCollisions(){
    for ( int i = 0; i < MAX_PLAYERS; i++ ){
        if ( SyncManager::playersOn[i] ){
            sf::Vector2f posI = SyncManager::playerPositions[i];
            for ( int j = 0; j < MAX_PLAYERS; j++ ){
                if ( SyncManager::playersOn[j] && i != j ){
                    sf::Vector2f posF = SyncManager::playerPositions[j];
                    float dist = sqrt( (posI.x-posF.x)*(posI.x-posF.x) + (posI.y-posF.y)*(posI.y-posF.y) );
                    if ( dist < 32.f ){
                        pushCrystal(i,j,5.f);
                        pushCrystal(j,i,5.f);
                    }
                }
            }
        }
    }
}

//char SyncManager::packageBuffer[128];

//char * SyncManager::packageBuffer = new char[128];
/*
void SyncManager::sendPosition(const sf::Vector2f& position){
    sf::Int32 x = position.x;
    sf::Int32 y = position.y;
    memcpy(SyncManager::packageBuffer,&x,sizeof(sf::Int32));
    memcpy(SyncManager::packageBuffer+sizeof(sf::Int32),&y,sizeof(sf::Int32));
    SyncManager::socket.send(packageBuffer,sizeof(sf::Int32)*2);
}*/
