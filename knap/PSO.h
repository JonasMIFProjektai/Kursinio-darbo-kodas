#ifndef PSO_H
#define PSO_H

#include "Optimization.h"
#include "Knapsack.h"
#include <cstdlib>

class ParticleSwarmOptimization : public Optimization {
private:
    float* position;
    float* velocity;
    float* personalBest;
    float* personalBestValue;
    float* globalBest;

    float inertia;
    float cognitive;
    float social;

    float randomFloat() {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    int index(int particle, int dimension) const {
        return particle * dimensions + dimension;
    }

public:
    ParticleSwarmOptimization(
        float* upperBound, float* lowerBound, int dimensions,
        int populationSize, int maxIterations,
        float (*function)(float*, int),
        float inertia = 0.729f,
        float cognitive = 1.49445f,
        float social = 1.49445f)
        : Optimization(upperBound, lowerBound, dimensions,
                       populationSize, maxIterations, function),
          inertia(inertia), cognitive(cognitive), social(social) {

        int totalElements = populationSize * dimensions;
        position = new float[totalElements];
        velocity = new float[totalElements];
        personalBest = new float[totalElements];
        personalBestValue = new float[populationSize];
        globalBest = new float[dimensions]{};
    }

    ~ParticleSwarmOptimization() override {
        delete[] position;
        delete[] velocity;
        delete[] personalBest;
        delete[] personalBestValue;
        delete[] globalBest;
    }

    ParticleSwarmOptimization(const ParticleSwarmOptimization&) = delete;
    ParticleSwarmOptimization& operator=(const ParticleSwarmOptimization&) = delete;

    void optimize() {
        result = std::numeric_limits<float>::infinity();

        for (int i = 0; i < populationSize; ++i) {
            for (int d = 0; d < dimensions; ++d) {
                int idx = index(i, d);
                float range = upperBound[d] - lowerBound[d];

                position[idx] = lowerBound[d] + randomFloat() * range;
                velocity[idx] = (randomFloat() * 0.2f - 0.1f) * range;
                personalBest[idx] = position[idx];
            }

            personalBestValue[i] =
                function(position + i * dimensions, dimensions);

            if (personalBestValue[i] < result) {
                result = personalBestValue[i];
                for (int d = 0; d < dimensions; ++d)
                    globalBest[d] = position[index(i, d)];
            }
        }

        for (int iteration = 0; iteration < maxIterations; ++iteration) {
            for (int i = 0; i < populationSize; ++i) {
                for (int d = 0; d < dimensions; ++d) {
                    int idx = index(i, d);
                    float r1 = randomFloat();
                    float r2 = randomFloat();

                    velocity[idx] =
                        inertia * velocity[idx]
                        + cognitive * r1 * (personalBest[idx] - position[idx])
                        + social * r2 * (globalBest[d] - position[idx]);

                    float maxVelocity = 0.2f * (upperBound[d] - lowerBound[d]);
                    if (velocity[idx] > maxVelocity) velocity[idx] = maxVelocity;
                    if (velocity[idx] < -maxVelocity) velocity[idx] = -maxVelocity;

                    position[idx] += velocity[idx];

                    if (position[idx] < lowerBound[d]) {
                        position[idx] = lowerBound[d];
                        velocity[idx] = 0.0f;
                    }
                    if (position[idx] > upperBound[d]) {
                        position[idx] = upperBound[d];
                        velocity[idx] = 0.0f;
                    }
                }

                float currentValue =
                    function(position + i * dimensions, dimensions);

                if (currentValue < personalBestValue[i]) {
                    personalBestValue[i] = currentValue;
                    for (int d = 0; d < dimensions; ++d)
                        personalBest[index(i, d)] = position[index(i, d)];
                }

                if (currentValue < result) {
                    result = currentValue;
                    for (int d = 0; d < dimensions; ++d)
                        globalBest[d] = position[index(i, d)];
                }
            }
        }
    }

    void getBestPosition(float* output) const {
        for (int d = 0; d < dimensions; ++d)
            output[d] = globalBest[d];
    }
};

#endif
