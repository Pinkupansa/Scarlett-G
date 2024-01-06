#ifndef TRAINER_HPP
#define TRAINER_HPP
#include <fstream>
#include "EAL.hpp"
#include "scarlett_core.hpp"

class ScarlettTrainer : public ProblemWrapper
{
public:
    void train();
    void testDefault();
    double evaluate(int *chromosome);
    void visualize(int *chromosome, SDL_Window *window, SDL_Renderer *renderer);

private:
    std::ifstream gamesFile;
    void convertChromosomeToIndividual(int *chromosome, ScarlettCore *individual);
    void convertChromosomeToWeights(int *chromosome, int *weights);
    int grayCodeToInt(int *chromosomeSlice, int size);
    // int chromosomeSize = 5 * 10 + 15 * 6;
    //  int chromosomeSize = 5 * 10;
    int chromosomeSize = 15 * 6;
    int nGames = 50;
};

#endif // TRAINER_HPP