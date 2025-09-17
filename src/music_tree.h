
#pragma once

#include <list>
#include <string>

namespace starling
{
    class MusicTree;
    class MusicEntry
    {
    public:
        MusicEntry(const std::string& file_name);
    private:
        std::string file_name = "";
    }

    std::list< MusicEntry > listSongs(const std::string& path)
    {
        
    }
}