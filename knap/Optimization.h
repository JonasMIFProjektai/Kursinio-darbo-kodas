#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include <iostream>
#include <cmath>
#include <limits>

class Optimization {
protected:
    float* upperBound;
    float* lowerBound;
    int populationSize;
    int dimensions;
    int maxIterations;
    float result;
    float (*function)(float*, int);

public:
    Optimization(float* upperBound, float* lowerBound,
                 int dimensions, int populationSize, int maxIterations,
                 float (*function)(float*, int))
        : upperBound(upperBound),
          lowerBound(lowerBound),
          populationSize(populationSize),
          dimensions(dimensions),
          maxIterations(maxIterations),
          result(std::numeric_limits<float>::infinity()),
          function(function) {}

    virtual ~Optimization() = default;

    float getResult() const {
        return result;
    }
};

#endif
