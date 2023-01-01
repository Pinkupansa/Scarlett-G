#include "pgn_to_coordinates_converter.hpp"

#include <iostream>
#include <regex>
#include <string>
#include <vector>

PgnToCoordinatesConverter::PgnToCoordinatesConverter(const std::string &pgn_file_path, const std::string &coordinates_file_path)
    : pgn_file_path_(pgn_file_path), coordinates_file_path_(coordinates_file_path)
{
    // open coordinates file
}

void PgnToCoordinatesConverter::convert()
{
    coordinates_file_.open(coordinates_file_path_);
    read_pgn_file();
    coordinates_file_.close();
}

void PgnToCoordinatesConverter::read_pgn_file()
{
    std::ifstream pgn_file(pgn_file_path_);
    std::string line;
    bool gameBegun = false;
    int i = 0;
    while (std::getline(pgn_file, line))
    {
        if (!gameBegun)
        {
            if (line.find("1. ") != std::string::npos)
            {
                gameBegun = true;
                std::cout << "Game " << i << std::endl;
                i++;
            }
        }

        if (gameBegun)
        {

            // split line by space
            std::regex re(" ");
            std::sregex_token_iterator first{line.begin(), line.end(), re, -1}, last;
            std::vector<std::string> tokens{first, last};
            for (const auto &token : tokens)
            {

                if (!(token.find("1-0") == std::string::npos && token.find("0-1") == std::string::npos && token.find("1/2-1/2") == std::string::npos))
                {
                    gameBegun = false;
                    break;
                }
                if (token.find(".") == std::string::npos)
                {
                    moves_.push_back(token);
                }
            }
            if (!gameBegun)
            {

                convert_moves_to_coordinates();
                write_coordinates_file();
                moves_.clear();
                coordinates_.clear();
            }
        }
    }
}

libchess::Square PgnToCoordinatesConverter::stringToSquare(std::string s)
{
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return libchess::Square(file, rank);
}

