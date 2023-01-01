#include "simple_mmab.hpp"
#include "movegen.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <set>
#include <time.h>
#define DEBUG 0
#define PAWN_VALUE 100
#define KNIGHT_VALUE 300
#define BISHOP_VALUE 300
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 10000

#define PAWN_ADVANCE_A 10
#define PAWN_ADVANCE_B 20
#define PASSED_PAWN_MULT 10
#define DOUBLED_PAWN_PENALTY 30
#define ISOLATED_PAWN_PENALTY 10
#define BACKWARD_PAWN_PENALTY 5
#define WEAK_SQUARE_PENALTY 5
#define PASSED_PAWN_ENEMY_KING_DISTANCE 10
#define KNIGHT_SQ_MULT 6
#define BISHOP_MOBILITY 6
#define BISHOP_PAIR 28
#define ROOK_ATTACK_KING_FILE 51
#define ROOK_ATTACK_KING_ADJ_FILE 8
#define ROOK_ATTACK_KING_ADJ_FILE_ABGH 26
#define ROOK_7TH_RANK 30
#define ROOK_CONNECTED 6
#define ROOK_MOBILITY 4
#define ROOK_BEHIND_PASSED_PAWN 40
#define ROOK_OPEN_FILE 27
#define ROOK_SEMI_OPEN_FILE 11
#define ROOK_ATCK_WEAK_PAWN_OPEN_COLUMN 15
#define ROOK_COLUMN_MULT 6
#define QUEEN_MOBILITY 2
#define KING_FRIENDLY_PAWNS 35
#define KING_NO_FRIENDLY_PAWN_ADJ 10
#define KING_FRIENDLY_PAWN_ADVANCED 10
#define KING_NO_ENEMY_PAWN 17
#define KING_NO_ENEMY_PAWN_ADJ 5
#define KING_PRESSURE_MULT 4

#define NULL_MOVE_REDUCTION 3
#define USE_ADPT 1
#define ADPT_DEPTH 6
#define FUTILITY_DEPTH 100
#define FUTILITY_MARGIN_1 300
#define FUTILITY_MARGIN_2 500
#define FUTILITY_MARGIN_3 900

#define MULTICUT_REDUCTION 3
#define MULTICUT_DEPTH 6
#define MC_NUM 6
#define MC_CUT 2

ScarlettCore::ScarlettCore(int isWhite, int depth)
{
    this->color = color;
    this->depth = depth;

    this->transpositionTable = new std::map<uint64_t, std::pair<int, int>>();
    pieceValues[libchess::Piece::Pawn] = PAWN_VALUE;
    pieceValues[libchess::Piece::Knight] = KNIGHT_VALUE;
    pieceValues[libchess::Piece::Bishop] = BISHOP_VALUE;
    pieceValues[libchess::Piece::Rook] = ROOK_VALUE;
    pieceValues[libchess::Piece::Queen] = QUEEN_VALUE;
    pieceValues[libchess::Piece::King] = KING_VALUE;
}

ScarlettCore::~ScarlettCore()
{
    delete this->transpositionTable;
}

