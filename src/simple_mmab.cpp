#include "simple_mmab.hpp"
#include "movegen.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <set>
#define PAWN_VALUE 100
#define KNIGHT_VALUE 300
#define BISHOP_VALUE 300
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 10000
#define PAWN_ADVANCE_A 10
#define PAWN_ADVANCE_B 20
#define PASSED_PAWN_MULT 10
#define DOUBLED_PAWN_PENALTY 10
#define ISOLATED_PAWN_PENALTY 10
#define BACKWARD_PAWN_PENALTY 5
#define WEAK_SQUARE_PENALTY 5
#define PASSED_PAWN_ENEMY_KING_DISTANCE 10
#define KNIGHT_SQ_MULT 6
#define BISHOP_MOBILITY 4
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
#define KING_NO_FRIENDLY_PAWN 35
#define KING_NO_FRIENDLY_PAWN_ADJ 10
#define KING_FRIENDLY_PAWN_ADVANCED 10
#define KING_NO_ENEMY_PAWN 17
#define KING_NO_ENEMY_PAWN_ADJ 5
#define KING_PRESSURE_MULT 4


#define NULL_MOVE_REDUCTION 3
#define USE_ADPT 1
#define ADPT_DEPTH 6
#define CANDIDATE_MOVE_COUNT 15
SimpleMMAB::SimpleMMAB(int isWhite, int depth){
    this->color = color;
    this->depth = depth;

    this->transpositionTable = new std::map<uint64_t, std::pair<int, int>>();

}

SimpleMMAB::~SimpleMMAB(){
    delete this->transpositionTable;
}

libchess::Move SimpleMMAB::getMove(libchess::Position pos){
    this->nodesSearched = 0;
    std::vector<libchess::Move> moves = pos.legal_moves();
    libchess::Move bestMove;
    int bestScore;
    
    
    bestScore = -10000000;
    for(libchess::Move move : moves){
        pos.makemove(move);
        int score = -minimax(pos, depth - 1, -1000000, 1000000, true);
        pos.undomove();
        if(score > bestScore){
            bestScore = score;
            bestMove = move;
        }
    }
    
    std::cout << "Score with calculation: " << color*bestScore << std::endl;
    std::cout << " \n ##BEST MOVE##: \n" << bestMove << std::endl;
    //nodesSearched
    std::cout << "Nodes searched: " << nodesSearched << std::endl;
    return bestMove;
}

int SimpleMMAB::nullMoveSearch(libchess::Position pos, int depth, int alpha, int beta){
    pos.makenull();
    int reduct = (USE_ADPT && depth > ADPT_DEPTH) ? NULL_MOVE_REDUCTION : NULL_MOVE_REDUCTION-1;
    int score;
    if(color == 1){
        score = minimax(pos, depth - reduct - 1, beta-1, beta, false);
    }
    else{
        score = minimax(pos, depth - reduct - 1, alpha, alpha+1, false);
    }
    pos.undonull();
    return score;
}

int SimpleMMAB::minimax(libchess::Position pos, int depth, int alpha, int beta, bool nullMoveAllowed){
    //if transposition table contains the current position and the depth is greater than or equal to the depth of the position in the table, return the score of the position in the table
    /*if(transpositionTable->find(pos.hash()) != transpositionTable->end()){
        std::pair<int, int> scoreAndDepth = transpositionTable->at(pos.hash());
        if(scoreAndDepth.second >= depth){
            return scoreAndDepth.first;
        }
    }*/
    nodesSearched++;
    if(depth <= 0){
        int score = evaluate(pos);
        //transpositionTable->insert(std::pair<uint32_t, std::pair<int, int>>(pos.hash(), std::pair<int, int>(score, depth)));
        return score;
    }

    /*if(nullMoveAllowed && !pos.in_check()){
        int score = nullMoveSearch(pos, depth, alpha, beta, color);
        if(color==1){
            if(score >= beta){
                return score;
            }
        }
        else{
            if(score <= alpha){
                return score;
            }
        }
    }

    nullMoveAllowed = true;*/

    std::vector<libchess::Move> moves = pos.legal_moves();
    //check if moves contains a Move for which from = to
    

    //put captures at the front of the list
    
    /*std::map<std::string, int> moveStaticScore;
    for(libchess::Move move : moves){
        pos.makemove(move);
        int score = evaluate(pos, -color);
        pos.undomove();
        moveStaticScore.insert(std::pair<std::string, int>((std::string)move, score));

    }
    
    //partial sort of CANDIDATE_MOVE_COUNT moves
    std::partial_sort(moves.begin(), std::min(moves.begin() + CANDIDATE_MOVE_COUNT, moves.end()), moves.end(), [moveStaticScore, color](libchess::Move a, libchess::Move b){
        return color == 1? moveStaticScore.at((std::string)a) > moveStaticScore.at((std::string)b) : moveStaticScore.at((std::string)b) > moveStaticScore.at((std::string)a);
    });*/

    
    /*std::cout << color; 
    for(libchess::Move move : moves){
        std::cout << " " << moveStaticScore.at((std::string)move);
    }
    std::cout << std::endl;*/

    //moves.resize(std::min(CANDIDATE_MOVE_COUNT, (int)moves.size()));
    
    for(libchess::Move move : moves){

        pos.makemove(move);
        int score = -minimax(pos, depth-1, -beta, -alpha, nullMoveAllowed);
        pos.undomove();
        if(score >= beta){
            return score;
        }
        if(score > alpha){
            alpha = score;
        }
        
    }
    return alpha;
}



