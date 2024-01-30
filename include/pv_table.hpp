#ifndef PV_TABLE_HPP
#define PV_TABLE_HPP

#include "hashmap.hpp"
#include <cstdint>
#include "position.hpp"
#include <omp.h>
class PVEntry
{
    public:
        int depth;
        libchess::Move pvMove;
};

class PVTable{
    private:
        __gnu_pbds::gp_hash_table<uint64_t, PVEntry, chash> table;
    public:
        void addEntry(uint64_t hash, libchess::Move move, int depth){
            if(table.find(hash) != table.end()){
                if(table[hash].depth > depth){
                    return;
                }
            }
            
            PVEntry entry;
            entry.depth = depth;
            entry.pvMove = move;
            #pragma omp critical
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