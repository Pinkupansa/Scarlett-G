// Simple negamax alpha-beta search

#ifndef SIMPLE_MMAB_HPP
#define SIMPLE_MMAB_HPP

#include <position.hpp>
#include <vector>
#include <iostream>
// include hash map
#include <map>
#include "player.hpp"
class ScarlettCore : public Player
{
public:
    ScarlettCore(int color, int depth);
    ~ScarlettCore();
    libchess::Move getMove(libchess::Position pos);
    int evaluate(libchess::Position pos);
    int search(libchess::Position pos, int depth, int alpha, int beta, bool nullMoveAllowed);
    int quiescenceSearch(libchess::Position pos, int alpha, int beta);

private:
    int color;
    int depth;
    int nodesSearched;
    int nCutoffs;
    clock_t timeSpentSorting;
    // transposition table
    std::map<uint64_t, std::pair<int, int>> *transpositionTable;

    int nullMoveSearch(libchess::Position pos, int depth, int alpha, int beta);
    bool futilityPrune(libchess::Position pos, libchess::Move move, int depth, int alpha, int posScore);
    void orderMoves(std::vector<libchess::Move> &moves, libchess::Position pos);
    bool isTactical(libchess::Position pos, libchess::Move move);
    bool multicutPruning(libchess::Position pos, std::vector<libchess::Move> &moves, int depth, int beta, bool nullMoveAllowed);
    int pieceValues[6];
    int materialScore(libchess::Position pos);
};

#endif // SIMPLE_MMAB_HPP
