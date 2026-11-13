#ifndef WOLF_H
#define WOLF_H

#include "Optimization.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <stdexcept>

class Wolf : public Optimization {
private:
    float* bestPosition;

    float clamp(float value, int d) const {
        return std::max(lowerBound[d], std::min(value, upperBound[d]));
    }

public:
    Wolf(float* upperBound, float* lowerBound, int dimensions,
         int populationSize, int maxIterations,
         float (*function)(float*, int))
        : Optimization(upperBound, lowerBound, dimensions,
                       populationSize, maxIterations, function),
          bestPosition(nullptr) {
        if (dimensions <= 0 || populationSize < 3 || maxIterations <= 0 ||
            !upperBound || !lowerBound || !function)
            throw std::invalid_argument("Invalid GWO parameters");

        bestPosition = new float[dimensions]{};
    }

    ~Wolf() override {
        delete[] bestPosition;
    }

    Wolf(const Wolf&) = delete;
    Wolf& operator=(const Wolf&) = delete;

    float optimize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> random01(0.0f, 1.0f);

        float* wolves = new float[populationSize * dimensions];
        float* alpha = new float[dimensions]{};
        float* beta = new float[dimensions]{};
        float* delta = new float[dimensions]{};

        float alphaScore = std::numeric_limits<float>::infinity();
        float betaScore = std::numeric_limits<float>::infinity();
        float deltaScore = std::numeric_limits<float>::infinity();

        for (int i = 0; i < populationSize; ++i) {
            for (int d = 0; d < dimensions; ++d) {
                std::uniform_real_distribution<float> dist(lowerBound[d], upperBound[d]);
                wolves[i * dimensions + d] = dist(gen);
            }
        }

        auto updateLeaders = [&]() {
            alphaScore = betaScore = deltaScore =
                std::numeric_limits<float>::infinity();

            for (int i = 0; i < populationSize; ++i) {
                float* wolf = wolves + i * dimensions;
                float score = function(wolf, dimensions);

                if (score < alphaScore) {
                    deltaScore = betaScore;
                    std::copy(beta, beta + dimensions, delta);
                    betaScore = alphaScore;
                    std::copy(alpha, alpha + dimensions, beta);
                    alphaScore = score;
                    std::copy(wolf, wolf + dimensions, alpha);
                } else if (score < betaScore) {
                    deltaScore = betaScore;
                    std::copy(beta, beta + dimensions, delta);
                    betaScore = score;
                    std::copy(wolf, wolf + dimensions, beta);
                } else if (score < deltaScore) {
                    deltaScore = score;
                    std::copy(wolf, wolf + dimensions, delta);
                }
            }
        };

        updateLeaders();
        std::copy(alpha, alpha + dimensions, bestPosition);
        result = alphaScore;

        for (int iter = 0; iter < maxIterations; ++iter) {
            float a = 2.0f - 2.0f * static_cast<float>(iter) / maxIterations;

            for (int i = 0; i < populationSize; ++i) {
                float* wolf = wolves + i * dimensions;

                for (int d = 0; d < dimensions; ++d) {
                    float r1 = random01(gen), r2 = random01(gen);
                    float A1 = 2.0f * a * r1 - a, C1 = 2.0f * r2;
                    float X1 = alpha[d] - A1 * std::fabs(C1 * alpha[d] - wolf[d]);

                    r1 = random01(gen); r2 = random01(gen);
                    float A2 = 2.0f * a * r1 - a, C2 = 2.0f * r2;
                    float X2 = beta[d] - A2 * std::fabs(C2 * beta[d] - wolf[d]);

                    r1 = random01(gen); r2 = random01(gen);
                    float A3 = 2.0f * a * r1 - a, C3 = 2.0f * r2;
                    float X3 = delta[d] - A3 * std::fabs(C3 * delta[d] - wolf[d]);

                    wolf[d] = clamp((X1 + X2 + X3) / 3.0f, d);
                }
            }

            updateLeaders();

            if (alphaScore < result) {
                result = alphaScore;
                std::copy(alpha, alpha + dimensions, bestPosition);
            }
        }

        delete[] wolves;
        delete[] alpha;
        delete[] beta;
        delete[] delta;
        return result;
    }

    const float* getBestPosition() const {
        return bestPosition;
    }
};

#endif
