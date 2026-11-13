#ifndef DE_H
#define DE_H

#include "Optimization.h"
#include <cstdlib>
#include <ctime>
#include <limits>

class DE : public Optimization {
private:
    float random01() {
        return static_cast<float>(rand()) /
               (static_cast<float>(RAND_MAX) + 1.0f);
    }

    int randomInt(int min, int max) {
        return min + rand() % (max - min + 1);
    }

    void initializePopulation(float* population) {
        for (int i = 0; i < populationSize; ++i) {
            for (int d = 0; d < dimensions; ++d) {
                population[i * dimensions + d] =
                    lowerBound[d] + random01() * (upperBound[d] - lowerBound[d]);
            }
        }
    }

public:
    DE(float* upperBound, float* lowerBound, int dimensions,
       int populationSize, int maxIterations,
       float (*function)(float*, int))
        : Optimization(upperBound, lowerBound, dimensions,
                       populationSize, maxIterations, function) {
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    float optimize(float* bestSolution) {
        const int N = populationSize;
        const int D = dimensions;

        if (N < 4 || D <= 0 || maxIterations <= 0) {
            result = std::numeric_limits<float>::infinity();
            return result;
        }

        const float mutationRate = 0.7f;
        const float crossoverRate = 0.5f;

        float* population = new float[N * D];
        float* trial = new float[D];
        float* mutant = new float[D];

        initializePopulation(population);

        int bestIndex = 0;
        result = function(population, D);

        for (int i = 1; i < N; ++i) {
            float value = function(population + i * D, D);
            if (value < result) {
                result = value;
                bestIndex = i;
            }
        }

        // Keep a copy of the best-ever individual, not just the last generation's best.
        float* bestEver = new float[D];
        for (int d = 0; d < D; ++d)
            bestEver[d] = population[bestIndex * D + d];

        for (int iteration = 0; iteration < maxIterations; ++iteration) {
            for (int j = 0; j < N; ++j) {
                int a, b, c;

                do { a = randomInt(0, N - 1); } while (a == j);
                do { b = randomInt(0, N - 1); } while (b == j || b == a);
                do { c = randomInt(0, N - 1); } while (c == j || c == a || c == b);

                for (int d = 0; d < D; ++d) {
                    mutant[d] = population[a * D + d] +
                        mutationRate * (population[b * D + d] - population[c * D + d]);

                    if (mutant[d] < lowerBound[d]) mutant[d] = lowerBound[d];
                    if (mutant[d] > upperBound[d]) mutant[d] = upperBound[d];
                }

                int forcedDimension = randomInt(0, D - 1);
                for (int d = 0; d < D; ++d) {
                    if (random01() < crossoverRate || d == forcedDimension)
                        trial[d] = mutant[d];
                    else
                        trial[d] = population[j * D + d];
                }

                float trialFitness = function(trial, D);
                float currentFitness = function(population + j * D, D);

                if (trialFitness < currentFitness) {
                    for (int d = 0; d < D; ++d)
                        population[j * D + d] = trial[d];

                    if (trialFitness < result) {
                        result = trialFitness;
                        for (int d = 0; d < D; ++d)
                            bestEver[d] = trial[d];
                    }
                }
            }
        }

        for (int d = 0; d < D; ++d)
            bestSolution[d] = bestEver[d];

        delete[] population;
        delete[] trial;
        delete[] mutant;
        delete[] bestEver;

        return result;
    }
};

#endif
