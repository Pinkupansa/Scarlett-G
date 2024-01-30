#include "scarlett_core.hpp"
#include "movegen.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <set>
#include <time.h>
#include "utils.hpp"
#define DEBUG 0
#define USAGE_MODE 1
#define CHECKMATE_CONSTANT 1000000
#define BEST_POSSIBLE_SCORE 100000000
#define DRAW_OR_STALEMATE_CONSTANT 0
#define USE_BOOK 0
#define USE_TRANSPOSITION_TABLE 0
#define USE_PV_TABLE 0
#define USE_HISTORY_TABLE 0
#define MAX_SECONDS 5
ScarlettCore::ScarlettCore(int color, int depth)
{
    this->color = color;
    this->depth = depth;

    
    this->transpositionTable = new TranspositionTable();
    this->pvTable = new PVTable();
    this->zobristHasher = new PositionHasher();
    this->openingBook = new OpeningBook("../resources/opening_book.txt", zobristHasher);

    this->PAWN_VALUE = 100;
    this->KNIGHT_VALUE = 521;
    this->BISHOP_VALUE = 572;
    this->ROOK_VALUE = 824;
    this->QUEEN_VALUE = 1710;

    pieceValues[libchess::Piece::Pawn] = this->PAWN_VALUE;
    pieceValues[libchess::Piece::Knight] = this->KNIGHT_VALUE;
    pieceValues[libchess::Piece::Bishop] = this->BISHOP_VALUE;
    pieceValues[libchess::Piece::Rook] = this->ROOK_VALUE;
    pieceValues[libchess::Piece::Queen] = this->QUEEN_VALUE;
    pieceValues[libchess::Piece::King] = this->KING_VALUE;
    pieceValues[libchess::Piece::None] = this->PAWN_VALUE; //only used for en passant
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

    this->MULTICUT_REDUCTION = 2;
    this->MULTICUT_DEPTH = 2;
    this->MC_NUM = 10;
    this->MC_CUT = 3;

    this->QUIESCENCE_DEPTH = 0;

    killerMoves = new std::unordered_set<std::pair<libchess::Move, int>, pair_hash>();
    historyMoves = new std::unordered_map<libchess::Move, int, MoveHash>();
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
    delete this->openingBook;
    delete this->pvTable;
    delete this->zobristHasher;
}

libchess::Move ScarlettCore::getMove(libchess::Position pos)
{
    turn++;
    this->nodesSearched = 0;
    this->nCutoffs = 0;
    this->timeSpentSorting = 0;
    this->nKillerHits = 0;

    


#if DEBUG
    std::cout << std::endl
              << "ALL MOVES: " << std::endl;
    for (libchess::Move move : moves)
    {
        std::cout << move << " ";
    }
    std::cout << std::endl;
#endif

    
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
    clock_t start;
    double duration;
    start = omp_get_wtime();
    int bestScore = 0;
    int currentDepth = 1;
    
    if(!(openingBook->tryGetMove(pos, bestMove, turn) and USE_BOOK)){
        
        while (true){
            //print current depth 
            std::cout << "Depth: " << currentDepth << std::endl;
            //create a copy of the position for each thread
            std::vector<libchess::Move> moves = pos.legal_moves();
            orderMoves(moves, pos, depth, pos.hash());

            int maxNumberOfThreads = omp_get_max_threads();
            std::vector<libchess::Position> positions(maxNumberOfThreads);
            for(int i = 0; i < maxNumberOfThreads; i++){
                positions[i].set_fen(pos.get_fen());
            }
            std::vector<int> bestScoresInIteration(maxNumberOfThreads, -BEST_POSSIBLE_SCORE);
            std::vector<libchess::Move> bestMovesInIteration(maxNumberOfThreads);
            #pragma omp parallel for
            for (libchess::Move move : moves)
            {
                int threadId = omp_get_thread_num();
                libchess::Position position = positions[threadId];
        #if DEBUG
                std::cout << "TEST MOVE : " << move << std::endl;
        #endif
                position.makemove(move);
        #if DEBUG
                std::cout << pos << std::endl;
        #endif
                int score = -search(position, currentDepth - 1, -BEST_POSSIBLE_SCORE, -bestScoresInIteration[threadId], true);
                position.undomove();
                if (score > bestScoresInIteration[threadId])
                {
                    // std::cout << "Score: " << (2 * (1 - color) - 1) * score << std::endl;
                    bestScoresInIteration[threadId] = score;
                    bestMovesInIteration[threadId] = move;
                }
            }

            int bestScoreInIteration = -BEST_POSSIBLE_SCORE;
            libchess::Move bestMoveInIteration;
            for(int i = 0; i < maxNumberOfThreads; i++){
                if(bestScoresInIteration[i] > bestScoreInIteration){
                    bestScoreInIteration = bestScoresInIteration[i];
                    bestMoveInIteration = bestMovesInIteration[i];
                }
            }
            currentDepth++;
            bestMove = bestMoveInIteration;
            bestScore = bestScoreInIteration;
            #if USE_PV_TABLE
            pvTable->addEntry(pos.hash(), bestMove, currentDepth);
            #endif
            transpositionTable->addEntry(pos.hash(), currentDepth, bestScore, EntryType::EXACT);
            if((omp_get_wtime() - start) > MAX_SECONDS){
                break;
            }
        }
    }
    //principalVariationTable->insert(std::pair<uint64_t, libchess::Move>(pos.hash(), bestMove));
    
#if USAGE_MODE
    duration = (omp_get_wtime() - start);
    
    std::cout << std::endl;
    std::cout << "Time: " << duration << std::endl;
    // write best move in
    std::cout << "\033[1;31mBEST MOVE: "
              << bestMove << " " << color * bestScore / 100.0 << "\033[0m" << std::endl;
    //print search depth in green
    std::cout << "\033[1;32mDepth: " << currentDepth << "\033[0m" << std::endl;
    // nodesSearched
    std::cout << "Nodes searched: " << nodesSearched << std::endl;
    std::cout << "Cutoffs: " << nCutoffs << std::endl;
#endif
    return bestMove;
}


