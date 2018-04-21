#ifndef SERVER_H
#define SERVER_H

#include <PlayerList.h>

class Server
{
    public:
        static void wait();
        static PlayerList playerList;
        static void process();

        static void init();

    protected:

    private:
        Server();
        virtual ~Server();
};

#endif // SERVER_H
