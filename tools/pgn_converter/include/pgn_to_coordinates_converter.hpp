// Takes in a pgn file and converts it to a file of moves noted by starting and ending coordinates
#ifndef PGN_TO_COORDINATES_CONVERTER_HPP
#define PGN_TO_COORDINATES_CONVERTER_HPP

#include <string>
#include <vector>
#include "position.hpp"

#include <fstream>
class PgnToCoordinatesConverter
{
public:
    PgnToCoordinatesConverter(const std::string &pgn_file_path, const std::string &coordinates_file_path);
    void convert();

private:
    std::string pgn_file_path_;
    std::string coordinates_file_path_;

    // file stream
    std::ofstream coordinates_file_;

    std::vector<std::string> moves_;
    std::vector<std::string> coordinates_;

    void read_pgn_file();
    void convert_moves_to_coordinates();
    void write_coordinates_file();
    libchess::Square stringToSquare(std::string s);
};

#endif // PGN_TO_COORDINATES_CONVERTER_HPP
