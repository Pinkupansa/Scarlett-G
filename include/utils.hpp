#ifndef UTILS_HPP
#define UTILS_HPP

#include <position.hpp>
#include <iterator>
#include <string>
#include <sstream>
#include <iostream>

class PositionHasher
{
    private:
        //zobrist hash table
        uint64_t zobristTable[64][12];
        uint64_t zobristBlackToMove;
        uint64_t zobristCastling[16];
        uint64_t zobristEnPassant[64];


    public: 

        PositionHasher(){
            //seed random
            srand(time(NULL));
            //init zobrist table
            for(int i = 0; i < 64; i++){
                for(int j = 0; j < 12; j++){
                    zobristTable[i][j] = rand();
                }
            }
            zobristBlackToMove = rand();
            for(int i = 0; i < 16; i++){
                zobristCastling[i] = rand();
            }
            for(int i = 0; i < 64; i++){
                zobristEnPassant[i] = rand();
            }
        }


        uint64_t calculateCustomZobristHash(const libchess::Position &pos, bool verbose = false){
            //calculates a custom zobrist hash that doesnt take into account move count
            uint64_t hash = 0;
            for(int i = 0; i < 64; i++){
                int piece = pos.piece_on(libchess::Square(i));
                if(piece != libchess::Piece::None){
                    hash ^= zobristTable[i][piece];
                }
            }

            if(pos.turn() == libchess::Side::Black){
                hash ^= zobristBlackToMove;
            }

            if(pos.can_castle(libchess::Side::White, libchess::MoveType::ksc)){
                hash ^= zobristCastling[0]; 
            }
            if(pos.can_castle(libchess::Side::White, libchess::MoveType::qsc)){
                hash ^= zobristCastling[1];
            }
            if(pos.can_castle(libchess::Side::Black, libchess::MoveType::ksc)){
                hash ^= zobristCastling[2];
            }
            if(pos.can_castle(libchess::Side::Black, libchess::MoveType::qsc)){
                hash ^= zobristCastling[3];
            }

            
            //convert ep square to index with formula (rank * 8) + file
            int epIndex = pos.ep().rank() * 8 + pos.ep().file();
            if(epIndex == 255) return hash;
            hash ^= zobristEnPassant[epIndex];
           

            return hash;
        }

       uint64_t calculateShortFenHash(const libchess::Position &pos){
            std::string fen = pos.get_fen();
            //split fen into words
            std::istringstream iss(fen);
            std::vector<std::string> words(std::istream_iterator<std::string>{iss},
                                        std::istream_iterator<std::string>());
            std::string shortFen = words[0] + " " + words[1] + " " + words[2] + " -";
            //get hash
            libchess::Position shortPos;
            shortPos.set_fen(shortFen);
            uint64_t hash = shortPos.hash();
            return hash;
        }
};

#endif