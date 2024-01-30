#ifndef OPENING_BOOK_HPP
#define OPENING_BOOK_HPP

#include <position.hpp>
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>
#include <cmath>
#include "utils.hpp"

class OpeningBook
{
    struct MoveData
    {
        public:
            libchess::Move move;
            int numGames;
            MoveData(libchess::Move move, int numGames){
                this->move = move;
                this->numGames = numGames;


            }
    };
    public:
        
        OpeningBook(std::string bookFile, PositionHasher* zobristHasher){
            this->bookFile = bookFile;
            this->zobristHasher = zobristHasher;
            //parseBook();
        }
        bool tryGetMove(libchess::Position &pos, libchess::Move &move, int turn){
            double pow = (double)turn / 2.0;
            //gen pos fen
            uint64_t hash = zobristHasher->calculateShortFenHash(pos);
            //get moves
            std::vector<MoveData> moves = book[hash];
            if(moves.size() == 0){
                return false;
            }
            //get random move in proportion to numGames
            int totalGames = 0;
            for (auto const& m : moves)
            {
                totalGames += std::pow(m.numGames, pow);
            }
            int randomNum = rand() % totalGames;
            int currentNum = 0;
            MoveData bestMove = moves[0];
            for (auto const& m : moves)
            {
                currentNum += std::pow(m.numGames, pow);
                if(currentNum >= randomNum){
                    bestMove = m;
                    break;
                }
            }
            move = bestMove.move;
            return true;
        }
        void parseBook(){
            std::ifstream bookStream;
            bookStream.open(bookFile);
            if (!bookStream.is_open()) {
                throw std::runtime_error("Failed to open book file");
            }
            std::string line;
            uint64_t currentHash = 0;
            std::vector<MoveData> currentMoves;
            libchess::Position pos;
            bool first = true;
            while (std::getline(bookStream, line))
            {
                
                //split into words. if first word is "pos", 4 next words are fen
                std::istringstream iss(line);
                std::vector<std::string> words(std::istream_iterator<std::string>{iss},
                                               std::istream_iterator<std::string>());
                if (words[0] == "pos")
                {
                    if(currentHash != 0){
                        book[currentHash] = currentMoves;
                        currentMoves.clear();
                    }
                    //get fen
                    std::string fen = words[1] + " " + words[2] + " " + words[3] + " " + words[4] + " " + words[5] + " " + words[6];
                    pos.set_fen(fen);
                    currentHash = zobristHasher->calculateShortFenHash(pos);
                    if (first)
                    {
                        first = false;
                        std::cout << "first fen: " << fen << std::endl;
                        std::cout << "first hash: " << currentHash << std::endl;
                    }
                }
                else
                {
                    //get move
                    libchess::Move move = pos.parse_move(words[0]);
                    //get num games
                    int numGames = std::stoi(words[1]);

                    MoveData moveData(move, numGames);

                    currentMoves.push_back(moveData);
                }
                
            }

            book[currentHash] = currentMoves;
            currentMoves.clear();



            std::cout << "done parsing book" << " num positions: " << book.size() << std::endl;

            //close file
            bookStream.close();
        }

        void printBook(){
            for (auto const& x : book)
            {
                std::cout << "hash: " << x.first << std::endl;
                for (auto const& m : x.second)
                {
                    std::cout << "move: " << m.move << " numGames: " << m.numGames << std::endl;
                }
                break;
            }
        }
    private:
        std::string bookFile;
        //map of positions to set of moves
        std::map<uint64_t, std::vector<MoveData>> book;
        PositionHasher *zobristHasher;


};
#endif // OPENING_BOOK_HPP