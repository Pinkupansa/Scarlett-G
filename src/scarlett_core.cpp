#include "scarlett_core.hpp"
#include "movegen.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <set>
#include <time.h>
#define DEBUG 0
#define USAGE_MODE 1
#define CHECKMATE_CONSTANT 1000000
#define DRAW_OR_STALEMATE_CONSTANT 0
ScarlettCore::ScarlettCore(int color, int depth)
{
    this->color = color;
    this->depth = depth;

    this->transpositionTable = new std::map<uint64_t, std::pair<int, int>>();

    pieceValues[libchess::Piece::Pawn] = PAWN_VALUE;
    pieceValues[libchess::Piece::Knight] = this->KNIGHT_VALUE;
    pieceValues[libchess::Piece::Bishop] = this->BISHOP_VALUE;
    pieceValues[libchess::Piece::Rook] = this->ROOK_VALUE;
    pieceValues[libchess::Piece::Queen] = this->QUEEN_VALUE;
    pieceValues[libchess::Piece::King] = KING_VALUE;

    this->PAWN_VALUE = 100;
    this->KNIGHT_VALUE = 521;
    this->BISHOP_VALUE = 572;
    this->ROOK_VALUE = 824;
    this->QUEEN_VALUE = 1710;

    // this->PAWN_ADVANCE_A = 10;
    // this->PAWN_ADVANCE_B = 20;
    this->PASSED_PAWN_MULT = 10;
    this->DOUBLED_PAWN_PENALTY = 14;
    this->ISOLATED_PAWN_PENALTY = 8;
    // this->BACKWARD_PAWN_PENALTY = 5;
    // this->WEAK_SQUARE_PENALTY = 5;
    // this->PASSED_PAWN_ENEMY_KING_DISTANCE = 10;
    this->KNIGHT_MOBILITY = 6;
    this->BISHOP_MOBILITY = 4;
    this->BISHOP_PAIR = 28;
    this->ROOK_ATTACK_KING_FILE = 51;
    this->ROOK_ATTACK_KING_ADJ_FILE = 8;
    // this->ROOK_ATTACK_KING_ADJ_FILE_ABGH = 26;
    this->ROOK_7TH_RANK = 30;
    this->ROOK_CONNECTED = 6;
    this->ROOK_MOBILITY = 4;
    // this->ROOK_BEHIND_PASSED_PAWN = 10;
    this->ROOK_OPEN_FILE = 27;
    this->ROOK_SEMI_OPEN_FILE = 11;
    this->QUEEN_MOBILITY = 2;
    this->KING_FRIENDLY_PAWNS = 35;
    // this->KING_NO_FRIENDLY_PAWN_ADJ = 10;
    // this->KING_PRESSURE_MULT = 5;

    this->NULL_MOVE_REDUCTION = 3;
    this->USE_ADPT = 1;
    this->ADPT_DEPTH = 6;
    this->FUTILITY_DEPTH = 3;
    this->FUTILITY_MARGIN_1 = 500;
    this->FUTILITY_MARGIN_2 = 650;
    this->FUTILITY_MARGIN_3 = 1200;

    this->MULTICUT_REDUCTION = 3;
    this->MULTICUT_DEPTH = 6;
    this->MC_NUM = 10;
    this->MC_CUT = 3;

    killerMoves = new std::unordered_set<std::pair<libchess::Move, int>, pair_hash>();
}

