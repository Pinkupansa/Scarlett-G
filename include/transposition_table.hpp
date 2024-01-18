#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include <map>
#include <cstdint>


//define an enum for the different types of entries
enum EntryType
{
    EXACT,
    BETA,
    ALPHA
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
        std::map<uint64_t, TTEntry> table;
    public:
        TranspositionTable(){
            table = std::map<uint64_t, TTEntry>();
        }
        void addEntry(uint64_t hash, int depth, int score, EntryType type){
            TTEntry entry;
            entry.depth = depth;
            entry.score = score;
            entry.type = type;
            table[hash] = entry;
        }
        bool tryGetEntry(uint64_t hash, TTEntry &entry){
            if(table.find(hash) == table.end()){
                return false;
            }
            entry = table[hash];
            return true;
        }
};


#endif // TRANSPOSITION_TABLE_HPP