#ifndef PLAYERLIST_H
#define PLAYERLIST_H

#include <PlayerData.h>
#include <vector>

#ifndef MAX_PLAYERS
#define MAX_PLAYERS 16
#endif // MAX_PLAYERS

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