int SimpleMMAB::evaluate(libchess::Position pos){
    int score = 0;
    std::vector<libchess::Move> moves;
    
    if(pos.is_checkmate()){
        return -1000000;
    }
    if(pos.is_stalemate()){
        return 0;
    }
    if(pos.is_draw()){
        return 0;
    }
    
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

    score += pos.pieces(libchess::Side::White, libchess::Piece::King).count() * KING_VALUE;
    score -= pos.pieces(libchess::Side::Black, libchess::Piece::King).count() * KING_VALUE;

    score += pos.passed_pawns(libchess::Side::White).count() * PASSED_PAWN_MULT;
    score -= pos.passed_pawns(libchess::Side::Black).count() * PASSED_PAWN_MULT;

    //white semi open files
    /*int whiteSemiOpenFiles = 0; //set of white semi open files
    libchess::Bitboard whiteSemiOpenFilesBitboard;

    if((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileA).empty()){
        whiteSemiOpenFiles ++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileA;
    }
    if((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileB).empty()){
        whiteSemiOpenFiles ++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileB;
    }
    if((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileC).empty()){
        whiteSemiOpenFiles ++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileC;
    }
    if((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileD).empty()){
        whiteSemiOpenFiles ++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileD;
    }
    if((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileE).empty()){
        whiteSemiOpenFiles ++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileE;
    }
    if((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileF).empty()){
        whiteSemiOpenFiles ++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileF;
    }
    if((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileG).empty()){
        whiteSemiOpenFiles ++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileG;
    }
    if((pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::bitboards::FileH).empty()){
        whiteSemiOpenFiles ++;
        whiteSemiOpenFilesBitboard |= libchess::bitboards::FileH;
    }




    //set of black semi open files
    int blackSemiOpenFiles = 0;
    libchess::Bitboard blackSemiOpenFilesBitboard;

    if((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileA).empty()){
        blackSemiOpenFiles ++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileA;
    }
    if((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileB).empty()){
        blackSemiOpenFiles ++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileB;
    }
    if((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileC).empty()){
        blackSemiOpenFiles ++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileC;
    }
    if((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileD).empty()){
        blackSemiOpenFiles ++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileD;
    }
    if((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileE).empty()){
        blackSemiOpenFiles ++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileE;
    }
    if((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileF).empty()){
        blackSemiOpenFiles ++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileF;
    }
    if((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileG).empty()){
        blackSemiOpenFiles ++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileG;
    }
    if((pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::bitboards::FileH).empty()){
        blackSemiOpenFiles ++;
        blackSemiOpenFilesBitboard |= libchess::bitboards::FileH;
    }



    //open files

    libchess::Bitboard openFilesBitboard = whiteSemiOpenFilesBitboard & blackSemiOpenFilesBitboard;
    //doubled paws = nbpawns - 8 + nbsemiopenfiles

    score -= (pos.pieces(libchess::Side::White, libchess::Piece::Pawn).count() - 8 + whiteSemiOpenFiles) * DOUBLED_PAWN_PENALTY;
    score += (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn).count() - 8 + blackSemiOpenFiles) * DOUBLED_PAWN_PENALTY;

    //isolated pawns

    score -= (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & whiteSemiOpenFilesBitboard.west() & whiteSemiOpenFilesBitboard.east()).count() * ISOLATED_PAWN_PENALTY;
    score += (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & blackSemiOpenFilesBitboard.west() & blackSemiOpenFilesBitboard.east()).count() * ISOLATED_PAWN_PENALTY;

    //bishop pair
    if(pos.pieces(libchess::Side::White, libchess::Piece::Bishop).count() >= 2){
        score += BISHOP_PAIR;
    }
    if(pos.pieces(libchess::Side::Black, libchess::Piece::Bishop).count() >= 2){
        score -= BISHOP_PAIR;
    }

    //rook on open file
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & openFilesBitboard).count() * ROOK_OPEN_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & openFilesBitboard).count() * ROOK_OPEN_FILE;

    //rook on semi open file
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & whiteSemiOpenFilesBitboard).count() * ROOK_SEMI_OPEN_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & blackSemiOpenFilesBitboard).count() * ROOK_SEMI_OPEN_FILE;

    //king file
    libchess::Square whiteKingSquare = pos.king_position(libchess::Side::White);
    libchess::Square blackKingSquare = pos.king_position(libchess::Side::Black);

    libchess::Bitboard whiteKingFile = libchess::bitboards::files[whiteKingSquare.file()];
    libchess::Bitboard blackKingFile = libchess::bitboards::files[blackKingSquare.file()];

    //rook on king file
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & blackKingFile).count() * ROOK_ATTACK_KING_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & whiteKingFile).count() * ROOK_ATTACK_KING_FILE;

    //rook attack king adjacent file
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & blackKingFile.west()).count() * ROOK_ATTACK_KING_ADJ_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & whiteKingFile.west()).count() * ROOK_ATTACK_KING_ADJ_FILE;

    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & blackKingFile.east()).count() * ROOK_ATTACK_KING_ADJ_FILE;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & whiteKingFile.east()).count() * ROOK_ATTACK_KING_ADJ_FILE;

    //rook on 7th rank
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Rook) & libchess::bitboards::Rank7).count() * ROOK_7TH_RANK;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Rook) & libchess::bitboards::Rank2).count() * ROOK_7TH_RANK;

    //king protection
    libchess::Bitboard whiteKingMask = libchess::movegen::king_moves(whiteKingSquare);
    libchess::Bitboard blackKingMask = libchess::movegen::king_moves(blackKingSquare);

    score += (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & whiteKingMask).count() * KING_NO_FRIENDLY_PAWN_ADJ;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & blackKingMask).count() * KING_NO_FRIENDLY_PAWN_ADJ;

    for(const auto &fr : pos.pieces(libchess::Side::White, libchess::Piece::Bishop)){
        libchess::Bitboard bishopMask = libchess::movegen::bishop_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::Black);
        score += bishopMask.count() * BISHOP_MOBILITY;
    }
    for(const auto &fr : pos.pieces(libchess::Side::Black, libchess::Piece::Bishop)){
        libchess::Bitboard bishopMask = libchess::movegen::bishop_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::White);
        score -= bishopMask.count() * BISHOP_MOBILITY;
    }

    for(const auto &fr : pos.pieces(libchess::Side::White, libchess::Piece::Queen)){
        libchess::Bitboard queenMask = libchess::movegen::queen_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::Black); 
        score += queenMask.count() * QUEEN_MOBILITY;
    }
    for(const auto &fr : pos.pieces(libchess::Side::Black, libchess::Piece::Queen)){
        libchess::Bitboard queenMask = libchess::movegen::queen_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::White);
        score -= queenMask.count() * QUEEN_MOBILITY;
    }

    for(const auto &fr : pos.pieces(libchess::Side::White, libchess::Piece::Rook)){
        libchess::Bitboard rookMask = libchess::movegen::rook_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::Black);
        score += rookMask.count() * ROOK_MOBILITY;
    }
    for(const auto &fr : pos.pieces(libchess::Side::Black, libchess::Piece::Rook)){
        libchess::Bitboard rookMask = libchess::movegen::rook_moves(fr, pos.occupied()) & ~pos.squares_attacked(libchess::Side::White);
        score -= rookMask.count() * ROOK_MOBILITY;
    }

    for(const auto &fr : pos.pieces(libchess::Side::White, libchess::Piece::Knight)){
        libchess::Bitboard knightMask = libchess::movegen::knight_moves(fr) & ~pos.squares_attacked(libchess::Side::Black);
        score += knightMask.count() * KNIGHT_SQ_MULT;
    }

    for(const auto &fr : pos.pieces(libchess::Side::Black, libchess::Piece::Knight)){
        libchess::Bitboard knightMask = libchess::movegen::knight_moves(fr) & ~pos.squares_attacked(libchess::Side::White);
        score -= knightMask.count() * KNIGHT_SQ_MULT;
    }

    //central pawns
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::Bitboard(0x0000001818000000)).count() * 20;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::Bitboard(0x0000001818000000)).count() * 20;

    //extended center pawns
    score += (pos.pieces(libchess::Side::White, libchess::Piece::Pawn) & libchess::Bitboard(0x00003c3c3c3c0000)).count() * 10;
    score -= (pos.pieces(libchess::Side::Black, libchess::Piece::Pawn) & libchess::Bitboard(0x00003c3c3c3c0000)).count() * 10;
    //king safety
    if(pos.king_position(libchess::Side::White) == libchess::squares::A1 || pos.king_position(libchess::Side::White) == libchess::squares::B1 || pos.king_position(libchess::Side::White) == libchess::squares::C1 || pos.king_position(libchess::Side::White) == libchess::squares::G1 || pos.king_position(libchess::Side::White) == libchess::squares::H1){
        score += 100;
    }
    if(pos.king_position(libchess::Side::Black) == libchess::squares::A8 || pos.king_position(libchess::Side::Black) == libchess::squares::B8 || pos.king_position(libchess::Side::Black) == libchess::squares::C8 || pos.king_position(libchess::Side::Black) == libchess::squares::G8 || pos.king_position(libchess::Side::Black) == libchess::squares::H8){
        score -= 100;
    }

    //castling rights
    if(pos.can_castle(libchess::Side::White, libchess::MoveType::ksc)){
        score += 70;
    }
    if(pos.can_castle(libchess::Side::White, libchess::MoveType::qsc)){
        score += 60;
    }
    if(pos.can_castle(libchess::Side::Black, libchess::MoveType::ksc)){
        score -= 70;
    }
    if(pos.can_castle(libchess::Side::Black, libchess::MoveType::qsc)){
        score -= 60;
    }

    */

    return score * (pos.turn() == libchess::Side::White ? 1 : -1);
}