void PgnToCoordinatesConverter::convert_moves_to_coordinates()
{
    libchess::Position pos;
    pos.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    for (const auto &move : moves_)
    {
        std::string moveString;
        if (move.find("O-O-O") != std::string::npos)
        {
            if (pos.turn() == libchess::Side::White)
            {
                moveString = "e1a1";
            }
            else
            {
                moveString = "e8a8";
            }
        }
        else if (move.find("O-O") != std::string::npos)
        {
            if (pos.turn() == libchess::Side::White)
            {
                moveString = "e1h1";
            }
            else
            {
                moveString = "e8h8";
            }
        }
        else
        {
            auto legal_moves = pos.legal_moves();
            // remove x and + and #
            std::string move_without_symbols = move;
            move_without_symbols.erase(std::remove(move_without_symbols.begin(), move_without_symbols.end(), 'x'), move_without_symbols.end());
            move_without_symbols.erase(std::remove(move_without_symbols.begin(), move_without_symbols.end(), '+'), move_without_symbols.end());
            move_without_symbols.erase(std::remove(move_without_symbols.begin(), move_without_symbols.end(), '#'), move_without_symbols.end());
            // remove substrings "=Q", "=R", "=B", "=N"
            int index = move_without_symbols.find("=Q");
            if (index != std::string::npos)
            {
                move_without_symbols.erase(index, 2);
            }
            index = move_without_symbols.find("=R");
            if (index != std::string::npos)
            {
                move_without_symbols.erase(index, 2);
            }
            index = move_without_symbols.find("=B");
            if (index != std::string::npos)
            {
                move_without_symbols.erase(index, 2);
            }
            index = move_without_symbols.find("=N");
            if (index != std::string::npos)
            {
                move_without_symbols.erase(index, 2);
            }

            libchess::Piece piece = libchess::Piece::None;
            if (move_without_symbols.length() == 2)
            {
                // pawn push
                piece = libchess::Piece::Pawn;
            }
            else
            {
                if (move_without_symbols[0] == 'N')
                {
                    piece = libchess::Piece::Knight;
                }
                else if (move_without_symbols[0] == 'B')
                {
                    piece = libchess::Piece::Bishop;
                }
                else if (move_without_symbols[0] == 'R')
                {
                    piece = libchess::Piece::Rook;
                }
                else if (move_without_symbols[0] == 'Q')
                {
                    piece = libchess::Piece::Queen;
                }
                else if (move_without_symbols[0] == 'K')
                {
                    piece = libchess::Piece::King;
                }
                else
                {
                    // pawn eats
                    piece = libchess::Piece::Pawn;
                }
            }

            // get two last characters to make toSquare
            std::string to = move_without_symbols.substr(move_without_symbols.length() - 2);
            // std::cout << move_without_symbols << std::endl;
            // std::cout << to << std::endl;
            libchess::Square toSquare = stringToSquare(to);
            libchess::Square fromSquare;
            // remove piece symbol
            if (piece != libchess::Piece::None && piece != libchess::Piece::Pawn)
            {
                move_without_symbols.erase(0, 1);
            }

            if (piece == libchess::Piece::Pawn && move_without_symbols.length() == 3)
            {
                int fromRank = toSquare.rank() + (pos.turn() == libchess::Side::White ? -1 : 1);
                int fromFile = move_without_symbols[0] - 'a';
                fromSquare = libchess::Square(fromFile, fromRank);
            }
            if (piece == libchess::Piece::Pawn && move_without_symbols.length() == 2)
            {
                int pawnPushDirection = pos.turn() == libchess::Side::White ? 1 : -1;

                if (pos.piece_on(libchess::Square(toSquare.file(), toSquare.rank() - pawnPushDirection)) != libchess::Piece::None)
                {
                    fromSquare = libchess::Square(toSquare.file(), toSquare.rank() - pawnPushDirection);
                }
                else
                {
                    fromSquare = libchess::Square(toSquare.file(), toSquare.rank() - 2 * pawnPushDirection);
                }
            }
            else
            {

                libchess::Bitboard potentialFromSquares = pos.attackers(toSquare, pos.turn()) & pos.pieces(pos.turn(), piece);
                std::vector<libchess::Square> potentialFromSquaresVector;

                for (auto square : potentialFromSquares)
                {
                    // check if piece is pinned
                    if (pos.pinned() & libchess::Bitboard(square))
                    {

                        try
                        {

                            pos.makemove((std::string)square + (std::string)toSquare);
                            pos.undomove();
                        }
                        catch (const std::exception &e)
                        {
                            // std::cout << "pinned" << std::endl;
                            //  the move is illegal
                            continue;
                        }
                    }
                    potentialFromSquaresVector.push_back(square);
                }
                // print potentialFromSquares

                if (potentialFromSquaresVector.size() == 1)
                {
                    // no ambiguity - only one piece of this type can move to toSquare
                    // std::cout << "???" << std::endl;
                    fromSquare = potentialFromSquaresVector[0];
                }
                else
                {
                    // the length should be 3
                    char disambiguation = move_without_symbols[0]; // a, b, c, d, e, f, g, h or 1, 2, 3, 4, 5, 6, 7, 8

                    if (disambiguation >= 'a' && disambiguation <= 'h')
                    {
                        // disambiguation by file
                        int file = disambiguation - 'a';
                        for (auto square : potentialFromSquaresVector)
                        {
                            if (square.file() == file)
                            {
                                fromSquare = square;
                                break;
                            }
                        }
                    }
                    else
                    {
                        // parse disambiguation to int
                        int rank = disambiguation - '1';
                        for (auto square : potentialFromSquaresVector)
                        {
                            if (square.rank() == rank)
                            {
                                fromSquare = square;
                                break;
                            }
                        }
                    }
                }
            }

            // make move
            moveString = (std::string)fromSquare + (std::string)toSquare;
        }

        try
        {
            pos.makemove(moveString);
            coordinates_.push_back(moveString);
            // std::cout << moveString << std::endl;
        }
        catch (const std::exception &e)
        {
            std::string promotion = move.substr(move.length() - 1, 1);
            // lower
            if (!(promotion[0] >= 'A' && promotion[0] <= 'Z'))
            {
                promotion = move.substr(move.length() - 2, 1);
            }

            promotion[0] = promotion[0] + 32;
            moveString = moveString + promotion;
            try
            {
                pos.makemove(moveString);
                coordinates_.push_back(moveString);
            }
            catch (const std::exception &e)
            {
                std::cout << "Error: " << e.what() << std::endl;
                std::cout << pos << std::endl;
                std::cout << "Move: " << move << std::endl;
                std::cout << moveString << std::endl;
                exit(1);
            }
        }
    }
}

void PgnToCoordinatesConverter::write_coordinates_file()
{

    for (const auto &move : coordinates_)
    {
        coordinates_file_ << move << " ";
    }
    coordinates_file_ << std::endl;
}

// add main

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: ./pgn_to_coordinates_converter <pgn_file_path> <coordinates_file_path>" << std::endl;
        exit(1);
    }
    PgnToCoordinatesConverter pgn_to_coordinates_converter(argv[1], argv[2]);
    pgn_to_coordinates_converter.convert();
}