void ScarlettCore::printWeights()
{
    std::cout << "PAWN_VALUE: " << this->PAWN_VALUE << std::endl;
    std::cout << "KNIGHT_VALUE: " << this->KNIGHT_VALUE << std::endl;
    std::cout << "BISHOP_VALUE: " << this->BISHOP_VALUE << std::endl;
    std::cout << "ROOK_VALUE: " << this->ROOK_VALUE << std::endl;
    std::cout << "QUEEN_VALUE: " << this->QUEEN_VALUE << std::endl;
    std::cout << "KING_VALUE: " << this->KING_VALUE << std::endl;
    std::cout << "PASSED_PAWN_MULT: " << this->PASSED_PAWN_MULT << std::endl;
    std::cout << "DOUBLED_PAWN_PENALTY: " << this->DOUBLED_PAWN_PENALTY << std::endl;
    std::cout << "ISOLATED_PAWN_PENALTY: " << this->ISOLATED_PAWN_PENALTY << std::endl;
    std::cout << "KNIGHT_MOBILITY: " << this->KNIGHT_MOBILITY << std::endl;
    std::cout << "BISHOP_MOBILITY: " << this->BISHOP_MOBILITY << std::endl;
    std::cout << "BISHOP_PAIR: " << this->BISHOP_PAIR << std::endl;
    std::cout << "ROOK_ATTACK_KING_FILE: " << this->ROOK_ATTACK_KING_FILE << std::endl;
    std::cout << "ROOK_ATTACK_KING_ADJ_FILE: " << this->ROOK_ATTACK_KING_ADJ_FILE << std::endl;
    std::cout << "ROOK_7TH_RANK: " << this->ROOK_7TH_RANK << std::endl;
    std::cout << "ROOK_CONNECTED: " << this->ROOK_CONNECTED << std::endl;
    std::cout << "ROOK_MOBILITY: " << this->ROOK_MOBILITY << std::endl;
    std::cout << "ROOK_OPEN_FILE: " << this->ROOK_OPEN_FILE << std::endl;
    std::cout << "ROOK_SEMI_OPEN_FILE: " << this->ROOK_SEMI_OPEN_FILE << std::endl;
    std::cout << "QUEEN_MOBILITY: " << this->QUEEN_MOBILITY << std::endl;
    std::cout << "KING_FRIENDLY_PAWNS: " << this->KING_FRIENDLY_PAWNS << std::endl;
}

void ScarlettCore::setPieceValues(int weights[6])
{
    this->KNIGHT_VALUE = weights[0];
    this->BISHOP_VALUE = weights[1];
    this->ROOK_VALUE = weights[2];
    this->QUEEN_VALUE = weights[3];
    this->KING_VALUE = weights[4];

    // pieceValues[libchess::Piece::Pawn] = PAWN_VALUE;
    pieceValues[libchess::Piece::Knight] = this->KNIGHT_VALUE;
    pieceValues[libchess::Piece::Bishop] = this->BISHOP_VALUE;
    pieceValues[libchess::Piece::Rook] = this->ROOK_VALUE;
    pieceValues[libchess::Piece::Queen] = this->QUEEN_VALUE;
    pieceValues[libchess::Piece::King] = this->KING_VALUE;
}

void ScarlettCore::setWeights(int weights[20])
{

    this->KNIGHT_VALUE = weights[0];
    this->BISHOP_VALUE = weights[1];
    this->ROOK_VALUE = weights[2];
    this->QUEEN_VALUE = weights[3];
    this->KING_VALUE = weights[4];

    this->PASSED_PAWN_MULT = weights[5];
    this->DOUBLED_PAWN_PENALTY = weights[6];
    this->ISOLATED_PAWN_PENALTY = weights[7];

    this->KNIGHT_MOBILITY = weights[8];
    this->BISHOP_MOBILITY = weights[9];
    this->BISHOP_PAIR = weights[10];
    this->ROOK_ATTACK_KING_FILE = weights[11];
    this->ROOK_ATTACK_KING_ADJ_FILE = weights[12];
    this->ROOK_7TH_RANK = weights[13];
    this->ROOK_CONNECTED = weights[14];
    this->ROOK_MOBILITY = weights[15];
    this->ROOK_OPEN_FILE = weights[16];
    this->ROOK_SEMI_OPEN_FILE = weights[17];
    this->QUEEN_MOBILITY = weights[18];
    this->KING_FRIENDLY_PAWNS = weights[19];

    pieceValues[libchess::Piece::Pawn] = this->PAWN_VALUE;
    pieceValues[libchess::Piece::Knight] = this->KNIGHT_VALUE;
    pieceValues[libchess::Piece::Bishop] = this->BISHOP_VALUE;
    pieceValues[libchess::Piece::Rook] = this->ROOK_VALUE;
    pieceValues[libchess::Piece::Queen] = this->QUEEN_VALUE;
    pieceValues[libchess::Piece::King] = this->KING_VALUE;

    this->FUTILITY_DEPTH = 0;
    this->MULTICUT_DEPTH = 6;
    this->MULTICUT_REDUCTION = 0;
}

