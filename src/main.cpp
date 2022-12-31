#include <stdio.h>
#include <iostream>
#include <cstring>
#include <vector>
#include "position.hpp"
#include "player.hpp"
#include "human_player.hpp"
#include "simple_mmab.hpp"


int main(int argc, char *argv[]){
    
    Player* player1;
    Player* player2;
    Player* currentPlayer;

    SimpleMMAB* evaluator = new SimpleMMAB(true, 2);

    if(argc >= 3){
        if(strcmp(argv[1], "ai") == 0){

            player1 = new SimpleMMAB(true, 4);
        }
        if(strcmp(argv[1], "human") == 0){

            player1 = new HumanPlayer();
        }
        if(strcmp(argv[2], "ai") == 0){

            player2 = new SimpleMMAB(false, 4);
        }
        if(strcmp(argv[2], "human") == 0){

            player2 = new HumanPlayer();
        }
    }

    libchess::Position pos;
    if(argc >= 4){
        //concatenate argv[3] and argv[4] and argv[5] and argv[6] and argv[7] and argv[8]
        std::string fen = argv[3];
        for(int i = 4; i < argc; i++){
            fen += " ";
            fen += argv[i];
        }
        std::cout << fen << std::endl;
        pos.set_fen(fen);

    }
    else{
        pos.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }
    std::cout << pos;
    int turn = 0;
    while(true){
        
        if(turn % 2 == 0){
            currentPlayer = player1;
        }else{
            currentPlayer = player2;
        }
        libchess::Move move = currentPlayer->getMove(pos);
        pos.makemove(move);
        
        
        //int score = evaluator->evaluate(cr);
        //std::cout << "Score without calculation: " << score << std::endl;
        std::cout << pos;
        if(pos.is_checkmate()){
            std::cout << "Checkmate!" << std::endl;
            if(turn % 2 == 0){
                std::cout << "White wins!" << std::endl;
            }else{
                std::cout << "Black wins!" << std::endl;
            }
            break;
        }
        if(pos.is_stalemate()){
            std::cout << "Stalemate!" << std::endl;
            break;
        }
        if(pos.is_draw()){
            std::cout << "Draw!" << std::endl;
            break;
        }
        
        turn++;
        
    }

    delete player1;
    delete player2;
    return 0;
}