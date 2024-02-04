#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include "hashmap.hpp"
#include <cstdint>
#include "position.hpp"
#include <omp.h>
#include <tbb/concurrent_hash_map.h>
//define an enum for the different types of entries
enum EntryType
{
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};

class TTEntry
{
    public:
        int depth;
        int score;
        EntryType type;
};

class TranspositionTable {
private:
    tbb::concurrent_hash_map<uint64_t, TTEntry> table;
public:

    inline void addEntry(uint64_t hash, int depth, int score, EntryType type) {
        tbb::concurrent_hash_map<uint64_t, TTEntry>::accessor accessor;
        if (table.find(accessor, hash)) {
            if (accessor->second.depth > depth) {
                return;
            }
        
        }

        TTEntry entry;
        entry.depth = depth;
        entry.score = score;
        entry.type = type;
        
        table.insert(accessor, hash);
        accessor->second = entry;
    }

    inline bool tryGetEntry(uint64_t hash, TTEntry &entry) {
        tbb::concurrent_hash_map<uint64_t, TTEntry>::accessor accessor;
        if (table.find(accessor, hash)) {
            entry = accessor->second;
            return true;
        }
        return false;
    }

    inline int getSize() {
        return table.size();
    }

};

#endif // TRANSPOSITION_TABLE_HPP