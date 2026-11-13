#include "PSO.h"
#include "Wolf.h"
#include "HEOA.h"
#include "BHO.h"
#include "Dolphin.h"
#include "DE.h"
#include "Knapsack.h"
#include "Other.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>
#include <random>
#include <cmath>
#include <cstdint>
#include <string>

using namespace std;

// ============================================================
// EXPERIMENT CONFIGURATION
// ============================================================

const int ITEM_COUNT = 27;
const int EXPERIMENT_COUNT = 50;

const int POPULATION_SIZE = 50;
const int MAX_ITERATIONS = 500;

float WEIGHTS[ITEM_COUNT];
float VALUES[ITEM_COUNT];
float CAPACITY = 0.0f;

// ============================================================
// ALGORITHM STATISTICS FOR SUMMARY
// ============================================================

struct AlgorithmStats {
    const char* name;
    double totalTime;
    double totalAccuracy;
};

AlgorithmStats algoStats[9] = {
    {"Brute Force", 0.0, 0.0},
    {"Particle Swarm Optimization", 0.0, 0.0},
    {"Grey Wolf Optimizer", 0.0, 0.0},
    {"HEOA", 0.0, 0.0},
    {"Black Hole Optimization", 0.0, 0.0},
    {"Dolphin Optimization", 0.0, 0.0},
    {"Differential Evolution", 0.0, 0.0},
    {"Harmony Search", 0.0, 0.0},
    {"Greedy", 0.0, 0.0}
};

// ============================================================
// FITNESS FUNCTION FOR THE POPULATION-BASED ALGORITHMS
// ============================================================

float knapsackFitness(float* position, int dimensions) {
    float totalWeight = 0.0f;
    float totalValue = 0.0f;

    for (int i = 0; i < dimensions && i < ITEM_COUNT; ++i) {
        if (decodeBit(position[i])) {
            totalWeight += WEIGHTS[i];
            totalValue += VALUES[i];
        }
    }

    if (totalWeight > CAPACITY) {
        return 1000000.0f + (totalWeight - CAPACITY) * 1000.0f;
    }

    // The optimization algorithms minimize their fitness function.
    return -totalValue;
}

// ============================================================
// RANDOM PROBLEM GENERATION
// ============================================================

void generateRandomProblem(mt19937& generator) {
    uniform_real_distribution<float> weightDistribution(1.0f, 20.0f);
    uniform_real_distribution<float> valueDistribution(10.0f, 200.0f);

    float totalWeight = 0.0f;

    for (int i = 0; i < ITEM_COUNT; ++i) {
        WEIGHTS[i] = weightDistribution(generator);
        VALUES[i] = valueDistribution(generator);
        totalWeight += WEIGHTS[i];
    }

    uniform_real_distribution<float> capacityDistribution(
        totalWeight * 0.25f,
        totalWeight * 0.50f
    );

    CAPACITY = capacityDistribution(generator);
}

// ============================================================
// SAVE GENERATED PROBLEM (TEXT & LATEX)
// ============================================================

void saveProblem(ofstream& file, ofstream& latexFile, int problemNumber) {
    file << fixed << setprecision(2);

    file << "========================================\n";
    file << "PROBLEM NUMBER: " << problemNumber << "\n";
    file << "========================================\n";
    file << "Capacity: " << CAPACITY << "\n\n";
    file << "Item\tWeight\t\tValue\n";

    for (int i = 0; i < ITEM_COUNT; ++i) {
        file << i + 1 << "\t"
             << WEIGHTS[i] << "\t\t"
             << VALUES[i] << "\n";
    }

    file << "\n\n";

    // --- Write Problem Section Header & Parameters to LaTeX ---
    latexFile << fixed << setprecision(2);
    latexFile << "\\section*{Problem " << problemNumber << "}\n";
    latexFile << "\\begin{itemize}\n";
    latexFile << "    \\item \\textbf{Daiktų kiekis:} " << ITEM_COUNT << "\n";
    latexFile << "    \\item \\textbf{Maksimali talpa:} " << CAPACITY << "\n";
    latexFile << "    \\item \\textbf{Vertės daiktų:} ";
    for (int i = 0; i < ITEM_COUNT; ++i) {
        latexFile << VALUES[i] << (i < ITEM_COUNT - 1 ? ", " : "");
    }
    latexFile << "\n\\end{itemize}\n\n";

    // Start LaTeX Results Table for this problem
    latexFile << "\\begin{table}[h!]\n";
    latexFile << "\\centering\n";
    latexFile << "\\begin{tabular}{lrrrr}\n";
    latexFile << "\\toprule\n";
    latexFile << "\\textbf{Algoritmas} & \\textbf{Vykdymo laikas (ms)} & \\textbf{Vertė} & \\textbf{Talpa} & \\textbf{Tikslumas (\\%)} \\\\\n";
    latexFile << "\\midrule\n";
}

