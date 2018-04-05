#ifndef SERVER_H
#define SERVER_H

#include <PlayerList.h>

class Server
{
    public:
        Server();
        virtual ~Server();

        void wait();
        PlayerList playerList;
        void process();

        void init();

    protected:

    private:
};

#endif // SERVER_H