ScarlettCore::~ScarlettCore()
{
    delete this->transpositionTable;
    delete this->killerMoves;
}

libchess::Move ScarlettCore::getMove(libchess::Position pos)
{

    this->nodesSearched = 0;
    this->nCutoffs = 0;
    this->timeSpentSorting = 0;
    this->nKillerHits = 0;

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

    orderMoves(moves, pos, depth);

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

    bestScore = -CHECKMATE_CONSTANT;

    for (libchess::Move move : moves)
    {
#if DEBUG
        std::cout << "TEST MOVE : " << move << std::endl;
#endif
        pos.makemove(move);
#if DEBUG
        std::cout << pos << std::endl;
#endif
        int score = -search(pos, depth - 1, -CHECKMATE_CONSTANT, CHECKMATE_CONSTANT, true);
        pos.undomove();
        if (score > bestScore)
        {
            // std::cout << "Score: " << (2 * (1 - color) - 1) * score << std::endl;
            bestScore = score;
            bestMove = move;
        }
    }
#if USAGE_MODE
    duration = (clock() - start) / (double)CLOCKS_PER_SEC;
    std::cout << std::endl;
    std::cout << "Time: " << duration << std::endl;
    // write best move in
    std::cout << "\033[1;31mBEST MOVE: "
              << bestMove << " " << color * bestScore / 100.0 << "\033[0m" << std::endl;
    // nodesSearched
    std::cout << "Nodes searched: " << nodesSearched << std::endl;
    std::cout << "Cutoffs: " << nCutoffs << std::endl;
    // std::cout << "Time spent sorting: " << timeSpentSorting / (double)CLOCKS_PER_SEC << std::endl;
    std::cout << "Killer hits: " << nKillerHits << std::endl;
    std::cout << "Number of killer moves: " << killerMoves->size() << std::endl;
    std::cout << pos << std::endl;
    std::cout << pos.get_fen() << std::endl;
#endif

    reinitKillerMoves();
    return bestMove;
}

bool ScarlettCore::multicutPruning(libchess::Position &pos, std::vector<libchess::Move> &moves, int depth, int beta, bool nullMoveAllowed)
{
    if (depth >= MULTICUT_DEPTH)
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
    }
    return false;
}

int ScarlettCore::nullMoveSearch(libchess::Position &pos, int depth, int alpha, int beta)
{
    pos.makenull();
    int reduct = (USE_ADPT && depth > ADPT_DEPTH) ? NULL_MOVE_REDUCTION : NULL_MOVE_REDUCTION - 1;
    int score;
    score = -search(pos, depth - reduct, -beta, -beta + 1, false);
    pos.undonull();
    return score;
}
int ScarlettCore::quiescenceSearch(libchess::Position &pos, int alpha, int beta)
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

bool ScarlettCore::moveIsCheck(libchess::Position &pos, libchess::Move &move)
{
    bool is_check = false;
    pos.makemove(move);
    if (pos.in_check())
    {
        is_check = true;
    }
    pos.undomove();
    return is_check;
}
bool ScarlettCore::isTactical(libchess::Position &pos, libchess::Move &move)
{
    libchess::Piece pieceType = pos.piece_on(move.from());
    if (move.is_capturing() || move.is_promoting())
    {
        return true;
    }
    else if (moveIsCheck(pos, move))
    {
        return true;
    }
    return false;
}

