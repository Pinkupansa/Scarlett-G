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
        std::vector<__gnu_pbds::gp_hash_table<uint64_t, PVEntry, chash>> tables;
        void addEntry(uint64_t hash, libchess::Move move, int depth, int tableNum){
            auto &table = tables[tableNum];
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
        bool tryGetEntry(uint64_t hash, PVEntry &entry, int tableNum){
            auto &table = tables[tableNum];
            if(table.find(hash) == table.end()){
                return false;
            }
            entry = table[hash];
            return true;
        }
    public:
        PVTable(){ 
            //table has the size of the max number of threads
            tables = std::vector<__gnu_pbds::gp_hash_table<uint64_t, PVEntry, chash>>(omp_get_max_threads() + 1);
        }
        void addEntry(uint64_t hash, libchess::Move move, int depth){
            addEntry(hash, move, depth, omp_get_thread_num());
        }
        bool tryGetEntry(uint64_t hash, PVEntry &entry){
            return tryGetEntry(hash, entry, omp_get_thread_num());
        }

        int getSize(){
            int size = 0;
            for(auto &table : tables){
                size += table.size();
            }
            return size;
        }

};


#endif // PV_TABLE_HPP