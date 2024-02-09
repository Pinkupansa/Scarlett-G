#ifndef KILLER_MOVE_TABLE_HPP
#define KILLER_MOVE_TABLE_HPP

#include "hashmap.hpp"
#include <cstdint>
#include "position.hpp"
#include <omp.h>
#include <tbb/concurrent_hash_map.h>
//define an enum for the different types of entries


class KillerMoveTable {
private:
    //tbb::concurrent_hash_map<uint64_t, uint64_t> table;
    bool killerMoveTable[2][64][64][100] = {false};
public:

    inline void addKillerMove(libchess::Side side, libchess::Move move, int depth) { 
        #pragma omp critical
        killerMoveTable[(int)side][(int)move.from()][(int)move.to()][depth] = true;
    }
    inline bool tryGetEntry(libchess::Side side, libchess::Move move, int depth) {
        return killerMoveTable[(int)side][(int)move.from()][(int)move.to()][depth];
    }
    inline void decrementAll(){
        //decrement the depth of all entries
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 64; j++) {
                for (int k = 0; k < 64; k++) {
                    for (int l = 0; l < 100; l++) {
                        if (killerMoveTable[i][j][k][l]) {
                            killerMoveTable[i][j][k][l] = false;
                            if (l > 1) {
                                killerMoveTable[i][j][k][l - 2] = true;
                            }
                        }
                    }
                }
            }
        }
    }

};

#endif // HISTORY_TABLE_HPP