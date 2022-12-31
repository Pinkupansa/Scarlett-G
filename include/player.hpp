#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <position.hpp>
class Player
{
    public:
        virtual libchess::Move getMove(libchess::Position pos) = 0;
};
#endif // PLAYER_HPP