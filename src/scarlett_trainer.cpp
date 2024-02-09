#include "scarlett_trainer.hpp"
#include <iostream>
#include <random>

#define USE_GRAY_CODE 0

int convertBinToDecimal(int *chromosome, int size)
{
    int result = 0;
    int power = 1;
    for (int i = 0; i < size; i++)
    {
        result += chromosome[i] * power;
        power *= 2;
    }
    return result;
}
void evolutionLoop(EvolutionInterface *evolutionInterface)
{
    SDL_Event ev;
    int nGens = 2000;
    while (true)
    {
        if (nGens-- == 0)
            return;
        evolutionInterface->step();
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
            {
                return;
            }
        }
        SDL_Delay(0);
    }
}

void ScarlettTrainer::testDefault()
{
    int defaultWeights[20] = {300, 300, 500, 900, 1000, 20, 30, 10, 7, 7, 28, 51, 27, 50, 30, 6, 40, 20, 5, 50};
    ScarlettCore *individual = new ScarlettCore();
    individual->setWeights(defaultWeights);

    // convert default weights to gray code
    int grayCode[5 * 10 + 15 * 6];
    int binary[5 * 10 + 15 * 6];
    // for the first 5 weights, we have 10 bits each, for the last 15 weights, we have 6 bits each
    for (int i = 0; i < 5; i++)
    {
        // convert defaultWeights[i] to binary
        for (int j = 0; j < 10; j++)
        {
            binary[i * 10 + j] = (defaultWeights[i] >> j) & 1;
            std::cout << binary[i * 10 + j];
        }
        std::cout << std::endl;
    }

    for (int i = 0; i < 15; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            binary[50 + i * 6 + j] = (defaultWeights[5 + i] >> j) & 1;
        }
    }
#if USE_GRAY_CODE
    for (int i = 0; i < 5; i++)
    {
        grayCode[i * 10] = binary[i * 10];
        for (int j = 1; j < 10; j++)
        {
            grayCode[i * 10 + j] = binary[i * 10 + j] ^ binary[i * 10 + j - 1];
        }
    }

    for (int i = 0; i < 15; i++)
    {
        grayCode[50 + i * 6] = binary[50 + i * 6];
        for (int j = 1; j < 6; j++)
        {
            grayCode[50 + i * 6 + j] = binary[50 + i * 6 + j] ^ binary[50 + i * 6 + j - 1];
        }
    }
#endif
    // print back to decimal
    int decimal[20];
#if USE_GRAY_CODE
    for (int i = 0; i < 5; i++)
    {
        decimal[i] = grayCodeToInt(grayCode + i * 10, 10);
    }
    for (int i = 0; i < 15; i++)
    {
        decimal[5 + i] = grayCodeToInt(grayCode + 50 + i * 6, 6);
    }
#else
    for (int i = 0; i < 5; i++)
    {
        decimal[i] = convertBinToDecimal(binary + i * 10, 10);
    }
    for (int i = 0; i < 15; i++)
    {
        decimal[5 + i] = convertBinToDecimal(binary + 50 + i * 6, 6);
    }
#endif
    for (int i = 0; i < 20; i++)
    {
        std::cout << decimal[i] << " ";
    }

    // evaluate gray code
    std::cout << "Gray code fitness: " << evaluate(grayCode) << std::endl;
}