void ScarlettCore::orderMoves(std::vector<libchess::Move> &moves, libchess::Position &pos, int depth)
{
    std::vector<int> moveScores(moves.size());
    libchess::Side opponent = (libchess::Side)(1 - (int)pos.turn());

    for (int i = 0; i < moves.size(); i++)
    {
        libchess::Move move = moves[i];
        int moveScoreGuess = 0;
        libchess::Piece movePieceType = pos.piece_on(move.from());
        libchess::Piece capturePieceType = pos.piece_on(move.to());

        // if killer move, add 100000 to the move score guess
        if (killerMoves->find(std::pair<libchess::Move, int>(move, depth)) != killerMoves->end())
        {
            moveScoreGuess += 100000;
            nKillerHits++;
        }
        if (moveIsCheck(pos, move))
        {
            moveScoreGuess += 10000;
        }
        // if the move is a capture, add the value of the captured piece to the move score guess
        if (move.is_capturing())
        {
            moveScoreGuess += 10 * pieceValues[(int)capturePieceType] - pieceValues[(int)movePieceType];
        }

        // if the move is a promotion, add the value of the promoted piece to the move score guess
        if (movePieceType == libchess::Piece::Pawn)
        {
            if (move.is_promoting())
            {
                moveScoreGuess += pieceValues[(int)move.promotion()];
            }
        }
        else
        {
            // if the target square is attacked by an enemy pawn, subtract the value of the moving piece from the move score guess
            if (!((pos.attackers(move.to(), opponent) & pos.pieces(opponent, libchess::Piece::Pawn)).empty()))
            {
                moveScoreGuess -= pieceValues[(int)movePieceType];
            }
        }
        moveScores[i] = moveScoreGuess;
    }

    // sort moves

    std::vector<std::pair<int, libchess::Move>> paired(moves.size());
    for (size_t i = 0; i < moves.size(); ++i)
    {
        paired[i] = std::make_pair(moveScores[i], moves[i]);
    }

    std::sort(paired.begin(), paired.end(), [](const auto &a, const auto &b)
              {
                  return a.first > b.first; // sort in descending order
              });

    // Now, moveScores and moves are sorted in descending order of moveScores
    for (size_t i = 0; i < moves.size(); ++i)
    {
        moveScores[i] = paired[i].first;
        moves[i] = paired[i].second;
    }
}
bool ScarlettCore::futilityPrune(libchess::Position &pos, libchess::Move &move, int depth, int alpha, int posScore)
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
bool ScarlettCore::checkTranspositionTable(libchess::Position &pos, int depth, int &score)
{
    if (transpositionTable->find(pos.hash()) != transpositionTable->end())
    {
        std::pair<int, int> scoreAndDepth = transpositionTable->at(pos.hash());
        if (scoreAndDepth.second >= depth)
        {
            nCutoffs++;
            score = scoreAndDepth.first;
            return true;
        }
    }
    return false;
}
bool ScarlettCore::tryNullMove(libchess::Position &pos, int depth, int alpha, int beta, bool nullMoveAllowed, int &score)
{
    // color to move
    libchess::Side side = pos.turn();

    if (nullMoveAllowed && !pos.in_check())
    {
        // count number of pieces
        int count = 0;
        for (const auto &piece_type : {libchess::Piece::Pawn, libchess::Piece::Knight, libchess::Piece::Bishop, libchess::Piece::Rook, libchess::Piece::Queen, libchess::Piece::King})
        {
            count += pos.pieces(side, piece_type).count();
        }
        if (count < 10)
        {
            return false;
        }
        int tryScore = nullMoveSearch(pos, depth, alpha, beta);
        if (tryScore >= beta)
        {
            score = tryScore;
            return true;
        }
    }
    return false;
}

