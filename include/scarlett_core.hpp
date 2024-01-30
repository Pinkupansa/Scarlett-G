// Simple negamax alpha-beta search

#ifndef SIMPLE_MMAB_HPP
#define SIMPLE_MMAB_HPP

#include <position.hpp>
#include <vector>
#include <iostream>
#include <unordered_set>
// include hash map
#include <map>
#include "player.hpp"
#include "hashmap.hpp"
#include "opening_book.hpp"
#include "transposition_table.hpp"
#include "pv_table.hpp"

struct MoveHash
{
    std::size_t operator()(const libchess::Move &move) const
    {
        std::size_t h1 = move.from().file() + 8 * move.from().rank();
        std::size_t h2 = move.to().file() + 8 * move.to().rank();
        std::size_t h3 = (int)move.piece();
        return h1 ^ h2 ^ h3;
    }
};

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        auto h1 = MoveHash{}(p.first);
        auto h2 = std::hash<int>{}(p.second);

        return h1 ^ h2;
    }
};

class ScarlettCore : public Player
{
public:
    ScarlettCore(int color, int depth);
    ~ScarlettCore();
    libchess::Move getMove(libchess::Position pos);
    int evaluate(libchess::Position &pos, int alpha, int beta);
    int search(libchess::Position &pos, int depth, int alpha, int beta, bool nullMoveAllowed);
    
    void setWeights(int weights[20]);
    void setPieceValues(int pieceValues[6]);
    void printWeights();
    int PAWN_VALUE = 100;
    int KNIGHT_VALUE = 300;
    int BISHOP_VALUE = 300;
    int ROOK_VALUE = 500;
    int QUEEN_VALUE = 900;
    int KING_VALUE = 10000;

    // int PAWN_ADVANCE_A = 10;
    // int PAWN_ADVANCE_B = 20;
    int PASSED_PAWN_MULT = 10;
    int ISOLATED_PAWN_PENALTY = 10;
    int DOUBLED_PAWN_PENALTY = 10;
    // int BACKWARD_PAWN_PENALTY = 5;
    // int WEAK_SQUARE_PENALTY = 5;
    // int PASSED_PAWN_ENEMY_KING_DISTANCE = 5;
    int KNIGHT_MOBILITY = 8;
    int BISHOP_MOBILITY = 7;
    int BISHOP_PAIR = 28;
    int ROOK_ATTACK_KING_FILE = 51;
    int ROOK_ATTACK_KING_ADJ_FILE = 8;
    // int ROOK_ATTACK_KING_ADJ_FILE_ABGH = 26;
    int ROOK_7TH_RANK = 30;
    int ROOK_CONNECTED = 6;
    int ROOK_MOBILITY = 6;
    // int ROOK_BEHIND_PASSED_PAWN = 10;
    int ROOK_OPEN_FILE = 27;
    int ROOK_SEMI_OPEN_FILE = 15;
    int QUEEN_MOBILITY = 5;
    int KING_FRIENDLY_PAWNS = 35;
    // int KING_NO_FRIENDLY_PAWN_ADJ = 10;
    // int KING_PRESSURE_MULT = 5;

    int NULL_MOVE_REDUCTION = 3;
    int USE_ADPT = 1;
    int ADPT_DEPTH = 6;
    int FUTILITY_DEPTH = 3;
    int FUTILITY_MARGIN_1 = 300;
    int FUTILITY_MARGIN_2 = 500;
    int FUTILITY_MARGIN_3 = 900;

    int MULTICUT_REDUCTION = 3;
    int MULTICUT_DEPTH = 6;
    int MC_NUM = 10;
    int MC_CUT = 3;

    int QUIESCENCE_DEPTH = 4;
    int color;

    int LAZY_EVAL_MARGIN = 300;

private:
    int depth;
    int nodesSearched;
    int nCutoffs;
    int nKillerHits;
    int turn;
    clock_t timeSpentSorting;
    const libchess::Bitboard centralSquares = libchess::Bitboard(0x0000001818000000);
    const libchess::Bitboard extendedCenterSquares = libchess::Bitboard(0x00003c3c3c3c0000);
    // transposition table
    TranspositionTable* transpositionTable;
    PVTable* pvTable;

    // killer moves hashset
    std::unordered_set<std::pair<libchess::Move, int>, pair_hash> *killerMoves;
    std::unordered_map<libchess::Move, int, MoveHash> *historyMoves;

    OpeningBook *openingBook;
    PositionHasher *zobristHasher;

    int nullMoveSearch(libchess::Position &pos, int depth, int alpha, int beta, bool quiescent);
    bool futilityPrune(const libchess::Position &pos, libchess::Move move, int depth, int alpha, int beta, int posScore);
    void orderMoves(std::vector<libchess::Move> &moves, libchess::Position &pos, int depth, uint64_t hash);
    bool isTactical(libchess::Position &pos, libchess::Move move);
    int pieceValues[7];
    bool tryNullMove(libchess::Position &pos, int depth, int alpha, int beta, bool nullMoveAllowed, int &score, bool quiescent);
    bool moveIsCheck(libchess::Position &pos, libchess::Move move);
    void reinitKillerMoves();
    void reinitHistoryMoves();
    void updateHistory(libchess::Move move, int depth);
    bool checkStaleMateOrCheckmateFromMoves(const libchess::Position &pos, const std::vector<libchess::Move> &legalMoves, int &score);
    int quiescenceSearch(libchess::Position &pos, int alpha, int beta, bool nullMoveAllowed, int depth);
    bool tryMultiCutPruning(libchess::Position &pos, const std::vector<libchess::Move> &moves, int depth, int beta);
    int evaluateMaterialBalance(const libchess::Bitboard& whitePawns, const libchess::Bitboard& whiteKnights, const libchess::Bitboard& whiteBishops, const libchess::Bitboard& whiteRooks, const libchess::Bitboard& whiteQueens, const libchess::Bitboard& blackPawns, const libchess::Bitboard& blackKnights, const libchess::Bitboard& blackBishops, const libchess::Bitboard& blackRooks, const libchess::Bitboard& blackQueens);
};

#endif // SIMPLE_MMAB_HPP