libchess::Move ScarlettCore::getMove(libchess::Position pos)
{

    this->nodesSearched = 0;
    this->nCutoffs = 0;
    this->timeSpentSorting = 0;
    std::vector<libchess::Move> moves = pos.legal_moves();

#if DEBUG
    std::cout << std::endl
              << "ALL MOVES: " << std::endl;
    for (libchess::Move move : moves)
    {
        std::cout << move << " ";
    }
    std::cout << std::endl;
#endif

    orderMoves(moves, pos);

#if DEBUG
    std::cout << std::endl
              << "ORDERED MOVES: " << std::endl;
    for (libchess::Move move : moves)
    {
        std::cout << move << " ";
    }
    std::cout << std::endl;
#endif
    libchess::Move bestMove;
    int bestScore;

    // time
    clock_t start;
    double duration;
    start = clock();

    bestScore = -10000000;

    for (libchess::Move move : moves)
    {
#if DEBUG
        std::cout << "TEST MOVE : " << move << std::endl;
#endif
        pos.makemove(move);
#if DEBUG
        std::cout << pos << std::endl;
#endif
        int score = -search(pos, depth - 1, -1000000, 1000000, true);
        pos.undomove();
        if (score > bestScore)
        {
            // std::cout << "Score: " << (2 * (1 - color) - 1) * score << std::endl;
            bestScore = score;
            bestMove = move;
        }
    }

    duration = (clock() - start) / (double)CLOCKS_PER_SEC;
    std::cout << std::endl;
    std::cout << "Time: " << duration << std::endl;
    // write best move in
    std::cout << "\033[1;31mBEST MOVE: "
              << bestMove << " " << bestScore / 100.0 << "\033[0m" << std::endl;
    // nodesSearched
    std::cout << "Nodes searched: " << nodesSearched << std::endl;
    std::cout << "Cutoffs: " << nCutoffs << std::endl;
    std::cout << "Time spent sorting: " << timeSpentSorting / (double)CLOCKS_PER_SEC << std::endl;
    return bestMove;
}

bool ScarlettCore::multicutPruning(libchess::Position pos, std::vector<libchess::Move> &moves, int depth, int beta, bool nullMoveAllowed)
{
    int count = 0;

    for (int i = 0; i < std::min((int)moves.size(), MC_NUM); i++)
    {
        libchess::Move move = moves[i];
        pos.makemove(move);
        int score = -search(pos, depth - MULTICUT_REDUCTION - 1, -beta, -beta + 1, nullMoveAllowed);
        pos.undomove();
        if (score >= beta)
        {
            count++;
            if (count >= MC_CUT)
            {
                return true;
            }
        }
    }
    return false;
}

int ScarlettCore::nullMoveSearch(libchess::Position pos, int depth, int alpha, int beta)
{
    pos.makenull();
    int reduct = (USE_ADPT && depth > ADPT_DEPTH) ? NULL_MOVE_REDUCTION : NULL_MOVE_REDUCTION - 1;
    int score;
    score = -search(pos, depth - reduct, -beta, -beta + 1, false);
    pos.undonull();
    return score;
}
int ScarlettCore::quiescenceSearch(libchess::Position pos, int alpha, int beta)
{
    int stand_pat = evaluate(pos);
    if (stand_pat >= beta)
        return beta;
    if (alpha < stand_pat)
        alpha = stand_pat;

    std::vector<libchess::Move> moves = pos.legal_captures();
    for (libchess::Move move : moves)
    {
        if (move.type() == libchess::MoveType::ksc || move.type() == libchess::MoveType::qsc)
            continue;
        pos.makemove(move);
        int score = -quiescenceSearch(pos, -beta, -alpha);
        pos.undomove();
        if (score >= beta)
        {
            nCutoffs++;
            return beta;
        }
        if (score > alpha)
            alpha = score;
    }
    return alpha;
}

bool ScarlettCore::isTactical(libchess::Position pos, libchess::Move move)
{
    bool isTactical = false;
    libchess::Piece pieceType = pos.piece_on(move.from());
    libchess::Piece capturePieceType = pos.piece_on(move.to());

    if (capturePieceType != libchess::Piece::None)
    {
        if (move.type() != libchess::MoveType::ksc && move.type() != libchess::MoveType::qsc)
        {
            isTactical = true;
        }
    }
    else if (pieceType == libchess::Piece::Pawn)
    {
        if (move.type() == libchess::MoveType::enpassant)
        {
            isTactical = true;
        }
    }
    else
    {
        pos.makemove(move);
        if (pos.in_check())
        {
            isTactical = true;
        }
        pos.undomove();
    }
    return isTactical;
}

