#ifndef PV_TABLE_HPP
#define PV_TABLE_HPP

#include <tbb/concurrent_hash_map.h>
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
    tbb::concurrent_hash_map<uint64_t, PVEntry> table;

public:
    inline void addEntry(uint64_t hash, libchess::Move move, int depth){
        tbb::concurrent_hash_map<uint64_t, PVEntry>::accessor accessor;
        if (table.find(accessor, hash)) {
            if(accessor->second.depth > depth){
                return;
            }
        }

        PVEntry entry;
        entry.depth = depth;
        entry.pvMove = move;
        table.insert(std::make_pair(hash, entry));
    }

    inline bool tryGetEntry(uint64_t hash, PVEntry &entry){
        tbb::concurrent_hash_map<uint64_t, PVEntry>::accessor accessor;
        if (table.find(accessor, hash)) {
            entry = accessor->second;
            return true;
        }
        return false;
    }

    inline int getSize(){
        return table.size();
    }
};

#endif // PV_TABLE_HPP