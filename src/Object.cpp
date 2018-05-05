#include "Object.h"

#include "Events.h"

#include <iostream>

Object::Object(){
    serverID = textureID = 0;
    x = y = vx = vy = rotation = friction = 0.f;
    dead = false;
    std::cout << "Object created!" << std::endl;
}

Object::~Object(){
    //dtor
}

void Object::update(float dt){
    // Update position
    x += vx;
    y += vy;

    // Update velocity
    vx -= vx*friction;
    vy -= vy*friction;
}

bool Object::isDead(){
    return dead;
}

sf::Packet Object::setPosition(const sf::Vector2f& position)
{
    x = position.x;
    y = position.y;

    sf::Packet newPacket;
    newPacket << SHARED_POSITION << serverID << x << y;
    return newPacket;
}

sf::Packet Object::setPosition(const float _x, const float _y){
    x = _x;
    y = _y;

    sf::Packet newPacket;
    newPacket << SHARED_POSITION << serverID << x << y;
    return newPacket;
}

sf::Vector2f Object::getPosition(){
    return sf::Vector2f(x,y);
}

void Object::getPosition(float& _x, float& _y)
{
    _x = x;
    _y = y;
}

sf::Packet Object::kill(){
    dead = true;

    sf::Packet newPacket;
    newPacket << SHARED_KILL << serverID;
    return newPacket;
}

sf::Packet Object::setRotation(float rot){
    rotation = rot;

    sf::Packet newPacket;
    newPacket << SHARED_ROTATION << serverID << rotation;
    return newPacket;
}

float Object::getRotation(){
    return rotation;
}

sf::Packet Object::setTextureID(sf::Uint16 id){
    textureID = id;

    sf::Packet newPacket;
    newPacket << SHARED_TEXTUREID << serverID << textureID;
    return newPacket;
}

sf::Uint16 Object::getTextureID(){
    return textureID;
}

sf::Packet Object::setVelocity(const sf::Vector2f& velocity){
    vx = velocity.x;
    vy = velocity.y;

    sf::Packet newPacket;
    newPacket << SHARED_VELOCITY << serverID << vx << vy << x << y;
    return newPacket;
}


sf::Packet Object::setVelocity(const float velx, const float vely){
    vx = velx;
    vy = vely;

    sf::Packet newPacket;
    newPacket << SHARED_VELOCITY << serverID << vx << vy << x << y;
    return newPacket;
}

sf::Vector2f Object::getVelocity(){
    return sf::Vector2f(vx,vy);
}

void Object::getVelocity(float& velx, float& vely){
    velx = vx;
    vely = vy;
}

sf::Packet Object::setFriction(float fric){
    friction = fric;

    sf::Packet newPacket;
    newPacket << SHARED_FRICTION << serverID << fric;
    return newPacket;
}

float Object::getFriction(){
    return friction;
}


sf::Packet Object::generateObjectPacket(){
    sf::Packet newPacket;
    newPacket << C_SINGLE_OBJECT << serverID << x << y << vx << vy << friction << rotation << textureID;
    return newPacket;
}
