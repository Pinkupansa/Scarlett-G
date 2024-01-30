#include <stdio.h>
#include <iostream>
#include <cstring>
#include <vector>
#include "position.hpp"
#include "player.hpp"
#include "human_player.hpp"
#include "scarlett_core.hpp"
#include "scarlett_trainer.hpp"
#include <random>

int main(int argc, char *argv[])
{
    //seed random
    srand(time(NULL));
    if (argc == 2)
    {
        if (strcmp(argv[1], "train") == 0)
        {
            ScarlettTrainer *trainer = new ScarlettTrainer();
            trainer->train();
            delete trainer;
        }
        if (strcmp(argv[1], "test") == 0)
        {
            ScarlettTrainer *trainer = new ScarlettTrainer();
            trainer->testDefault();
            delete trainer;
        }
        return 0;
    }
    Player *player1;
    Player *player2;
    Player *currentPlayer;

    if (argc >= 3)
    {
        if (strcmp(argv[1], "ai") == 0)
        {

            player1 = new ScarlettCore(1, 2);
        }
        if (strcmp(argv[1], "human") == 0)
        {

            player1 = new HumanPlayer();
        }
        if (strcmp(argv[2], "ai") == 0)
        {

            player2 = new ScarlettCore(1, 8);
        }
        if (strcmp(argv[2], "human") == 0)
        {

            player2 = new HumanPlayer();
        }
    }
    libchess::Position pos;
    if (argc >= 4)
    {
        // concatenate argv[3] and argv[4] and argv[5] and argv[6] and argv[7] and argv[8]
        std::string fen = argv[3];
        for (int i = 4; i < argc; i++)
        {
            fen += " ";
            fen += argv[i];
        }
        std::cout << fen << std::endl;
        pos.set_fen(fen);
    }
    else
    {
        pos.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
    }

    std::cout << pos;
    while (true)
    {

        if (pos.turn() == 0)
        {
            currentPlayer = player1;
        }
        else
        {
            currentPlayer = player2;
        }
        libchess::Move move = currentPlayer->getMove(pos);
        pos.makemove(move);

        std::cout << pos << std::endl;
        std::cout << pos.get_fen() << std::endl;
    
        if (pos.is_checkmate())
        {
            std::cout << "Checkmate!" << std::endl;
            if (pos.turn() == 0)
            {
                std::cout << "White wins!" << std::endl;
            }
            else
            {
                std::cout << "Black wins!" << std::endl;
            }
            break;
        }
        if (pos.is_stalemate())
        {
            std::cout << "Stalemate!" << std::endl;
            break;
        }
        if (pos.is_draw())
        {
            std::cout << "Draw!" << std::endl;
            break;
        }
    }

    delete player1;
    delete player2;
    return 0;
}