int ScarlettCore::nullMoveSearch(libchess::Position &pos, int depth, int alpha, int beta, bool quiescent)
{
    pos.makenull();
    int reduct = (USE_ADPT && depth > ADPT_DEPTH) ? NULL_MOVE_REDUCTION : NULL_MOVE_REDUCTION - 1;
    int score;
    
    //score = quiescent? -quiescenceSearch(pos, depth - reduct, -beta, -beta + 1, false) : -search(pos, -beta, -beta + 1, false, depth - reduct);
    score = -search(pos, depth - reduct, -beta, -beta + 1, false);
    pos.undonull();
    return score;
}
int ScarlettCore::quiescenceSearch(libchess::Position &pos, int alpha, int beta,  bool nullMoveAllowed, int depth)
{
    int stand_pat = evaluate(pos, alpha, beta);
    if (stand_pat >= beta or depth == 0)
        return stand_pat;
    if (alpha < stand_pat)
        alpha = stand_pat;

    /*int nullMoveScore = 0;
    if (tryNullMove(pos, depth, alpha, beta, nullMoveAllowed, nullMoveScore, true))
    {
        return nullMoveScore;
    }*/

    std::vector<libchess::Move> captures = pos.legal_captures();
    orderMoves(captures, pos, 0, pos.hash());
    //add checks to captures

    for (libchess::Move move : captures)
    {
        if (move.type() == libchess::MoveType::ksc || move.type() == libchess::MoveType::qsc)
            continue;
        pos.makemove(move);
        int score = -quiescenceSearch(pos, -beta, -alpha, true, depth - 1);
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

bool ScarlettCore::moveIsCheck(libchess::Position &pos, libchess::Move move)
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
bool ScarlettCore::isTactical(libchess::Position &pos, libchess::Move move)
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

void ScarlettCore::orderMoves(std::vector<libchess::Move> &moves, libchess::Position &pos, int depth, uint64_t hash)
{
    std::vector<int> moveScores(moves.size(), 0);
    libchess::Side opponent = (libchess::Side)(1 - (int)pos.turn());

    PVEntry entry;

    #if USE_PV_TABLE
        bool pvMoveFound = pvTable->tryGetEntry(hash, entry);
    #endif
    for (int i = 0; i < moves.size(); i++)
    {

        libchess::Move move = moves[i];
    
        int moveScoreGuess = 0;
        libchess::Piece movePieceType = pos.piece_on(move.from());
        libchess::Piece capturePieceType = pos.piece_on(move.to());

        #if USE_PV_TABLE
        if(pvMoveFound && move == entry.pvMove){
            moveScoreGuess += 20000;
            continue;
        }
        #endif
        if (moveIsCheck(pos, move))
        {
            moveScoreGuess += 1000;
        }
        // if the move is a capture, add the value of the captured piece to the move score guess
        if (move.is_capturing())
        {
            
            moveScoreGuess += pieceValues[(int)capturePieceType] - pieceValues[(int)movePieceType];
        }
        #if USE_HISTORY_TABLE
            else{
                //if move in history, add history score
                
                if(historyMoves->find(move) != historyMoves->end()){
                    moveScoreGuess += (*historyMoves)[move];
                }
            }
        #endif

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

bool ScarlettCore::tryNullMove(libchess::Position &pos, int depth, int alpha, int beta, bool nullMoveAllowed, int &score, bool quiescent)
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
        int tryScore = nullMoveSearch(pos, depth, alpha, beta, quiescent);
        
        
        if (tryScore >= beta)
        {
            score = tryScore;
            return true;
        }
    }
    return false;
}

bool ScarlettCore::checkStaleMateOrCheckmateFromMoves(const libchess::Position &pos, const std::vector<libchess::Move> &legalMoves, int &score)
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

bool ScarlettCore::tryMultiCutPruning(libchess::Position &pos, const std::vector<libchess::Move> &moves, int depth, int beta){
    if (depth >= MULTICUT_DEPTH)
    {
        int count = 0;

        for (int i = 0; i < std::min((int)moves.size(), MC_NUM); i++)
        {
            libchess::Move move = moves[i];
            pos.makemove(move);
            int score = -search(pos, depth - MULTICUT_REDUCTION - 1, -beta, -beta + 1, true);
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
bool ScarlettCore::futilityPrune(const libchess::Position &pos, libchess::Move move, int depth, int alpha, int beta, int posScore)
{
    
    if(depth > 3){
        return false;
    }
    int margin = 0;
    if(depth == 1){
        margin = FUTILITY_MARGIN_1;
    }
    else if(depth == 2){
        margin = FUTILITY_MARGIN_2;
    }
    else if(depth == 3){
        margin = FUTILITY_MARGIN_3;
    }

    bool capture = move.is_capturing();
    if(capture){
        //capture piece value
        int captureValue = pieceValues[(int)pos.piece_on(move.to())];
        //if the move is a capture, and the value of the captured piece is greater than the margin, don't prune
        if(captureValue + posScore + margin > alpha){
            return false;
        }
    }
    bool promotion = move.is_promoting();
    if(!promotion) return false;
    //promotion piece value
    int promotionValue = pieceValues[(int)move.promotion()];
    //if the move is a promotion, and the value of the promoted piece is greater than the margin, don't prune
    if(promotionValue + posScore + margin > alpha){
        return false;
    }

    return true;
}
int ScarlettCore::search(libchess::Position &pos, int depth, int alpha, int beta, bool nullMoveAllowed)
{
    /*Alpha is the best score that the current player (treated as a maximizing player in negamax framework) can achieve so far.
    Beta is the best score that the opponent (treated as a minimizing player in negamax framework) can achieve so far.
    Can achieve means that the opponent can force a score of beta or higher, and the current player can force a score of alpha or higher.*/

    //nodesSearched++;

    if (pos.is_draw())
    {
        return DRAW_OR_STALEMATE_CONSTANT;
    }
    //print thread id and position
    uint64_t hash = pos.hash();
    #if USE_TRANSPOSITION_TABLE
        TTEntry entry;
        
        if(transpositionTable->tryGetEntry(hash, entry)){
            if(entry.depth >= depth){
                switch (entry.type)
                {
                    case EntryType::EXACT:
                        return entry.score;
                        break;
                    
                    case EntryType::LOWERBOUND:
                        if(entry.score > alpha){
                            alpha = entry.score;
                        }
                        break;
                    
                    case EntryType::UPPERBOUND:
                        if(entry.score < beta){
                            beta = entry.score;
                        }
                        break;
                    
                    default:
                        break;
                }
            }
            if (alpha >= beta)
            {
                return alpha;
            }

        }
    #endif

    // If search ended, evaluate position
    if (depth <= 0)
    {
        int score = evaluate(pos, alpha, beta);

        #if USE_TRANSPOSITION_TABLE
           transpositionTable->addEntry(hash, depth, score, EntryType::EXACT);
        #endif
        return score;
    }

    int nullMoveScore = 0;
    if (tryNullMove(pos, depth, alpha, beta, nullMoveAllowed, nullMoveScore, false))
    {
        return nullMoveScore;
    }

    std::vector<libchess::Move> moves = pos.legal_moves();
    int checkmateOrStalemateScore;
    if (checkStaleMateOrCheckmateFromMoves(pos, moves, checkmateOrStalemateScore))
    {
        int score = checkmateOrStalemateScore * depth;
        #if USE_TRANSPOSITION_TABLE
            transpositionTable->addEntry(hash, depth, score, EntryType::EXACT);
        #endif
        return score;
    }

    orderMoves(moves, pos, depth, hash);

    if (tryMultiCutPruning(pos, moves, depth, beta))
    {
        return beta;
    }

    int b = beta;
    int score;
    bool first = true;
    int originalAlpha = alpha;
    libchess::Move bestMove = moves[0];
    int posScore = evaluate(pos, alpha, beta);
    for (libchess::Move move : moves)
    {  
        if(depth <= 3 and futilityPrune(pos, move, depth, alpha, beta, posScore)){
            continue;
        }
        pos.makemove(move);
        score = -search(pos, depth - 1, -b, -alpha, true);
        if(score > alpha && score < beta && !first){
            score = -search(pos, depth - 1, -beta, -alpha, true);
        }
        pos.undomove();
        if(score > alpha){
            alpha = score;
            bestMove = move;
        }
        if(alpha >= beta){
            nCutoffs++;
            //const std::pair<libchess::Move, int> killerMove = std::pair<libchess::Move, int>(move, depth);
            //killerMoves->insert(killerMove);
            #if USE_HISTORY_TABLE
                updateHistory(move, depth);
            #endif
            #if USE_TRANSPOSITION_TABLE
                transpositionTable->addEntry(hash, depth, alpha, EntryType::LOWERBOUND);
            #endif
            return alpha;
            break;
        }
        b = alpha + 1;
        first = false;
        
    }
    #if USE_PV_TABLE
        if(score > originalAlpha){
            pvTable->addEntry(hash, bestMove, depth);
        }
    #endif
    #if USE_TRANSPOSITION_TABLE
        EntryType entryType;

        if (alpha <= originalAlpha)
        {
            entryType = EntryType::UPPERBOUND;
        }
        else
        {
            entryType = EntryType::EXACT;
        }
        transpositionTable->addEntry(hash, depth, alpha, entryType);
        
        
        
    #endif
    return alpha;
}
  int ScarlettCore::evaluateMaterialBalance(const libchess::Bitboard& whitePawns, const libchess::Bitboard& whiteKnights, const libchess::Bitboard& whiteBishops, const libchess::Bitboard& whiteRooks, const libchess::Bitboard& whiteQueens, const libchess::Bitboard& blackPawns, const libchess::Bitboard& blackKnights, const libchess::Bitboard& blackBishops, const libchess::Bitboard& blackRooks, const libchess::Bitboard& blackQueens){
    int score = 0;
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
    return score;
  
  }
int ScarlettCore::evaluate(libchess::Position &pos, int alpha, int beta)
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

    
    libchess::Bitboard whitePawns = pos.pieces(libchess::Side::White, libchess::Piece::Pawn);
    libchess::Bitboard blackPawns = pos.pieces(libchess::Side::Black, libchess::Piece::Pawn);

    libchess::Bitboard whiteKnights = pos.pieces(libchess::Side::White, libchess::Piece::Knight);
    libchess::Bitboard blackKnights = pos.pieces(libchess::Side::Black, libchess::Piece::Knight);

    libchess::Bitboard whiteBishops = pos.pieces(libchess::Side::White, libchess::Piece::Bishop);
    libchess::Bitboard blackBishops = pos.pieces(libchess::Side::Black, libchess::Piece::Bishop);

    libchess::Bitboard whiteRooks = pos.pieces(libchess::Side::White, libchess::Piece::Rook);
    libchess::Bitboard blackRooks = pos.pieces(libchess::Side::Black, libchess::Piece::Rook);

    libchess::Bitboard whiteQueens = pos.pieces(libchess::Side::White, libchess::Piece::Queen);
    libchess::Bitboard blackQueens = pos.pieces(libchess::Side::Black, libchess::Piece::Queen);

    libchess::Bitboard whitePassedPawns = pos.passed_pawns(libchess::Side::White);
    libchess::Bitboard blackPassedPawns = pos.passed_pawns(libchess::Side::Black);

    int nbPassedPawnsWhite = whitePassedPawns.count();
    if(nbPassedPawnsWhite > 0){
        score += nbPassedPawnsWhite * PASSED_PAWN_MULT;
        score += 1 * (whitePawns & libchess::bitboards::Rank6).count() * PASSED_PAWN_MULT;
        score += 2 * (whitePawns & libchess::bitboards::Rank7).count() * PASSED_PAWN_MULT;
    }

    int nbPassedPawnsBlack = blackPassedPawns.count();
    if(nbPassedPawnsBlack > 0){
        score -= nbPassedPawnsBlack * PASSED_PAWN_MULT;
        score -= 1 * (blackPawns & libchess::bitboards::Rank3).count() * PASSED_PAWN_MULT;
        score -= 2 * (blackPawns & libchess::bitboards::Rank2).count() * PASSED_PAWN_MULT;
    }

    score += evaluateMaterialBalance(whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, blackPawns, blackKnights, blackBishops, blackRooks, blackQueens);

    int sideMultiplier = (pos.turn() == libchess::Side::White) ? 1 : -1;
    if(pos.turn() == libchess::Side::Black){
        beta = -alpha;
        alpha = -beta;
    }
    if(score >= beta + LAZY_EVAL_MARGIN ){
        return score * sideMultiplier;
    }
    if(score <= alpha - LAZY_EVAL_MARGIN){
        return score * sideMultiplier;
    }

    int whiteSemiOpenFiles = 0;
    libchess::Bitboard whiteSemiOpenFilesBitboard;
    int blackSemiOpenFiles = 0;
    libchess::Bitboard blackSemiOpenFilesBitboard;
    for (const auto &file : libchess::bitboards::files)
    {
        if ((whitePawns & file).empty())
        {
            whiteSemiOpenFiles++;
            whiteSemiOpenFilesBitboard |= file;
        }

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

    libchess::Bitboard whiteRooksOnOpenFile = whiteRooks & openFilesBitboard;
    libchess::Bitboard blackRooksOnOpenFile = blackRooks & openFilesBitboard;

    libchess::Bitboard whiteRooksOnSemiOpenFile = whiteRooks & whiteSemiOpenFilesBitboard;
    libchess::Bitboard blackRooksOnSemiOpenFile = blackRooks & blackSemiOpenFilesBitboard;

    // rook on open file
    score += whiteRooksOnOpenFile.count() * ROOK_OPEN_FILE;
    score -= blackRooksOnOpenFile.count() * ROOK_OPEN_FILE;

    // rook on semi open file
    score += whiteRooksOnSemiOpenFile.count() * ROOK_SEMI_OPEN_FILE;
    score -= blackRooksOnSemiOpenFile.count() * ROOK_SEMI_OPEN_FILE;

    // king file
    libchess::Square whiteKingSquare = pos.king_position(libchess::Side::White);
    libchess::Square blackKingSquare = pos.king_position(libchess::Side::Black);

    libchess::Bitboard whiteKingFile = libchess::bitboards::files[whiteKingSquare.file()];
    libchess::Bitboard blackKingFile = libchess::bitboards::files[blackKingSquare.file()];

    // rook on king file
    score += (blackKingFile & whiteRooksOnSemiOpenFile).count() * ROOK_ATTACK_KING_FILE;
    score -= (whiteKingFile & blackRooksOnSemiOpenFile).count() * ROOK_ATTACK_KING_FILE;

    
    // rook attack king adjacent file
    score += (whiteRooksOnOpenFile & blackKingFile.west()).count() * ROOK_ATTACK_KING_ADJ_FILE;
    score -= (blackRooksOnOpenFile & whiteKingFile.west()).count() * ROOK_ATTACK_KING_ADJ_FILE;

    score += (whiteRooksOnOpenFile & blackKingFile.east()).count() * ROOK_ATTACK_KING_ADJ_FILE;
    score -= (blackRooksOnOpenFile & whiteKingFile.east()).count() * ROOK_ATTACK_KING_ADJ_FILE;

    // rook on 7th rank
    score += (whiteRooks & libchess::bitboards::Rank7).count() * ROOK_7TH_RANK;
    score -= (blackRooks & libchess::bitboards::Rank2).count() * ROOK_7TH_RANK;

    // king protection
    libchess::Bitboard whiteKingMask = libchess::movegen::king_moves(whiteKingSquare);
    libchess::Bitboard blackKingMask = libchess::movegen::king_moves(blackKingSquare);
 
    libchess::Bitboard blackSquaresAttacked = pos.squares_attacked(libchess::Side::Black);
    libchess::Bitboard whiteSquaresAttacked = pos.squares_attacked(libchess::Side::White);
    
    for (const auto &fr : whiteBishops)
    {
        libchess::Bitboard bishopMask = libchess::movegen::bishop_moves(fr, pos.occupied()) & ~blackSquaresAttacked;
        score += bishopMask.count() * BISHOP_MOBILITY;
    }
    for (const auto &fr : blackBishops)
    {
        libchess::Bitboard bishopMask = libchess::movegen::bishop_moves(fr, pos.occupied()) & ~whiteSquaresAttacked;
        score -= bishopMask.count() * BISHOP_MOBILITY;
    }

    for (const auto &fr : whiteQueens)
    {
        libchess::Bitboard queenMask = libchess::movegen::queen_moves(fr, pos.occupied()) & ~blackSquaresAttacked;
        score += queenMask.count() * QUEEN_MOBILITY;
    }
    for (const auto &fr : blackQueens)
    {
        libchess::Bitboard queenMask = libchess::movegen::queen_moves(fr, pos.occupied()) & ~whiteSquaresAttacked;
        score -= queenMask.count() * QUEEN_MOBILITY;
    }

    for (const auto &fr : whiteRooks)
    {
        libchess::Bitboard rookMoves = libchess::movegen::rook_moves(fr, pos.occupied());
        libchess::Bitboard rookMask = rookMoves & ~blackSquaresAttacked;
        score += rookMask.count() * ROOK_MOBILITY;

        // intersection of rook moves and rook bitboard
        libchess::Bitboard rookIntersect = rookMoves & whiteRooks;
        score += rookIntersect.count() * ROOK_CONNECTED;
    }
    for (const auto &fr : blackRooks)
    {
        libchess::Bitboard rookMoves = libchess::movegen::rook_moves(fr, pos.occupied());
        libchess::Bitboard rookMask = rookMoves & ~whiteSquaresAttacked;
        score -= rookMask.count() * ROOK_MOBILITY;

        // intersection of rook moves and rook bitboard
        libchess::Bitboard rookIntersect = rookMoves & blackRooks;
        score -= rookIntersect.count() * ROOK_CONNECTED;
    }

    for (const auto &fr : whiteKnights)
    {
        libchess::Bitboard knightMask = libchess::movegen::knight_moves(fr) & ~blackSquaresAttacked;
        score += knightMask.count() * KNIGHT_MOBILITY;
    }

    for (const auto &fr : blackKnights)
    {
        libchess::Bitboard knightMask = libchess::movegen::knight_moves(fr) & ~whiteSquaresAttacked;
        score -= knightMask.count() * KNIGHT_MOBILITY;
    }

    // central pawns
    score += (whitePawns & centralSquares).count() * 20;
    score -= (blackPawns & extendedCenterSquares).count() * 20;

    // extended center pawns
    score += (whitePawns & centralSquares).count() * 10;
    score -= (blackPawns & extendedCenterSquares).count() * 10;
    // king safety
    if (whiteKingSquare == libchess::squares::A1 || whiteKingSquare == libchess::squares::B1 || whiteKingSquare == libchess::squares::C1 || whiteKingSquare == libchess::squares::G1 || whiteKingSquare == libchess::squares::H1)
    {
        score += 100;
    }
    if (blackKingSquare == libchess::squares::A8 || blackKingSquare == libchess::squares::B8 || blackKingSquare == libchess::squares::C8 || blackKingSquare == libchess::squares::G8 || blackKingSquare == libchess::squares::H8)
    {
        score -= 100;
    }

    score += KING_FRIENDLY_PAWNS * (whitePawns & libchess::movegen::king_moves(whiteKingSquare)).count();
    score -= KING_FRIENDLY_PAWNS * (blackPawns & libchess::movegen::king_moves(blackKingSquare)).count();

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

    return score * sideMultiplier;
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

void ScarlettCore::reinitHistoryMoves()
{
    // initialize the map std::map<std::pair<libchess::Move, int>, int> *killerMoves;
    if (historyMoves != nullptr)
    {
        delete historyMoves;
    }
    historyMoves = new std::unordered_map<libchess::Move, int, MoveHash>();
}

void ScarlettCore::updateHistory(libchess::Move move, int depth)
{
    if(move.is_capturing()){
        return;
    }
    if (historyMoves->find(move) != historyMoves->end())
    {
        #pragma omp critical
        (*historyMoves)[move] += 1 << depth;
    }
    else
    {
        #pragma omp critical
        (*historyMoves)[move] = 1 << depth;
    }
}