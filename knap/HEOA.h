#ifndef HEOA_H
#define HEOA_H

#include "Optimization.h"
#include <cstdlib>
#include <cmath>
#include <ctime>

class HEOA : public Optimization {
private:
    float random01() {
        return (static_cast<float>(rand()) + 1.0f) /
               (static_cast<float>(RAND_MAX) + 2.0f);
    }

    float randomNormal() {
        float u1 = random01(), u2 = random01();
        return sqrtf(-2.0f * logf(u1)) *
               cosf(2.0f * 3.14159265358979323846f * u2);
    }

    void levyFlight(float* step) {
        for (int d = 0; d < dimensions; ++d) {
            float u = randomNormal(), v = randomNormal();
            if (fabsf(v) < 1e-6f) v = 1e-6f;
            step[d] = u / powf(fabsf(v), 1.0f / 1.5f);
        }
    }

    void initialize(float* X) {
        float* chaos = new float[dimensions];
        for (int d = 0; d < dimensions; ++d) chaos[d] = random01();

        for (int i = 0; i < populationSize; ++i) {
            for (int d = 0; d < dimensions; ++d) {
                chaos[d] = 4.0f * chaos[d] * (1.0f - chaos[d]);
                X[i * dimensions + d] =
                    lowerBound[d] + (upperBound[d] - lowerBound[d]) * chaos[d];
            }
        }
        delete[] chaos;
    }

    void sortPopulation(float* X, float* fitness) {
        for (int i = 0; i < populationSize - 1; ++i) {
            int best = i;
            for (int j = i + 1; j < populationSize; ++j)
                if (fitness[j] < fitness[best]) best = j;

            if (best != i) {
                std::swap(fitness[i], fitness[best]);
                for (int d = 0; d < dimensions; ++d)
                    std::swap(X[i * dimensions + d], X[best * dimensions + d]);
            }
        }
    }

public:
    HEOA(float* upperBound, float* lowerBound, int dimensions,
         int populationSize, int maxIterations,
         float (*function)(float*, int))
        : Optimization(upperBound, lowerBound, dimensions,
                       populationSize, maxIterations, function) {
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    float optimize(float* bestSolution) {
        const int N = populationSize, D = dimensions, Max_iter = maxIterations;
        if (N <= 0 || D <= 0 || Max_iter <= 0) return result;

        const int LNNumber = static_cast<int>(0.2f * N);
        const int ENNumber = static_cast<int>(0.3f * N);
        const int FNNumber = static_cast<int>(0.3f * N);

        const float f_jump = 0.1f, b = 0.05f, A = 0.5f, w = 0.9f;

        float* X = new float[N * D];
        float* X_new = new float[N * D];
        float* fitness = new float[N];
        float* fitness_new = new float[N];
        float* GBestX = new float[D];
        float* levy = new float[D];

        initialize(X);
        for (int i = 0; i < N; ++i)
            fitness[i] = function(X + i * D, D);

        sortPopulation(X, fitness);
        for (int d = 0; d < D; ++d) GBestX[d] = X[d];
        result = fitness[0];

        for (int i = 0; i < Max_iter; ++i) {
            float R = random01();
            for (int k = 0; k < N * D; ++k) X_new[k] = X[k];

            for (int j = 0; j < N; ++j) {
                float* current = X + j * D;
                float* next = X_new + j * D;

                if (i <= Max_iter / 4) {
                    float mean_x = 0.0f;
                    for (int d = 0; d < D; ++d) mean_x += current[d];
                    mean_x /= D;
                    levyFlight(levy);
                    int floor_val = static_cast<int>(floorf(random01() / f_jump));

                    for (int d = 0; d < D; ++d)
                        next[d] = GBestX[d] * (1.0f - static_cast<float>(i) / Max_iter)
                                  + (mean_x - GBestX[d]) * floor_val * f_jump
                                  + b * levy[d];
                } else if (j < LNNumber) {
                    if (R < A) {
                        float denominator = random01() * Max_iter;
                        float exponent = (-static_cast<float>(i) * randomNormal()) / denominator;
                        float factor = w * expf(exponent);
                        for (int d = 0; d < D; ++d) next[d] = factor * current[d];
                    } else {
                        float noise = randomNormal();
                        for (int d = 0; d < D; ++d) next[d] = w * current[d] + noise;
                    }
                } else if (j < LNNumber + ENNumber) {
                    float noise = randomNormal();
                    float denominator = static_cast<float>((j + 1) * (j + 1));
                    for (int d = 0; d < D; ++d) {
                        float exponent = (X[(N - 1) * D + d] - current[d]) / denominator;
                        next[d] = noise * expf(exponent);
                    }
                } else if (j < LNNumber + ENNumber + FNNumber) {
                    for (int d = 0; d < D; ++d)
                        next[d] = current[d] + w * random01() * (X[d] - current[d]);
                } else {
                    float noise = randomNormal();
                    for (int d = 0; d < D; ++d)
                        next[d] = GBestX[d] + (GBestX[d] - current[d]) * noise;
                }
            }

            for (int j = 0; j < N; ++j) {
                for (int d = 0; d < D; ++d) {
                    int idx = j * D + d;
                    if (!std::isfinite(X_new[idx])) X_new[idx] = 0.0f;
                    if (X_new[idx] < lowerBound[d]) X_new[idx] = lowerBound[d];
                    if (X_new[idx] > upperBound[d]) X_new[idx] = upperBound[d];
                }
                fitness_new[j] = function(X_new + j * D, D);
            }

            for (int j = 0; j < N; ++j) {
                if (fitness_new[j] < fitness[j]) {
                    fitness[j] = fitness_new[j];
                    for (int d = 0; d < D; ++d) X[j * D + d] = X_new[j * D + d];
                }
            }

            sortPopulation(X, fitness);
            if (fitness[0] < result) {
                result = fitness[0];
                for (int d = 0; d < D; ++d) GBestX[d] = X[d];
            }
        }

        for (int d = 0; d < D; ++d) bestSolution[d] = GBestX[d];

        delete[] X; delete[] X_new; delete[] fitness; delete[] fitness_new;
        delete[] GBestX; delete[] levy;
        return result;
    }
};

#endif
