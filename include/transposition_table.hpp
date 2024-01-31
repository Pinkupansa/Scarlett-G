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
        std::vector<__gnu_pbds::gp_hash_table<uint64_t, TTEntry, chash>> tables;

        void addEntry(uint64_t hash, int depth, int score, EntryType type, int tableNum){
            auto &table = tables[tableNum];
            if(table.find(hash) != table.end()){
                if(table[hash].depth > depth){
                    return;
                }
            }
            
            TTEntry entry;
            entry.depth = depth;
            entry.score = score;
            entry.type = type;
            table[hash] = entry;
        }

        bool tryGetEntry(uint64_t hash, TTEntry &entry, int tableNum){
            auto &table = tables[tableNum];
            if(table.find(hash) == table.end()){
                return false;
            }
            
            entry = table[hash];
            return true;
            
        }

    public:
       TranspositionTable(){
        //table has the size of the max number of threads
        tables = std::vector<__gnu_pbds::gp_hash_table<uint64_t, TTEntry, chash>>(omp_get_max_threads() + 1);
       }
        void addEntry(uint64_t hash, int depth, int score, EntryType type){
            addEntry(hash, depth, score, type, omp_get_thread_num());
        }
        bool tryGetEntry(uint64_t hash, TTEntry &entry){
            return tryGetEntry(hash, entry, omp_get_thread_num());
        }
        int getSize(){
            int size = 0;
            for(int i = 0; i < tables.size(); i++){
                size += tables[i].size();
            }
            return size;
        }
        void mergeAllTables(){
            for(int i = 0; i < tables.size() - 1; i++){
                auto &table = tables[i];
                for (auto const& x : table)
                {
                    addEntry(x.first, x.second.depth, x.second.score, x.second.type, omp_get_max_threads());
                }
                //empty the table
                table.clear();
            }

            //print size of last table
            std::cout << "size of last table: " << tables[tables.size() - 1].size() << std::endl;
        }
        
};


#endif // TRANSPOSITION_TABLE_HPP