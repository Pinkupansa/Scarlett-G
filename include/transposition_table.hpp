#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include "hashmap.hpp"
#include <cstdint>
#include "position.hpp"
#include <omp.h>
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

class TranspositionTable{
    private:
        __gnu_pbds::gp_hash_table<uint64_t, TTEntry, chash> table;
    public:
       
        void addEntry(uint64_t hash, int depth, int score, EntryType type){
            bool in;
            #pragma omp critical
            in = table.find(hash) != table.end();

            if(in){
                if(table[hash].depth > depth){
                    return;
                }
            }
            
            TTEntry entry;
            entry.depth = depth;
            entry.score = score;
            entry.type = type;
            #pragma omp critical
            table[hash] = entry;
        }
        bool tryGetEntry(uint64_t hash, TTEntry &entry){
            bool notIn;
            #pragma omp critical
            notIn = table.find(hash) == table.end();

            if(notIn){
                return false;
            }
            
            #pragma omp critical
            entry = table[hash];
            return true;
            
        }
};


#endif // TRANSPOSITION_TABLE_HPP