bool ScarlettCore::checkStaleMateOrCheckmateFromMoves(libchess::Position &pos, std::vector<libchess::Move> &legalMoves, int &score)
{
    if (legalMoves.size() == 0)
    {
        if (pos.in_check())
        {
            score = -CHECKMATE_CONSTANT;
        }
        else
        {
            score = DRAW_OR_STALEMATE_CONSTANT;
        }
        return true;
    }
    return false;
}
int ScarlettCore::search(libchess::Position &pos, int depth, int alpha, int beta, bool nullMoveAllowed)
{
    /*Alpha is the best score that the current player (treated as a maximizing player in negamax framework) can achieve so far.
    Beta is the best score that the opponent (treated as a minimizing player in negamax framework) can achieve so far.
    Can achieve means that the opponent can force a score of beta or higher, and the current player can force a score of alpha or higher.*/

    nodesSearched++;

    if (pos.is_draw())
    {
        return DRAW_OR_STALEMATE_CONSTANT;
    }

    int transpositionScore; // TODO : look if draw
    if (checkTranspositionTable(pos, depth, transpositionScore))
    {
        return transpositionScore;
    }

    // If search ended, evaluate position
    if (depth <= 0)
    {
        int score = evaluate(pos);
        return score;
    }

    int nullMoveScore = 0;
    if (tryNullMove(pos, depth, alpha, beta, nullMoveAllowed, nullMoveScore))
    {
        return nullMoveScore;
    }

    std::vector<libchess::Move> moves = pos.legal_moves();
    int checkmateOrStalemateScore;
    if (checkStaleMateOrCheckmateFromMoves(pos, moves, checkmateOrStalemateScore))
    {
        return checkmateOrStalemateScore;
    }

#if DEBUG
    std::cout << "ALL MOVES: ";
    for (libchess::Move move : moves)
    {
        std::cout << move << " ";
    }
    std::cout << std::endl;
#endif

    orderMoves(moves, pos, depth);

    if (multicutPruning(pos, moves, depth, beta, true))
    {
        nCutoffs++;
        return beta;
    }

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
        int score = -search(pos, depth - 1, -beta, -alpha, true);
        pos.undomove();
        if (score >= beta) // The enemy can force a score of beta or higher so it won't go down this path
        {
            nCutoffs++;
            killerMoves->insert(std::pair<libchess::Move, int>(move, depth));
            return score;
        }
        if (score > alpha) // Found a new reachable best score
        {
            alpha = score;
        }
    }

    transpositionTable->insert(std::pair<uint64_t, std::pair<int, int>>(pos.hash(), std::pair<int, int>(alpha, depth)));
    return alpha;
}
int ScarlettCore::evaluate(libchess::Position &pos)
{
    int score = 0;
    std::vector<libchess::Move> moves;
    moves = pos.legal_moves();

    int checkmateOrStalemateScore;
    if (checkStaleMateOrCheckmateFromMoves(pos, moves, checkmateOrStalemateScore))
    {
        return checkmateOrStalemateScore;
    }
    if (pos.is_draw())
    {
        return DRAW_OR_STALEMATE_CONSTANT;
    }

    score += pos.passed_pawns(libchess::Side::White).count() * PASSED_PAWN_MULT;
    score -= pos.passed_pawns(libchess::Side::Black).count() * PASSED_PAWN_MULT;

    libchess::Bitboard whitePawns = pos.pieces(libchess::Side::White, libchess::Piece::Pawn);
    libchess::Bitboard blackPawns = pos.pieces(libchess::Side::Black, libchess::Piece::Pawn);
    ;

    libchess::Bitboard whiteKnights = pos.pieces(libchess::Side::White, libchess::Piece::Knight);
    libchess::Bitboard blackKnights = pos.pieces(libchess::Side::Black, libchess::Piece::Knight);

    libchess::Bitboard whiteBishops = pos.pieces(libchess::Side::White, libchess::Piece::Bishop);
    libchess::Bitboard blackBishops = pos.pieces(libchess::Side::Black, libchess::Piece::Bishop);

    libchess::Bitboard whiteRooks = pos.pieces(libchess::Side::White, libchess::Piece::Rook);
    libchess::Bitboard blackRooks = pos.pieces(libchess::Side::Black, libchess::Piece::Rook);

    libchess::Bitboard whiteQueens = pos.pieces(libchess::Side::White, libchess::Piece::Queen);
    libchess::Bitboard blackQueens = pos.pieces(libchess::Side::Black, libchess::Piece::Queen);

    score += whitePawns.count() * PAWN_VALUE;
    score -= blackPawns.count() * PAWN_VALUE;

    score += whiteKnights.count() * KNIGHT_VALUE;
    score -= blackKnights.count() * KNIGHT_VALUE;

    score += whiteBishops.count() * BISHOP_VALUE;
    score -= blackBishops.count() * BISHOP_VALUE;

    score += whiteRooks.count() * ROOK_VALUE;
    score -= blackRooks.count() * ROOK_VALUE;

    score += whiteQueens.count() * QUEEN_VALUE;
    score -= blackQueens.count() * QUEEN_VALUE;

    int whiteSemiOpenFiles = 0;
    libchess::Bitboard whiteSemiOpenFilesBitboard;
    for (const auto &file : libchess::bitboards::files)
    {
        if ((whitePawns & file).empty())
        {
            whiteSemiOpenFiles++;
            whiteSemiOpenFilesBitboard |= file;
        }
    }

    // set of black semi open files
    int blackSemiOpenFiles = 0;
    libchess::Bitboard blackSemiOpenFilesBitboard;

    for (const auto &file : libchess::bitboards::files)
    {
        if ((blackPawns & file).empty())
        {
            blackSemiOpenFiles++;
            blackSemiOpenFilesBitboard |= file;
        }
    }

    // open files
    libchess::Bitboard openFilesBitboard = whiteSemiOpenFilesBitboard & blackSemiOpenFilesBitboard;
    // doubled paws = nbpawns - 8 + nbsemiopenfiles

    score -= (whitePawns.count() - 8 + whiteSemiOpenFiles) * DOUBLED_PAWN_PENALTY;
    score += (blackPawns.count() - 8 + blackSemiOpenFiles) * DOUBLED_PAWN_PENALTY;

    // isolated pawns

    score -= (whitePawns & whiteSemiOpenFilesBitboard.west() & whiteSemiOpenFilesBitboard.east()).count() * ISOLATED_PAWN_PENALTY;
    score += (blackPawns & blackSemiOpenFilesBitboard.west() & blackSemiOpenFilesBitboard.east()).count() * ISOLATED_PAWN_PENALTY;

    // bishop pair
    if (whiteBishops.count() >= 2)
    {
        score += BISHOP_PAIR;
    }
    if (blackBishops.count() >= 2)
    {
        score -= BISHOP_PAIR;
    }

    // rook on open file
    score += (whiteRooks & openFilesBitboard).count() * ROOK_OPEN_FILE;
    score -= (blackRooks & openFilesBitboard).count() * ROOK_OPEN_FILE;

    // rook on semi open file
    score += (whiteRooks & whiteSemiOpenFilesBitboard).count() * ROOK_SEMI_OPEN_FILE;
    score -= (blackRooks & blackSemiOpenFilesBitboard).count() * ROOK_SEMI_OPEN_FILE;

    // king file
    libchess::Square whiteKingSquare = pos.king_position(libchess::Side::White);
    libchess::Square blackKingSquare = pos.king_position(libchess::Side::Black);

    libchess::Bitboard whiteKingFile = libchess::bitboards::files[whiteKingSquare.file()];
    libchess::Bitboard blackKingFile = libchess::bitboards::files[blackKingSquare.file()];

    // rook on king file
    score += (whiteRooks & blackKingFile).count() * ROOK_ATTACK_KING_FILE;
    score -= (blackRooks & whiteKingFile).count() * ROOK_ATTACK_KING_FILE;

    // rook attack king adjacent file
    score += (whiteRooks & blackKingFile.west()).count() * ROOK_ATTACK_KING_ADJ_FILE;
    score -= (blackRooks & whiteKingFile.west()).count() * ROOK_ATTACK_KING_ADJ_FILE;

    score += (whiteRooks & blackKingFile.east()).count() * ROOK_ATTACK_KING_ADJ_FILE;
    score -= (blackRooks & whiteKingFile.east()).count() * ROOK_ATTACK_KING_ADJ_FILE;

    // rook on 7th rank
    score += (whiteRooks & libchess::bitboards::Rank7).count() * ROOK_7TH_RANK;
    score -= (blackRooks & libchess::bitboards::Rank2).count() * ROOK_7TH_RANK;

    // king protection
    libchess::Bitboard whiteKingMask = libchess::movegen::king_moves(whiteKingSquare);
    libchess::Bitboard blackKingMask = libchess::movegen::king_moves(blackKingSquare);

    for (const auto &fr : whiteBishops)
    {
        libchess::Bitboard bishopMask = libchess::movegen::bishop_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::Black);
        score += bishopMask.count() * BISHOP_MOBILITY;
    }
    for (const auto &fr : blackBishops)
    {
        libchess::Bitboard bishopMask = libchess::movegen::bishop_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::White);
        score -= bishopMask.count() * BISHOP_MOBILITY;
    }

    for (const auto &fr : whiteQueens)
    {
        libchess::Bitboard queenMask = libchess::movegen::queen_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::Black);
        score += queenMask.count() * QUEEN_MOBILITY;
    }
    for (const auto &fr : blackQueens)
    {
        libchess::Bitboard queenMask = libchess::movegen::queen_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::White);
        score -= queenMask.count() * QUEEN_MOBILITY;
    }

    for (const auto &fr : whiteRooks)
    {
        libchess::Bitboard rookMoves = libchess::movegen::rook_moves(fr, pos.occupied());
        libchess::Bitboard rookMask = rookMoves & ~pos.squares_attacked(libchess::Side::Black);
        score += rookMask.count() * ROOK_MOBILITY;

        // intersection of rook moves and rook bitboard
        libchess::Bitboard rookIntersect = rookMoves & whiteRooks;
        score += rookIntersect.count() * ROOK_CONNECTED;
    }
    for (const auto &fr : blackRooks)
    {
        libchess::Bitboard rookMoves = libchess::movegen::rook_moves(fr, pos.occupied());
        libchess::Bitboard rookMask = rookMoves & ~pos.squares_attacked(libchess::Side::White);
        score -= rookMask.count() * ROOK_MOBILITY;

        // intersection of rook moves and rook bitboard
        libchess::Bitboard rookIntersect = rookMoves & blackRooks;
        score -= rookIntersect.count() * ROOK_CONNECTED;
    }

    for (const auto &fr : whiteKnights)
    {
        libchess::Bitboard knightMask = libchess::movegen::knight_moves(fr) & ~pos.squares_attacked(libchess::Side::Black);
        score += knightMask.count() * KNIGHT_MOBILITY;
    }

    for (const auto &fr : blackKnights)
    {
        libchess::Bitboard knightMask = libchess::movegen::knight_moves(fr) & ~pos.squares_attacked(libchess::Side::White);
        score -= knightMask.count() * KNIGHT_MOBILITY;
    }

    // central pawns
    score += (whitePawns & libchess::Bitboard(0x0000001818000000)).count() * 20;
    score -= (blackPawns & libchess::Bitboard(0x0000001818000000)).count() * 20;

    // extended center pawns
    score += (whitePawns & libchess::Bitboard(0x00003c3c3c3c0000)).count() * 10;
    score -= (blackPawns & libchess::Bitboard(0x00003c3c3c3c0000)).count() * 10;
    // king safety
    if (pos.king_position(libchess::Side::White) == libchess::squares::A1 || pos.king_position(libchess::Side::White) == libchess::squares::B1 || pos.king_position(libchess::Side::White) == libchess::squares::C1 || pos.king_position(libchess::Side::White) == libchess::squares::G1 || pos.king_position(libchess::Side::White) == libchess::squares::H1)
    {
        score += 100;
    }
    if (pos.king_position(libchess::Side::Black) == libchess::squares::A8 || pos.king_position(libchess::Side::Black) == libchess::squares::B8 || pos.king_position(libchess::Side::Black) == libchess::squares::C8 || pos.king_position(libchess::Side::Black) == libchess::squares::G8 || pos.king_position(libchess::Side::Black) == libchess::squares::H8)
    {
        score -= 100;
    }

    score += KING_FRIENDLY_PAWNS * (whitePawns & libchess::movegen::king_moves(pos.king_position(libchess::Side::White))).count();
    score -= KING_FRIENDLY_PAWNS * (blackPawns & libchess::movegen::king_moves(pos.king_position(libchess::Side::Black))).count();

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

void ScarlettCore::reinitKillerMoves()
{
    // initialize the map std::map<std::pair<libchess::Move, int>, int> *killerMoves;
    if (killerMoves != nullptr)
    {
        delete killerMoves;
    }
    killerMoves = new std::unordered_set<std::pair<libchess::Move, int>, pair_hash>();
}
