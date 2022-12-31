#ifndef HUMAN_PLAYER_HPP
#define HUMAN_PLAYER_HPP

#include "player.hpp"

class HumanPlayer : public Player
{
    public:
        libchess::Move getMove(libchess::Position pos);
};

#endif // HUMAN_PLAYER_HPP