void ScarlettCore::orderMoves(std::vector<libchess::Move> &moves, libchess::Position pos)
{
    int moveScores[moves.size()];
    libchess::Side opponent = (libchess::Side)(1 - (int)pos.turn());

    for (int i = 0; i < moves.size(); i++)
    {
        libchess::Move move = moves[i];
        // std::cout << "Move " << move;
        int moveScoreGuess = 0;
        libchess::Piece movePieceType = pos.piece_on(move.from());
        libchess::Piece capturePieceType = pos.piece_on(move.to());

        // if the move is a capture, add the value of the captured piece to the move score guess
        if (capturePieceType != libchess::Piece::None && move.type() != libchess::MoveType::ksc && move.type() != libchess::MoveType::qsc)
        {
            // std::cout << " capture " << (int)capturePieceType;
            moveScoreGuess += 10 * pieceValues[(int)capturePieceType] - pieceValues[(int)movePieceType];
        }
        if (movePieceType == libchess::Piece::Pawn)
        {
            if (move.type() == libchess::MoveType::promo || move.type() == libchess::MoveType::promo_capture)
            {
                // std::cout << " promotion " << move.promotion();
                moveScoreGuess += pieceValues[(int)move.promotion()];
            }
        }
        else
        {

            if (!(pos.attackers(move.to(), opponent) & pos.pieces(opponent, libchess::Piece::Pawn)).empty())
            {
                moveScoreGuess -= pieceValues[(int)movePieceType];
            }
        }
        // std::cout << " score " << moveScoreGuess << " pointer " << std::endl;

        moveScores[i] = moveScoreGuess;
    }

    // sort moves
    clock_t start = clock();
    for (int i = 0; i < moves.size(); i++)
    {
        for (int j = i + 1; j < moves.size(); j++)
        {
            if (moveScores[j] > moveScores[i])
            {
                int temp = moveScores[i];
                moveScores[i] = moveScores[j];
                moveScores[j] = temp;
                libchess::Move tempMove = moves[i];
                moves[i] = moves[j];
                moves[j] = tempMove;
            }
        }
    }

    timeSpentSorting += (clock() - start);
}
bool ScarlettCore::futilityPrune(libchess::Position pos, libchess::Move move, int depth, int alpha, int posScore)
{
    if (!isTactical(pos, move))
    {
        if (depth == 1)
        {
            if (posScore + FUTILITY_MARGIN_1 <= alpha)
            {
                nCutoffs++;
                return true;
            }
        }
        if (depth == 2)
        {
            if (posScore + FUTILITY_MARGIN_2 <= alpha)
            {
                nCutoffs++;
                return true;
            }
        }
        if (depth == 3)
        {
            if (posScore + FUTILITY_MARGIN_3 <= alpha)
            {
                nCutoffs++;
                return true;
            }
        }
        return false;
    }
    return false;
}
int ScarlettCore::search(libchess::Position pos, int depth, int alpha, int beta, bool nullMoveAllowed)
{
    if (pos.is_draw())
    {

        return 0;
    }
    // if transposition table contains the current position and the depth is greater than or equal to the depth of the position in the table, return the score of the position in the table
    if (transpositionTable->find(pos.hash()) != transpositionTable->end())
    {
        std::pair<int, int> scoreAndDepth = transpositionTable->at(pos.hash());
        if (scoreAndDepth.second >= depth)
        {
            // std::cout << "Transposition table hit" << std::endl;
            nCutoffs++;
            return scoreAndDepth.first;
        }
    }
    nodesSearched++;
    if (depth <= 0)
    {
        int score = evaluate(pos);
        // transpositionTable->insert(std::pair<uint32_t, std::pair<int, int>>(pos.hash(), std::pair<int, int>(score, depth)));
        return score;
    }

    if (nullMoveAllowed && !pos.in_check())
    {
        int score = nullMoveSearch(pos, depth, alpha, beta);
        if (score >= beta)
        {
            return score;
        }
    }

    nullMoveAllowed = true;

    std::vector<libchess::Move> moves = pos.legal_moves();
    if (moves.size() == 0)
    {
        if (pos.in_check())
        {
            return -100000;
        }
        else
        {
            return 0;
        }
    }

#if DEBUG
    std::cout << "ALL MOVES: ";
    for (libchess::Move move : moves)
    {
        std::cout << move << " ";
    }
    std::cout << std::endl;
#endif
    orderMoves(moves, pos);

    if (depth >= MC_CUT)
    {
        if (multicutPruning(pos, moves, depth, beta, nullMoveAllowed))
        {
            nCutoffs++;
            return beta;
        }
    }
//  kill process
// print moves
#if DEBUG
    std::cout << "ORDERERD MOVES: ";
    for (libchess::Move move : moves)
    {
        std::cout << move << " ";
    }
    std::cout << std::endl;
#endif
    int score;
    if (depth <= FUTILITY_DEPTH)
    {
        score = evaluate(pos);
    }

    for (libchess::Move move : moves)
    {
        if (depth <= FUTILITY_DEPTH && futilityPrune(pos, move, depth, alpha, score))
        {
            continue;
        }

        pos.makemove(move);
        int score = -search(pos, depth - 1, -beta, -alpha, nullMoveAllowed);
        pos.undomove();
        if (score >= beta)
        {
            nCutoffs++;
            return score;
        }
        if (score > alpha)
        {
            alpha = score;
        }
    }
    transpositionTable->insert(std::pair<uint64_t, std::pair<int, int>>(pos.hash(), std::pair<int, int>(alpha, depth)));
    return alpha;
}
int ScarlettCore::materialScore(libchess::Position pos)
{
    int score = 0;
    score += pos.pieces(libchess::Side::White, libchess::Piece::Pawn).count() * PAWN_VALUE;
    score -= pos.pieces(libchess::Side::Black, libchess::Piece::Pawn).count() * PAWN_VALUE;

    score += pos.pieces(libchess::Side::White, libchess::Piece::Knight).count() * KNIGHT_VALUE;
    score -= pos.pieces(libchess::Side::Black, libchess::Piece::Knight).count() * KNIGHT_VALUE;

    score += pos.pieces(libchess::Side::White, libchess::Piece::Bishop).count() * BISHOP_VALUE;
    score -= pos.pieces(libchess::Side::Black, libchess::Piece::Bishop).count() * BISHOP_VALUE;

    score += pos.pieces(libchess::Side::White, libchess::Piece::Rook).count() * ROOK_VALUE;
    score -= pos.pieces(libchess::Side::Black, libchess::Piece::Rook).count() * ROOK_VALUE;

    score += pos.pieces(libchess::Side::White, libchess::Piece::Queen).count() * QUEEN_VALUE;
    score -= pos.pieces(libchess::Side::Black, libchess::Piece::Queen).count() * QUEEN_VALUE;

    return score;
}
int ScarlettCore::evaluate(libchess::Position pos)
{
    int score = 0;
    std::vector<libchess::Move> moves;

    if (pos.is_checkmate())
    {
        return -1000000;
    }
    if (pos.is_stalemate())
    {
        return 0;
    }
    if (pos.is_draw())
    {
        return 0;
    }

    score += materialScore(pos);

    score += pos.passed_pawns(libchess::Side::White).count() * PASSED_PAWN_MULT;
    score -= pos.passed_pawns(libchess::Side::Black).count() * PASSED_PAWN_MULT;

    // white semi open files
    int whiteSemiOpenFiles = 0; // set of white semi open files
    libchess::Bitboard whiteSemiOpenFilesBitboard;

    if ((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileA).empty())
    {
        whiteSemiOpenFiles++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileA;
    }
    if ((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileB).empty())
    {
        whiteSemiOpenFiles++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileB;
    }
    if ((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileC).empty())
    {
        whiteSemiOpenFiles++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileC;
    }
    if ((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileD).empty())
    {
        whiteSemiOpenFiles++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileD;
    }
    if ((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileE).empty())
    {
        whiteSemiOpenFiles++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileE;
    }
    if ((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileF).empty())
    {
        whiteSemiOpenFiles++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileF;
    }
    if ((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileG).empty())
    {
        whiteSemiOpenFiles++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileG;
    }
    if ((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileH).empty())
    {
        whiteSemiOpenFiles++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileH;
    }

    // set of black semi open files
    int blackSemiOpenFiles = 0;
    libchess::Bitboard blackSemiOpenFilesBitboard;

    if ((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileA).empty())
    {
        blackSemiOpenFiles++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileA;
    }
    if ((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileB).empty())
    {
        blackSemiOpenFiles++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileB;
    }
    if ((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileC).empty())
    {
        blackSemiOpenFiles++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileC;
    }
    if ((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileD).empty())
    {
        blackSemiOpenFiles++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileD;
    }
    if ((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileE).empty())
    {
        blackSemiOpenFiles++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileE;
    }
    if ((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileF).empty())
    {
        blackSemiOpenFiles++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileF;
    }
    if ((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileG).empty())
    {
        blackSemiOpenFiles++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileG;
    }
    if ((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileH).empty())
    {
        blackSemiOpenFiles++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileH;
    }

    // open files

    libchess::Bitboard openFilesBitboard = whiteSemiOpenFilesBitboard & blackSemiOpenFilesBitboard;
    // doubled paws = nbpawns - 8 + nbsemiopenfiles

    score -= (pos.pieces(libchess::Side::White, libchess::Piece::Pawn).count() - 8 + whiteSemiOpenFiles) * DOUBLED_PAWN_PENALTY;
    score += (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn).count() - 8 + blackSemiOpenFiles) * DOUBLED_PAWN_PENALTY;

    // isolated pawns

    score -= (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & whiteSemiOpenFilesBitboard.west() & whiteSemiOpenFilesBitboard.east()).count() * ISOLATED_PAWN_PENALTY;
    score += (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & blackSemiOpenFilesBitboard.west() & blackSemiOpenFilesBitboard.east()).count() * ISOLATED_PAWN_PENALTY;

    // bishop pair
    if (pos.pieces(libchess::Side::White, libchess::Piece::Bishop).count() >= 2)
    {
        score += BISHOP_PAIR;
    }
    if (pos.pieces(libchess::Side::Black, libchess::Piece::Bishop).count() >= 2)
    {
        score -= BISHOP_PAIR;
    }

    // rook on open file
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & openFilesBitboard).count() * ROOK_OPEN_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & openFilesBitboard).count() * ROOK_OPEN_FILE;

    // rook on semi open file
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & whiteSemiOpenFilesBitboard).count() * ROOK_SEMI_OPEN_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & blackSemiOpenFilesBitboard).count() * ROOK_SEMI_OPEN_FILE;

    // king file
    libchess::Square whiteKingSquare = pos.king_position(libchess::Side::White);
    libchess::Square blackKingSquare = pos.king_position(libchess::Side::Black);

    libchess::Bitboard whiteKingFile = libchess::bitboards::files[whiteKingSquare.file()];
    libchess::Bitboard blackKingFile = libchess::bitboards::files[blackKingSquare.file()];

    // rook on king file
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & blackKingFile).count() * ROOK_ATTACK_KING_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & whiteKingFile).count() * ROOK_ATTACK_KING_FILE;

    // rook attack king adjacent file
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & blackKingFile.west()).count() * ROOK_ATTACK_KING_ADJ_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & whiteKingFile.west()).count() * ROOK_ATTACK_KING_ADJ_FILE;

    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & blackKingFile.east()).count() * ROOK_ATTACK_KING_ADJ_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & whiteKingFile.east()).count() * ROOK_ATTACK_KING_ADJ_FILE;

    // rook on 7th rank
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & libchess::bitboards::Rank7).count() * ROOK_7TH_RANK;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & libchess::bitboards::Rank2).count() * ROOK_7TH_RANK;

    // king protection
    libchess::Bitboard whiteKingMask = libchess::movegen::king_moves(whiteKingSquare);
    libchess::Bitboard blackKingMask = libchess::movegen::king_moves(blackKingSquare);

    score += (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & whiteKingMask).count() * KING_NO_FRIENDLY_PAWN_ADJ;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & blackKingMask).count() * KING_NO_FRIENDLY_PAWN_ADJ;

    for (const auto &fr : pos.pieces(libchess::Side::White, libchess::Piece::Bishop))
    {
        libchess::Bitboard bishopMask = libchess::movegen::bishop_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::Black);
        score += bishopMask.count() * BISHOP_MOBILITY;
    }
    for (const auto &fr : pos.pieces(libchess::Side::Black, libchess::Piece::Bishop))
    {
        libchess::Bitboard bishopMask = libchess::movegen::bishop_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::White);
        score -= bishopMask.count() * BISHOP_MOBILITY;
    }

    for (const auto &fr : pos.pieces(libchess::Side::White, libchess::Piece::Queen))
    {
        libchess::Bitboard queenMask = libchess::movegen::queen_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::Black);
        score += queenMask.count() * QUEEN_MOBILITY;
    }
    for (const auto &fr : pos.pieces(libchess::Side::Black, libchess::Piece::Queen))
    {
        libchess::Bitboard queenMask = libchess::movegen::queen_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::White);
        score -= queenMask.count() * QUEEN_MOBILITY;
    }

    for (const auto &fr : pos.pieces(libchess::Side::White, libchess::Piece::Rook))
    {
        libchess::Bitboard rookMask = libchess::movegen::rook_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::Black);
        score += rookMask.count() * ROOK_MOBILITY;
    }
    for (const auto &fr : pos.pieces(libchess::Side::Black, libchess::Piece::Rook))
    {
        libchess::Bitboard rookMask = libchess::movegen::rook_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::White);
        score -= rookMask.count() * ROOK_MOBILITY;
    }

    for (const auto &fr : pos.pieces(libchess::Side::White, libchess::Piece::Knight))
    {
        libchess::Bitboard knightMask = libchess::movegen::knight_moves(fr) & ~pos.squares_attacked(libchess::Side::Black);
        score += knightMask.count() * KNIGHT_SQ_MULT;
    }

    for (const auto &fr : pos.pieces(libchess::Side::Black, libchess::Piece::Knight))
    {
        libchess::Bitboard knightMask = libchess::movegen::knight_moves(fr) & ~pos.squares_attacked(libchess::Side::White);
        score -= knightMask.count() * KNIGHT_SQ_MULT;
    }

    // central pawns
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::Bitboard(0x0000001818000000)).count() * 20;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::Bitboard(0x0000001818000000)).count() * 20;

    // extended center pawns
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::Bitboard(0x00003c3c3c3c0000)).count() * 10;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::Bitboard(0x00003c3c3c3c0000)).count() * 10;
    // king safety
    if (pos.king_position(libchess::Side::White) == libchess::squares::A1 || pos.king_position(libchess::Side::White) == libchess::squares::B1 || pos.king_position(libchess::Side::White) == libchess::squares::C1 || pos.king_position(libchess::Side::White) == libchess::squares::G1 || pos.king_position(libchess::Side::White) == libchess::squares::H1)
    {
        score += 100;
    }
    if (pos.king_position(libchess::Side::Black) == libchess::squares::A8 || pos.king_position(libchess::Side::Black) == libchess::squares::B8 || pos.king_position(libchess::Side::Black) == libchess::squares::C8 || pos.king_position(libchess::Side::Black) == libchess::squares::G8 || pos.king_position(libchess::Side::Black) == libchess::squares::H8)
    {
        score -= 100;
    }

    score += KING_FRIENDLY_PAWNS * (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::movegen::king_moves(pos.king_position(libchess::Side::White))).count();
    score -= KING_FRIENDLY_PAWNS * (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::movegen::king_moves(pos.king_position(libchess::Side::Black))).count();

    // castling rights
    if (pos.can_castle(libchess::Side::White, libchess::MoveType::ksc))
    {
        score += 70;
    }
    if (pos.can_castle(libchess::Side::White, libchess::MoveType::qsc))
    {
        score += 60;
    }
    if (pos.can_castle(libchess::Side::Black, libchess::MoveType::ksc))
    {
        score -= 70;
    }
    if (pos.can_castle(libchess::Side::Black, libchess::MoveType::qsc))
    {
        score -= 60;
    }

    return score * (pos.turn() == libchess::Side::White ? 1 : -1);
}