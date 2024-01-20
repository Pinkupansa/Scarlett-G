#ifndef PV_TABLE_HPP
#define PV_TABLE_HPP

#include <unordered_map>
#include <cstdint>
#include "position.hpp"

class PVEntry
{
    public:
        int depth;
        libchess::Move pvMove;
};

class PVTable{
    private:
        std::unordered_map<uint64_t, PVEntry> table;
    public:
        PVTable(){
            table = std::unordered_map<uint64_t, PVEntry>();
        }
        void addEntry(uint64_t hash, libchess::Move move, int depth){
            if(table.find(hash) != table.end()){
                if(table[hash].depth > depth){
                    return;
                }
            }
            
            PVEntry entry;
            entry.depth = depth;
            entry.pvMove = move;

            table[hash] = entry;
        }
        bool tryGetEntry(uint64_t hash, PVEntry &entry){
            if(table.find(hash) == table.end()){
                return false;
            }
            entry = table[hash];
            return true;
        }
};


#endif // PV_TABLE_HPP