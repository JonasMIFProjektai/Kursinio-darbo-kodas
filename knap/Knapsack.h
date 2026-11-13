#ifndef KNAPSACK_H
#define KNAPSACK_H

// Convert a continuous coordinate to a selection probability.
inline float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

// A deterministic binary transfer: p >= 0.5 means the item is selected.
// Since sigmoid(x) >= 0.5 exactly when x >= 0, this is a threshold decoder.
inline int decodeBit(float x) {
    return sigmoid(x) >= 0.5f ? 1 : 0;
}

// Returns a minimization fitness:
//   feasible solution   -> negative total value (higher value is better)
//   infeasible solution -> large positive penalty
float knapsackFitness(float* position, int dimensions);

#endif
