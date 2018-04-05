#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <SFML/Network.hpp>
#include <PlayerList.h>

#define MAX_PLAYERS 16

class SyncManager
{
    public:
        static void init();
        static void acceptConnections();
        static void receivePackets();

        /**Redirect Packet from a client to peers ( including packet type )*/
        static void redirectPacket( int receivedFrom, char * buffer, int size, bool skipSender );

        //static void sceneProcess();
        //static void startRound();
        //static void checkRoundEnd();

        static int playerCount;

        static sf::UdpSocket udpSocket;
        static sf::TcpListener listener;
        static char * packageBuffer;

        static sf::TcpSocket * sockets;
        //static int crystalTypes[MAX_PLAYERS];
        //static int crystalHP[MAX_PLAYERS];
        static bool playersOn[MAX_PLAYERS];
        //static bool isAlive[MAX_PLAYERS];
        //static sf::Vector2f playerPositions[MAX_PLAYERS];

        static PlayerList playerList;

        static int nextSocket;

        static char * options;

        //static void damageCrystal( int player );
        //static void pushCrystal( int affectedPlayer, int player, float force );
        static void sendDisconnected( int player );

        //static void detectCollisions();
        //static void damageOutsideCircle();

        static sf::SocketSelector selector;
    protected:

    private:
        SyncManager();
        static void parseBuffer( int player, std::size_t received );
};

#endif // SYNCMANAGER_H
