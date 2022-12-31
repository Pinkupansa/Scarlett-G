//Simple negamax alpha-beta search

#ifndef SIMPLE_MMAB_HPP
#define SIMPLE_MMAB_HPP

#include <position.hpp>
#include <vector>
#include <iostream>
//include hash map
#include <map>
#include "player.hpp"
class SimpleMMAB : public Player
{
    public:
        SimpleMMAB(int color, int depth);
        ~SimpleMMAB();
        libchess::Move getMove(libchess::Position pos);
        int evaluate(libchess::Position pos);
        int minimax(libchess::Position pos, int depth, int alpha, int beta, bool nullMoveAllowed);
    
    private:
        
        
        int color;
        int depth;
        int nodesSearched;
        //transposition table
        std::map<uint64_t, std::pair<int, int>>* transpositionTable;

       
        int nullMoveSearch(libchess::Position pos, int depth, int alpha, int beta);

};

#endif // SIMPLE_MMAB_HPP
