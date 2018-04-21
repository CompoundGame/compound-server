#ifndef PLAYERLIST_H
#define PLAYERLIST_H

#include <PlayerData.h>
#include <vector>

#ifndef MAX_PLAYERS
#define MAX_PLAYERS 16
#endif // MAX_PLAYERS

#include <map>
#include <string>

#include "Object.h"

// Maximum number of server-side ( sync'd objects )
#define MAX_SERVER_OBJECTS 1024

class strless {
   public:
      bool operator() (const std::string & first, const std::string & second ) const  {
         return first < second;
      }
};

class PlayerList
{
    public:
        PlayerList();
        virtual ~PlayerList();

        void init();

        void process();
        void acceptNewConnection();
        void receiveMessage();
        void proxyTCPMessage( unsigned int id, char * buffer, size_t size, bool skipSender );
        void proxyUDPMessage( unsigned int id, char * buffer, size_t size, bool skipSender );
        void broadcastTCPMessage( char * buffer, size_t size );
        void broadcastUDPMessage( char * buffer, size_t size );

        void proxyTCPMessage( unsigned int id, sf::Packet packetToSend );
        void proxyUDPMessage( unsigned int id, sf::Packet packetToSend );
        void broadcastTCPMessage( sf::Packet packetToSend );
        void broadcastUDPMessage( sf::Packet packetToSend );
        void sendTCPMessageTo( sf::Packet packetToSend, int playerID );
        void sendUDPMessageTo( sf::Packet packetToSend, int playerID );

        sf::Uint16 registerObject( const Object & obj );

        void registerServerEvent( const char * eventName, sf::Uint16 id );

        std::map<std::string,sf::Uint16,strless> serverEvents;
        void sendServerEvents( unsigned int player );

        sf::Uint16 registerClientEvent( const char * eventName );
        std::map<std::string,sf::Uint16,strless> clientEvents;

    protected:

    private:
        std::vector<PlayerData*> playerData;
        sf::SocketSelector selector;
        sf::UdpSocket udpSocket;
        sf::TcpListener listener;
        PlayerData* getNextPlayerData();
        PlayerData* createPlayerData();
        PlayerData* nextPlayerData;
        unsigned int playerCount;

        Object * m_objects[MAX_SERVER_OBJECTS];

        PlayerData* getConnection( unsigned int playerID );
        void sendPeerData( PlayerData* connection );
        char generalBuffer[512];

        void parseBuffer( unsigned int playerID );

        //void damageCrystal( unsigned int player );
        //void detectCollisions();
        //void pushCrystal( int affectedPlayer, int player, float force );

        bool takenIDS[MAX_PLAYERS];
        unsigned int getFreshID();
};

#endif // PLAYERLIST_H