// ============================================================
// SAVE RESULTS FOR A BOOL-BASED KNAPSACK ALGORITHM
// Used by Harmony, Greedy and BruteForce.
// ============================================================

void saveBoolResult(
    ofstream& file,
    ofstream& latexFile,
    const char* algorithm,
    int problemNumber,
    const bool* selectedItems,
    double executionTime,
    float optimalValue
) {
    float totalWeight = 0.0f;
    float totalValue = 0.0f;

    for (int i = 0; i < ITEM_COUNT; ++i) {
        if (selectedItems[i]) {
            totalWeight += WEIGHTS[i];
            totalValue += VALUES[i];
        }
    }

    const bool feasible = (totalWeight <= CAPACITY);
    const bool optimal = feasible &&
        (fabs(totalValue - optimalValue) < 0.001f);

    // Calculate accuracy percentage (algorithm value / brute force value * 100)
    float accuracy = (optimalValue > 0.0f) ? (totalValue / optimalValue) * 100.0f : 0.0f;

    // Accumulate stats for summary
    for (int i = 0; i < 9; ++i) {
        if (string(algoStats[i].name) == string(algorithm)) {
            algoStats[i].totalTime += executionTime;
            algoStats[i].totalAccuracy += accuracy;
            break;
        }
    }

    // --- Write to standard .txt log ---
    file << fixed << setprecision(4);
    file << "========================================\n";
    file << "PROBLEM NUMBER: " << problemNumber << "\n";
    file << "ALGORITHM: " << algorithm << "\n";
    file << "========================================\n";

    file << "Selected items: ";
    for (int i = 0; i < ITEM_COUNT; ++i) {
        file << (selectedItems[i] ? 1 : 0) << " ";
    }

    file << "\nTotal weight: " << totalWeight << "\n";
    file << "Capacity: " << CAPACITY << "\n";
    file << "Total value: " << totalValue << "\n";
    file << "Optimal value: " << optimalValue << "\n";
    file << "Execution time: " << executionTime << " ms\n";
    file << "Feasible: " << (feasible ? "YES" : "NO") << "\n";
    file << "Optimal solution found: " << (optimal ? "YES" : "NO") << "\n\n\n";

    // --- Write Row to LaTeX Table ---
    latexFile << fixed << setprecision(4);
    latexFile << algorithm << " & "
              << executionTime << " & "
              << setprecision(2) << totalValue << " & "
              << totalWeight << " / " << CAPACITY << " & "
              << accuracy << "\\% \\\\\n";

    cout << algorithm
         << " | laikas: " << executionTime << " ms"
         << " | Value: " << totalValue
         << " | Accuracy: " << setprecision(2) << accuracy << "%"
         << " | Optimal: " << (optimal ? "YES" : "NO")
         << " | Feasible: " << (feasible ? "YES" : "NO")
         << "\n";
}

// ============================================================
// ADAPTER FOR THE FLOAT-BASED POPULATION ALGORITHMS
// ============================================================

