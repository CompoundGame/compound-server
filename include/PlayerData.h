#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include <SFML/Network.hpp>

struct PlayerData{
    unsigned int id;
    //int crystalType;
    //int crystalHP;
    bool isOn;
    sf::TcpSocket tcpSocket;
    //sf::Vector2f position;
    sf::IpAddress ipAddress;
};
#endif // PLAYERDATA_H
