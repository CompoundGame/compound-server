#include "LuaConsole.h"
#include "SyncManager.h"
#include <string.h>
#include <string>

#include "Server.h"

#include <iostream>


lua_State * LuaConsole::state = NULL;


//LUA print() redefinition
/*
int l_my_print(lua_State* L) {
    int nargs = lua_gettop(L);

    for (int i=1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
              // Pop the next arg using lua_tostring(L, i) and do your print
              puts( lua_tostring(L,i) );
        }
        else {

        }
    }

    return 0;
}*/

// SetPosition( object, x, y )
int l_SetPosition(lua_State* L) {
/*
    int objectID;
    float x,y;

    if ( luaL_checkinteger(L,-3) ){
        objectID = lua_tonumber(L,-3);
    }

    x = lua_tonumber(L,-2);
    y = lua_tonumber(L,-1);

    StageManager::gameState->getObject(objectID)->setPosition(x,y);

    return 0;
*/
}

// CreateObject()
int l_CreateObject(lua_State* L){
    /*
    Object * newObject = new Object();
    sf::Uint16 id = StageManager::gameState->registerClientObject( newObject );

    lua_pushnumber( L, id );

    return 1;
    */
}

sf::Uint16 serverEventNextCode = 500;

// RegisterServerEvent ( eventName )
int l_RegisterServerEvent(lua_State* L) {
    char eventName[256];

    strcpy(eventName,lua_tostring(L,-1));

    Server::playerList.registerServerEvent(eventName,serverEventNextCode);
    serverEventNextCode++;

    return 0;
}

// TriggerServerEvent ( eventName )
int l_TriggerServerEvent(lua_State* L) {
    /*
    char eventName[256];
    strcpy(eventName,lua_tostring(L,-1));
    SyncManager::triggerServerEvent(eventName);

    return 0;
    */
}

// RegisterClientEvent ( eventName )
int l_RegisterClientEvent(lua_State* L) {
    /*
    char eventName[256];

    strcpy(eventName,lua_tostring(L,-1));

    SyncManager::requestClientEvent(eventName);

    return 0;
    */
}

// AddEventHandler ( eventName, eventHandler )
int l_AddEventHandler(lua_State* L) {

    //char eventName[256];
    //strcpy(eventName,lua_tostring(L,-2));

    //TODO : checktypes

    lua_settable(L, LUA_REGISTRYINDEX);

    return 0;
}

void LuaConsole::triggerServerEvent( sf::Uint16 eventCode, sf::Packet restOfPacket ){
    std::cout << "DEBUG 1 " << std::endl;

    for (std::map<std::string,sf::Uint16>::iterator it=Server::playerList.serverEvents.begin(); it!=Server::playerList.serverEvents.end(); ++it){
        if ( it->second == eventCode ){

            std::cout << "DEBUG 2 " << std::endl;

            std::string eventName = it->first;

            lua_pushstring(LuaConsole::state, eventName.c_str()); // push the event name
            lua_gettable(LuaConsole::state, LUA_REGISTRYINDEX); // retrieve the function

            if ( ! lua_isnil(LuaConsole::state,-1) ){

                std::cout << "DEBUG 3 " << std::endl;

                sf::Uint8 encoding;
                restOfPacket >> encoding;
                char index = 1;

                float floatBuffer;
                std::string stringBuffer;

                int args = 0;

                while ( !restOfPacket.endOfPacket() ){
                    if ( index & encoding ){ // 1 for string, 0 for number
                        // string
                        restOfPacket >> stringBuffer;
                        lua_pushstring(LuaConsole::state,stringBuffer.c_str());
                        std::cout << "Pushed a string : " << stringBuffer.c_str() << std::endl;
                    }else
                    {
                        restOfPacket >> floatBuffer;
                        lua_pushnumber(LuaConsole::state, floatBuffer);
                        std::cout << "Pushed a float : " << floatBuffer << std::endl;
                    }

                    index = index << 1;
                    args++;
                }

                std::cout << "DEBUG 4 " << std::endl;

                std::cout << "args : " << args << std::endl;

                //Now the function is below the args
                lua_call(LuaConsole::state,args,0);
            }
            return;
        }
    }

    std::cout << "No such event with code " << eventCode << std::endl;


}

void LuaConsole::init(){
    if ( state == NULL ){
        state = luaL_newstate();

        // Make standard libraries available in the Lua object
        luaL_openlibs(state);

        //lua_pushcfunction(state, l_my_print);
        //lua_setglobal(state, "print");

        /*
        lua_pushcfunction(state, l_SetPosition);
        lua_setglobal(state, "SetPosition");

        lua_pushcfunction(state, l_TriggerServerEvent);
        lua_setglobal(state, "TriggerServerEvent");

        lua_pushcfunction(state, l_AddEventHandler);
        lua_setglobal(state, "AddEventHandler");

        lua_pushcfunction(state, l_CreateObject);
        lua_setglobal(state, "CreateObject");
        */

        lua_pushcfunction(state, l_AddEventHandler);
        lua_setglobal(state, "AddEventHandler");

        lua_pushcfunction(state, l_RegisterServerEvent);
        lua_setglobal(state, "RegisterServerEvent");


    }
}

void LuaConsole::print_error(lua_State* state) {
  // The error message is on top of the stack.
  // Fetch it, print it and then pop it off the stack.
  const char* message = lua_tostring(state, -1);
  puts(message);
  lua_pop(state, 1);
}

void LuaConsole::execute(const char* filename)
{
  int result;

  // Load the program; this supports both source code and bytecode files.
  result = luaL_loadfile(state, filename);

  if ( result != LUA_OK ) {
    print_error(state);
    return;
  }

  // Finally, execute the program by calling into it.
  // Change the arguments if you're not running vanilla Lua code.

  printf("[Lua] File loaded correctly %s\n", filename);

  result = lua_pcall(state, 0, LUA_MULTRET, 0);

  if ( result != LUA_OK ) {
    print_error(state);
    return;
  }
}

void LuaConsole::executeLine( const char * s ){
    int result = luaL_loadstring ( state, s );
    switch( result ){
        case LUA_ERRSYNTAX:
            puts("[Lua] Error executing line ( syntax ) !");
            break;
        case LUA_ERRMEM:
            puts("[Lua] Error executing line ( memory ) !");
            break;
        default:{
            int res = lua_pcall(state,0,LUA_MULTRET, 0 );
            if ( res != LUA_OK ) {
                print_error(state);
                return;
            }
        }
    }
}