void saveFloatResult(
    ofstream& file,
    ofstream& latexFile,
    const char* algorithm,
    int problemNumber,
    const float* position,
    double executionTime,
    float optimalValue
) {
    bool selectedItems[ITEM_COUNT];

    for (int i = 0; i < ITEM_COUNT; ++i) {
        selectedItems[i] = decodeBit(position[i]);
    }

    saveBoolResult(
        file, latexFile, algorithm, problemNumber,
        selectedItems, executionTime, optimalValue
    );
}

// ============================================================
// MAIN EXPERIMENT
// ============================================================

int main() {
    mt19937 generator(static_cast<unsigned int>(time(nullptr)));
    srand(static_cast<unsigned int>(time(nullptr)));

    float lowerBound[ITEM_COUNT];
    float upperBound[ITEM_COUNT];

    for (int i = 0; i < ITEM_COUNT; ++i) {
        lowerBound[i] = -6.0f;
        upperBound[i] = 6.0f;
    }

    // All experiment output files are written to this directory.
    const string outputDirectory =
        "C:\\Users\\jonas\\OneDrive\\Desktop\\kurisinis\\knap\\res\\";

    ofstream problemsFile(outputDirectory + "Problems.txt");
    ofstream latexFile(outputDirectory + "Results.tex");

    ofstream psoFile(outputDirectory + "PSO_Results.txt");
    ofstream wolfFile(outputDirectory + "GWO_Results.txt");
    ofstream heoaFile(outputDirectory + "HEOA_Results.txt");
    ofstream bhoFile(outputDirectory + "BHO_Results.txt");
    ofstream dolphinFile(outputDirectory + "Dolphin_Results.txt");
    ofstream deFile(outputDirectory + "DE_Results.txt");

    ofstream harmonyFile(outputDirectory + "Harmony_Results.txt");
    ofstream greedyFile(outputDirectory + "Greedy_Results.txt");
    ofstream bruteForceFile(outputDirectory + "BruteForce_Results.txt");

    if (!problemsFile.is_open() ||
        !latexFile.is_open() ||
        !psoFile.is_open() ||
        !wolfFile.is_open() ||
        !heoaFile.is_open() ||
        !bhoFile.is_open() ||
        !dolphinFile.is_open() ||
        !deFile.is_open() ||
        !harmonyFile.is_open() ||
        !greedyFile.is_open() ||
        !bruteForceFile.is_open()) {

        cerr << "Error: Could not create one or more output files.\n"
             << "Check that this directory exists:\n"
             << outputDirectory << endl;
        return 1;
    }

    // Write LaTeX Document Preamble
    latexFile << "\\documentclass[a4paper,11pt]{article}\n"
              << "\\usepackage[utf8]{inputenc}\n"
              << "\\usepackage[T1]{fontenc}\n"
              << "\\usepackage[lithuanian]{babel}\n"
              << "\\usepackage{booktabs}\n"
              << "\\usepackage{geometry}\n"
              << "\\geometry{margin=1in}\n\n"
              << "\\begin{document}\n\n"
              << "\\title{Knapsack Problem Optimization Results}\n"
              << "\\date{\\today}\n"
              << "\\maketitle\n\n";

    cout << fixed << setprecision(4);
    cout << "========================================\n";
    cout << "      KNAPSACK ALGORITHM EXPERIMENT\n";
    cout << "========================================\n";
    cout << "Problems: " << EXPERIMENT_COUNT << "\n";
    cout << "Items per problem: " << ITEM_COUNT << "\n";
    cout << "========================================\n";

    for (int experiment = 1;
         experiment <= EXPERIMENT_COUNT;
         ++experiment) {

        cout << "\n########################################\n";
        cout << "PROBLEM " << experiment
             << " / " << EXPERIMENT_COUNT << "\n";
        cout << "########################################\n";

        // 1. Generate and save one random problem.
        generateRandomProblem(generator);
        saveProblem(problemsFile, latexFile, experiment);

        cout << "Capacity: " << CAPACITY << "\n";

        // ----------------------------------------------------
        // 2. BRUTE FORCE (Run first to establish exact optimum)
        // ----------------------------------------------------
        float optimalValue = 0.0f;
        {
            BruteForce bruteForce(CAPACITY, VALUES, WEIGHTS, ITEM_COUNT);

            auto start = chrono::steady_clock::now();
            bruteForce.solve();
            auto end = chrono::steady_clock::now();

            const bool* optimalItems = bruteForce.getSolution();
            for (int i = 0; i < ITEM_COUNT; ++i) {
                if (optimalItems[i]) {
                    optimalValue += VALUES[i];
                }
            }

            cout << "True optimal value: " << optimalValue << "\n\n";

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveBoolResult(bruteForceFile, latexFile, "Brute Force",
                           experiment, optimalItems,
                           elapsed, optimalValue);
        }

        // ----------------------------------------------------
        // 3. PARTICLE SWARM OPTIMIZATION
        // ----------------------------------------------------
        {
            ParticleSwarmOptimization pso(
                upperBound, lowerBound, ITEM_COUNT,
                POPULATION_SIZE, MAX_ITERATIONS, knapsackFitness
            );

            auto start = chrono::steady_clock::now();
            pso.optimize();
            auto end = chrono::steady_clock::now();

            float position[ITEM_COUNT];
            pso.getBestPosition(position);

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveFloatResult(psoFile, latexFile, "Particle Swarm Optimization",
                            experiment, position, elapsed, optimalValue);
        }

        // ----------------------------------------------------
        // 4. GREY WOLF OPTIMIZER
        // ----------------------------------------------------
        {
            Wolf wolf(
                upperBound, lowerBound, ITEM_COUNT,
                POPULATION_SIZE, MAX_ITERATIONS, knapsackFitness
            );

            auto start = chrono::steady_clock::now();
            wolf.optimize();
            auto end = chrono::steady_clock::now();

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveFloatResult(wolfFile, latexFile, "Grey Wolf Optimizer",
                            experiment, wolf.getBestPosition(),
                            elapsed, optimalValue);
        }

        // ----------------------------------------------------
        // 5. HEOA
        // ----------------------------------------------------
        {
            HEOA heoa(
                upperBound, lowerBound, ITEM_COUNT,
                POPULATION_SIZE, MAX_ITERATIONS, knapsackFitness
            );

            float position[ITEM_COUNT];

            auto start = chrono::steady_clock::now();
            heoa.optimize(position);
            auto end = chrono::steady_clock::now();

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveFloatResult(heoaFile, latexFile, "HEOA",
                            experiment, position, elapsed, optimalValue);
        }

        // ----------------------------------------------------
        // 6. BLACK HOLE OPTIMIZATION
        // ----------------------------------------------------
        {
            BHO bho(
                upperBound, lowerBound, ITEM_COUNT,
                POPULATION_SIZE, MAX_ITERATIONS, knapsackFitness
            );

            float position[ITEM_COUNT];

            auto start = chrono::steady_clock::now();
            bho.optimize(position);
            auto end = chrono::steady_clock::now();

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveFloatResult(bhoFile, latexFile, "Black Hole Optimization",
                            experiment, position, elapsed, optimalValue);
        }

        // ----------------------------------------------------
        // 7. DOLPHIN OPTIMIZATION
        // ----------------------------------------------------
        {
            Dolphin dolphin(
                upperBound, lowerBound, ITEM_COUNT,
                POPULATION_SIZE, MAX_ITERATIONS, knapsackFitness
            );

            auto start = chrono::steady_clock::now();
            dolphin.optimize();
            auto end = chrono::steady_clock::now();

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveFloatResult(dolphinFile, latexFile, "Dolphin Optimization",
                            experiment, dolphin.getBestPosition(),
                            elapsed, optimalValue);
        }

        // ----------------------------------------------------
        // 8. DIFFERENTIAL EVOLUTION
        // ----------------------------------------------------
        {
            DE de(
                upperBound, lowerBound, ITEM_COUNT,
                POPULATION_SIZE, MAX_ITERATIONS, knapsackFitness
            );

            float position[ITEM_COUNT];

            auto start = chrono::steady_clock::now();
            de.optimize(position);
            auto end = chrono::steady_clock::now();

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveFloatResult(deFile, latexFile, "Differential Evolution",
                            experiment, position, elapsed, optimalValue);
        }

        // ----------------------------------------------------
        // 9. HARMONY SEARCH
        // KnapSack constructor expects: capacity, values, weights, count.
        // ----------------------------------------------------
        {
            Harmony harmony(CAPACITY, VALUES, WEIGHTS, ITEM_COUNT);

            auto start = chrono::steady_clock::now();
            harmony.solve();
            auto end = chrono::steady_clock::now();

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveBoolResult(harmonyFile, latexFile, "Harmony Search",
                           experiment, harmony.getSolution(),
                           elapsed, optimalValue);
        }

        // ----------------------------------------------------
        // 10. GREEDY
        // ----------------------------------------------------
        {
            Greedy greedy(CAPACITY, VALUES, WEIGHTS, ITEM_COUNT);

            auto start = chrono::steady_clock::now();
            greedy.solve();
            auto end = chrono::steady_clock::now();

            double elapsed = chrono::duration<double, milli>(end - start).count();
            saveBoolResult(greedyFile, latexFile, "Greedy",
                           experiment, greedy.getSolution(),
                           elapsed, optimalValue);
        }

        // Close the LaTeX table for the current problem
        latexFile << "\\bottomrule\n"
                  << "\\end{tabular}\n"
                  << "\\caption{Results for Problem " << experiment << "}\n"
                  << "\\end{table}\n"
                  << "\\clearpage\n\n";

        cout << "\nFinished problem "
             << experiment << " / " << EXPERIMENT_COUNT << "\n";
    }

    // ============================================================
    // WRITE SUMMARY TABLE TO LATEX (AVERAGES)
    // ============================================================
    latexFile << "\\newpage\n";
    latexFile << "\\section*{Algoritmų apibendrinimas}\n";
    latexFile << "Vidutiniai rezultatai per " << EXPERIMENT_COUNT << " eksperimentus:\\par\\medskip\n\n";
    latexFile << "\\begin{table}[h!]\n";
    latexFile << "\\centering\n";
    latexFile << "\\begin{tabular}{lrr}\n";
    latexFile << "\\toprule\n";
    latexFile << "\\textbf{Algoritmas} & \\textbf{Vidutinis laikas (ms)} & \\textbf{Vidutinis tikslumas (\\%)} \\\\\n";
    latexFile << "\\midrule\n";

    for (int i = 0; i < 9; ++i) {
        double avgTime = algoStats[i].totalTime / EXPERIMENT_COUNT;
        double avgAccuracy = algoStats[i].totalAccuracy / EXPERIMENT_COUNT;

        latexFile << fixed << setprecision(4)
                  << algoStats[i].name << " & "
                  << avgTime << " & "
                  << setprecision(2) << avgAccuracy << "\\% \\\\\n";
    }

    latexFile << "\\bottomrule\n";
    latexFile << "\\end{tabular}\n";
    latexFile << "\\caption{Bendras algoritmų efektyvumo apibendrinimas}\n";
    latexFile << "\\end{table}\n\n";

    // Close LaTeX document
    latexFile << "\\end{document}\n";

    // Close all files.
    problemsFile.close();
    latexFile.close();

    psoFile.close();
    wolfFile.close();
    heoaFile.close();
    bhoFile.close();
    dolphinFile.close();
    deFile.close();

    harmonyFile.close();
    greedyFile.close();
    bruteForceFile.close();

    cout << "\n========================================\n";
    cout << "          EXPERIMENT COMPLETED\n";
    cout << "========================================\n";
    cout << "Generated problems: " << EXPERIMENT_COUNT << "\n";
    cout << "Results saved to:\n" << outputDirectory << "\n";

    return 0;
}