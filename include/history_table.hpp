#ifndef HISTORY_TABLE_HPP
#define HISTORY_TABLE_HPP

#include "hashmap.hpp"
#include <cstdint>
#include "position.hpp"
#include <omp.h>
#include <tbb/concurrent_hash_map.h>
//define an enum for the different types of entries


class HistoryTable {
private:
    //tbb::concurrent_hash_map<uint64_t, uint64_t> table;
    uint64_t scoreTable[2][64][64] = { 0 };
    int trialsTable[2][64][64] = { 0 };
public:

    inline void increaseHistoryScore(libchess::Side side, libchess::Move move, int depth) {
        #pragma omp critical
        scoreTable[(int)side][(int)move.from()][(int)move.to()] += 1<<depth;
    }

    inline void increaseTrialCount(libchess::Side side, libchess::Move move){
        #pragma omp critical
        trialsTable[(int)side][(int)move.from()][(int)move.to()]++;
    }

    inline bool tryGetEntry(libchess::Side side, libchess::Move move, uint64_t& score) {
        int nbTrials = trialsTable[(int)side][(int)move.from()][(int)move.to()];
        if(nbTrials > 0){
            score = scoreTable[(int)side][(int)move.from()][(int)move.to()];
            score /= trialsTable[(int)side][(int)move.from()][(int)move.to()];
            return true;
        }
        
        return false;
    }

    inline void print() {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                std::cout << scoreTable[0][i][j] << " ";
            }
            std::cout << std::endl;
        }

         for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                std::cout << scoreTable[1][i][j] << " ";
            }
            std::cout << std::endl;
        }

    }

};

#endif // HISTORY_TABLE_HPP