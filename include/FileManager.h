#ifndef FILEMANAGER_H
#define FILEMANAGER_H

/// Number of bytes of a file that can be sent at a time
#define MAX_CHUNK_SIZE 512

#define MAX_FILES 32

#include <string>
#include <SFML/Config.hpp>

class FileManager
{
    public:
        static void parseClientFiles();
        static void parseServerFiles();
        static void loadFiles();

        //7df7147c7dd03126041c18e3b6431129


    protected:

    private:
        FileManager();
        static void loadFile( sf::Uint32 id );
        static sf::Uint32 m_fileCount;
        static std::string m_hashes[MAX_FILES];
        static std::string m_fileNames[MAX_FILES];
        static char * m_files[MAX_FILES];
        static sf::Uint32 m_fileSizes[MAX_FILES];
};

#endif // FILEMANAGER_H
