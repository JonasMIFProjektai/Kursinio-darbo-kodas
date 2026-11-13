#ifndef DOLPHIN_OPTIMIZATION_H
#define DOLPHIN_OPTIMIZATION_H

#include "Optimization.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

class Dolphin : public Optimization {
private:
    float* bestPosition;

    float clampToBounds(float value, int d) const {
        return std::max(lowerBound[d], std::min(value, upperBound[d]));
    }

public:
    Dolphin(float* upperBound, float* lowerBound, int dimensions,
            int populationSize, int maxIterations,
            float (*function)(float*, int))
        : Optimization(upperBound, lowerBound, dimensions,
                       populationSize, maxIterations, function),
          bestPosition(nullptr) {
        if (dimensions <= 0 || populationSize < 1 || maxIterations <= 0 ||
            upperBound == nullptr || lowerBound == nullptr || function == nullptr)
            throw std::invalid_argument("Dolphin: invalid parameters.");

        for (int d = 0; d < dimensions; ++d) {
            if (lowerBound[d] > upperBound[d])
                throw std::invalid_argument("Dolphin: lower bound exceeds upper bound.");
        }
        bestPosition = new float[dimensions]{};
    }

    ~Dolphin() override {
        delete[] bestPosition;
    }

    Dolphin(const Dolphin&) = delete;
    Dolphin& operator=(const Dolphin&) = delete;

    float optimize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> unit(0.0f, 1.0f);

        float* dolphins = new float[populationSize * dimensions];
        float* trial = new float[dimensions];
        float bestScore = std::numeric_limits<float>::infinity();

        for (int i = 0; i < populationSize; ++i) {
            for (int d = 0; d < dimensions; ++d) {
                std::uniform_real_distribution<float> dist(lowerBound[d], upperBound[d]);
                dolphins[i * dimensions + d] = dist(gen);
            }
        }

        for (int i = 0; i < populationSize; ++i) {
            float* dolphin = dolphins + i * dimensions;
            float score = function(dolphin, dimensions);
            if (score < bestScore) {
                bestScore = score;
                std::copy(dolphin, dolphin + dimensions, bestPosition);
            }
        }

        for (int iter = 0; iter < maxIterations; ++iter) {
            float progress = static_cast<float>(iter) / maxIterations;
            float radiusFactor = 1.0f - progress;

            for (int i = 0; i < populationSize; ++i) {
                float* dolphin = dolphins + i * dimensions;

                for (int d = 0; d < dimensions; ++d) {
                    float range = upperBound[d] - lowerBound[d];
                    float exploration =
                        (2.0f * unit(gen) - 1.0f) * range * 0.10f * radiusFactor;
                    float attraction = unit(gen) * (bestPosition[d] - dolphin[d]);

                    float candidate;
                    if (unit(gen) < 0.10f)
                        candidate = lowerBound[d] + unit(gen) * range;
                    else
                        candidate = dolphin[d] + attraction + exploration;

                    trial[d] = clampToBounds(candidate, d);
                }

                float trialScore = function(trial, dimensions);
                float currentScore = function(dolphin, dimensions);

                if (trialScore < currentScore)
                    std::copy(trial, trial + dimensions, dolphin);

                if (trialScore < bestScore) {
                    bestScore = trialScore;
                    std::copy(trial, trial + dimensions, bestPosition);
                }
            }
        }

        result = bestScore;
        delete[] dolphins;
        delete[] trial;
        return result;
    }

    const float* getBestPosition() const {
        return bestPosition;
    }
};

#endif
