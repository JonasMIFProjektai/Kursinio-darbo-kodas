#ifndef BHO_H
#define BHO_H

#include "Optimization.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>

class BHO : public Optimization {
private:
    float random01() {
        return static_cast<float>(rand()) /
               (static_cast<float>(RAND_MAX) + 1.0f);
    }

    void initializePopulation(float* stars) {
        for (int i = 0; i < populationSize; ++i)
            for (int d = 0; d < dimensions; ++d)
                stars[i * dimensions + d] =
                    lowerBound[d] + random01() * (upperBound[d] - lowerBound[d]);
    }

    float distance(const float* a, const float* b) {
        float sum = 0.0f;
        for (int d = 0; d < dimensions; ++d) {
            float diff = a[d] - b[d];
            sum += diff * diff;
        }
        return sqrtf(sum);
    }

public:
    BHO(float* upperBound, float* lowerBound, int dimensions,
        int populationSize, int maxIterations,
        float (*function)(float*, int))
        : Optimization(upperBound, lowerBound, dimensions,
                       populationSize, maxIterations, function) {
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    float optimize(float* bestSolution) {
        const int N = populationSize, D = dimensions;
        if (N <= 0 || D <= 0 || maxIterations <= 0) return result;

        float* stars = new float[N * D];
        float* values = new float[N];
        float* fitness = new float[N];
        float* blackHole = new float[D];

        initializePopulation(stars);
        result = std::numeric_limits<float>::infinity();
        for (int i = 0; i < N; ++i) {
            values[i] = function(stars + i * D, D);
            if (values[i] < result) {
                result = values[i];
                for (int d = 0; d < D; ++d) bestSolution[d] = stars[i * D + d];
            }
        }

        for (int iteration = 0; iteration < maxIterations; ++iteration) {
            int blackHoleIndex = 0;
            float blackHoleValue = values[0];

            for (int j = 1; j < N; ++j) {
                if (values[j] < blackHoleValue) {
                    blackHoleValue = values[j];
                    blackHoleIndex = j;
                }
            }

            for (int d = 0; d < D; ++d)
                blackHole[d] = stars[blackHoleIndex * D + d];

            for (int j = 0; j < N; ++j) {
                if (j == blackHoleIndex) continue;
                float r = random01();
                for (int d = 0; d < D; ++d) {
                    int idx = j * D + d;
                    stars[idx] += r * (blackHole[d] - stars[idx]);
                    if (stars[idx] < lowerBound[d]) stars[idx] = lowerBound[d];
                    if (stars[idx] > upperBound[d]) stars[idx] = upperBound[d];
                }
            }

            // Re-evaluate the moved population and update the black hole index.
            for (int j = 0; j < N; ++j) {
                values[j] = function(stars + j * D, D);
                if (values[j] < result) {
                    result = values[j];
                    for (int d = 0; d < D; ++d) bestSolution[d] = stars[j * D + d];
                }
            }

            blackHoleIndex = 0;
            for (int j = 1; j < N; ++j)
                if (values[j] < values[blackHoleIndex]) blackHoleIndex = j;

            for (int d = 0; d < D; ++d)
                blackHole[d] = stars[blackHoleIndex * D + d];

            float minValue = values[0], maxValue = values[0];
            for (int j = 1; j < N; ++j) {
                if (values[j] < minValue) minValue = values[j];
                if (values[j] > maxValue) maxValue = values[j];
            }

            float fitnessSum = 0.0f;
            for (int j = 0; j < N; ++j)
                fitness[j] = maxValue - values[j] + 1e-10f;

            for (int j = 0; j < N; ++j) fitnessSum += fitness[j];

            float radius = fitnessSum > 0.0f
                ? fitness[blackHoleIndex] / fitnessSum : 0.0f;

            for (int j = 0; j < N; ++j) {
                if (j == blackHoleIndex) continue;
                float* star = stars + j * D;
                if (distance(star, blackHole) <= radius) {
                    for (int d = 0; d < D; ++d)
                        star[d] = lowerBound[d] +
                                  random01() * (upperBound[d] - lowerBound[d]);
                    values[j] = function(star, D);
                    if (values[j] < result) {
                        result = values[j];
                        for (int d = 0; d < D; ++d) bestSolution[d] = star[d];
                    }
                }
            }
        }

        delete[] stars; delete[] values; delete[] fitness; delete[] blackHole;
        return result;
    }
};

#endif