void ScarlettTrainer::train()
{
    // set random seed
    srand(time(NULL));
    UniformPseudoBooleanInitializer initializer;
    StandardBitMutation mutation(0.06);
    // int segmentSizes[20] = {10, 10, 10, 10, 10, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
    int segmentSizes[15] = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
    // int segmentSizes[5] = {10, 10, 10, 10, 10};
    CustomSegmentCrossover crossover(segmentSizes, 15);
    // UniformCrossover crossover;
    FitnessProportionalParentSelection parentSelection(0.5);

    SDL_Window *window = SDL_CreateWindow("Scarlett", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    ElitistEA elitistEA(2, 1, chromosomeSize, &mutation, &initializer, &crossover, &parentSelection);
    EvolutionInterface interface = EvolutionInterface(&elitistEA, this, window, renderer, 1);

    evolutionLoop(&interface);

    // destroy window
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

double ScarlettTrainer::evaluate(int *chromosome)
{
    std::cout << "New individual " << std::endl;
    libchess::Position pos;
    gamesFile.open("../CCRL_games_converted.txt");

    ScarlettCore individual;
    convertChromosomeToIndividual(chromosome, &individual);
    double fitness = 0;
    int firstGame = 0;
    // std::cout << "First game: " << firstGame << std::endl;
    for (int i = firstGame; i < firstGame + nGames; i++)
    {

        // std::cout << "Game " << i << std::endl;
        std::string gameString;
        std::getline(gamesFile, gameString);
        // split gameString into moves
        std::vector<std::string> moves;
        std::string delimiter = " ";
        size_t index = 0;
        std::string token;
        // std::cout << gameString << std::endl;

        while ((index = gameString.find(delimiter)) != std::string::npos)
        {
            token = gameString.substr(0, index);
            moves.push_back(token);
            gameString.erase(0, index + delimiter.length());
        }
        pos.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        int winner = 1;
        if (moves[0] == "0") // white wins
        {
            winner = 0;
        }
        else if (moves[0] == "1") // black wins
        {
            winner = 1;
        }
        else if (moves[0] == "-1") // draw
        {
            winner = -1;
        }

        for (int j = 1; j < moves.size(); j++)
        {
            std::string move = moves[j];
            if (j >= 8 && ((int)pos.turn() == winner || winner == -1))
            {
                libchess::Move bestMove = individual.getMove(pos);
                if (move == (std::string)bestMove)
                {

                    fitness += 1;
                }
            }

            pos.makemove(move);
        }
    }
    gamesFile.close();
    return fitness * fitness;
}

void ScarlettTrainer::visualize(int *chromosome, SDL_Window *window, SDL_Renderer *renderer)
{
    // print piece values
    int weights[20];
    convertChromosomeToWeights(chromosome, weights);

    std::cout << "Piece values: ";
    for (int i = 0; i < 5; i++)
    {
        std::cout << weights[i] << " ";
    }

    // print the rest of the weights
    std::cout
        << std::endl
        << "Other weights: ";
    for (int i = 5; i < 20; i++)
    {
        std::cout << weights[i] << " ";
    }
}

void ScarlettTrainer::convertChromosomeToWeights(int *chromosome, int *weights)
{
#if USE_GRAY_CODE
    for (int i = 0; i < 5; i++)
    {
        weights[i] = grayCodeToInt(chromosome + i * 10, 10);
    }
    for (int i = 0; i < 15; i++)
    {
        weights[5 + i] = grayCodeToInt(chromosome + 50 + i * 6, 6);
    }
#else
    /*for (int i = 0; i < 5; i++)
    {
        // convert chromosome slice to decimal
        weights[i] = convertBinToDecimal(chromosome + i * 10, 10);
    }*/
    weights[0] = 100;
    weights[1] = 305;
    weights[2] = 333;
    weights[3] = 563;
    weights[4] = 950;
    for (int i = 0; i < 15; i++)
    {
        // weights[5 + i] = convertBinToDecimal(chromosome + 50 + i * 6, 6);
        weights[5 + i] = convertBinToDecimal(chromosome + i * 6, 6);
    }
#endif
}
int ScarlettTrainer::grayCodeToInt(int *chromosomeSlice, int size)
{
    int binary[size];

    // convert gray code to binary
    binary[0] = chromosomeSlice[0];
    for (int i = 1; i < size; i++)
    {
        binary[i] = binary[i - 1] ^ chromosomeSlice[i];
    }

    // convert binary to decimal
    int result = 0;
    int power = 1;
    for (int i = 0; i < size; i++)
    {
        result += binary[i] * power;
        power *= 2;
    }

    return result;
}
void ScarlettTrainer::convertChromosomeToIndividual(int *chromosome, ScarlettCore *individual)
{
    int weights[20];
    convertChromosomeToWeights(chromosome, weights);
    individual->setWeights(weights);
}