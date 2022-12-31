#include "human_player.hpp"
#include <iostream>


libchess::Move HumanPlayer::getMove(libchess::Position pos){
    std::string move;
    std::cout << "Enter a move: ";
    std::cin >> move;
    libchess::Move m;
    while(true){
        try{
            if(move == "undo"){

            }
            m = pos.parse_move(move);
            break;
        }
        catch(const std::exception& e){
            if(move == "undo"){
                pos.undomove();
                pos.undomove();
                std::cout << pos;
                std::cout << "Enter a move: ";
                std::cin >> move;
                continue;
            }
            std::cout << "Enter a move: ";
            std::cin >> move;
            continue;
        }
    }
    
    return m;
}