#include "FileManager.h"

#ifdef __cplusplus
# include <lua.hpp>
#else
# include <lua.h>
# include <lualib.h>
# include <lauxlib.h>
#endif

#include "LuaConsole.h"

#include <iostream>

#include <string.h>

/*
        sf::Uint32 m_fileCount;
        std::string m_hashes[MAX_FILES];
        std::string m_fileNames[MAX_FILES];
        char * m_files[MAX_FILES];
        sf::UInt32 m_fileSizes[MAX_FILES];
*/

sf::Uint32 FileManager::m_fileCount;
std::string FileManager::m_hashes[MAX_FILES];
std::string FileManager::m_fileNames[MAX_FILES];
char * FileManager::m_files[MAX_FILES];
sf::Uint32 FileManager::m_fileSizes[MAX_FILES];

void FileManager::parseClientFiles(){
    lua_State * L = LuaConsole::getState();

    lua_settop(L, 0);
    lua_getglobal( L, "client_scripts" );

    if ( lua_type(L,1) == LUA_TTABLE ){
        std::cout << "[Lua] Reading clientside scripts names!" << std::endl;

        int index = 1;
        while( true ){
            lua_pushnumber(L,index);
            lua_gettable(L,1);
            if ( lua_type(L,2) == LUA_TSTRING ){
                m_fileNames[m_fileCount] = lua_tostring(L,2);
                m_fileNames[m_fileCount] = "resources/crystal/" + m_fileNames[m_fileCount];
                std::cout << "[Lua] Loading client script (" << m_fileNames[m_fileCount] << ")" << std::endl;
                m_fileCount++;
            }else
                break;
            lua_settop(L,1);
            index++;
        }
    }else{
        std::cout << "[Lua] No clientside scripts detected!" << std::endl;
    }
    lua_settop(L,0);
}

void FileManager::parseServerFiles(){
    lua_State * L = LuaConsole::getState();

    lua_settop(L, 0);
    lua_getglobal( L, "server_scripts" );

    if ( lua_type(L,1) == LUA_TTABLE ){
        std::cout << "[Lua] Reading serverside scripts names!" << std::endl;

        int index = 1;
        while( true ){
            lua_pushnumber(L,index);
            lua_gettable(L,1);
            if ( lua_type(L,2) == LUA_TSTRING ){
                std::cout << "[Lua] Loading server script (" << lua_tostring(L,2) << ")" << std::endl;
                std::string script = "resources/crystal/";
                script += lua_tostring(L,2);
                LuaConsole::execute( script.c_str() );
            }else
                break;
            lua_settop(L,1);
            index++;
        }
    }else{
        std::cout << "[Lua] No serverside scripts detected!" << std::endl;
    }
    lua_settop(L,0);
}


void FileManager::loadFile(sf::Uint32 id){
   FILE* f = fopen( m_fileNames[id].c_str(), "r" );

   // Determine file size
   fseek(f, 0, SEEK_END);
   size_t size = ftell(f);

   m_fileSizes[id] = size;

   m_files[id] = new char[size];

   rewind(f);
   fread(m_files[id], sizeof(char), size, f);

   fclose(f);
}


void FileManager::loadFiles(){
    for ( sf::Uint32 id; id < m_fileCount; id++ ){
        loadFile(id);
    }
}
