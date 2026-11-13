#ifndef OTHER_H
#define OTHER_H

#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

class KnapSack{

    protected:

        float maxLeistinaTalpa, sprendinioTalpa, sprendinioKaina;
        float* kainos, *turiai; 
        int sackArraySize;
        bool* solution;
        clock_t start, end;

    public:
        //constructors
        KnapSack(float maxLeistinaTalpa, float kainos[], float turiai[], int sackArraySize){
            this->maxLeistinaTalpa = maxLeistinaTalpa;
            this->kainos = kainos;
            this->turiai = turiai;
            this->sackArraySize = sackArraySize;
            solution = new bool[sackArraySize]{};
            sprendinioTalpa = 0.0f;
            sprendinioKaina = 0.0f;
        }

        KnapSack(){
            this->maxLeistinaTalpa = 20.0f;
            this->sackArraySize = 4;
            this->kainos = new float[sackArraySize]{15.0, 20.0, 18.0, 30.0};
            this->turiai = new float[sackArraySize]{5.0, 9.0, 7.0, 11.2};
            solution = new bool[sackArraySize]{};
            sprendinioTalpa = 0.0f;
            sprendinioKaina = 0.0f;
        }

        //getters
        bool* getSolution(){
            return solution;
        }
        float getSprendinioTalpa(){
            return sprendinioTalpa;
        }
        float getSprendinioKaina(){
            return sprendinioKaina;
        }

        //demonstration functions
        void displayStats(){
            cout << "Took: " << fixed << setprecision(5) << float(end - start) / CLOCKS_PER_SEC << " s.\n";
            cout << "Found the best value to be: " << fixed << setprecision(2) << sprendinioKaina;
            cout << " which took up " << sprendinioTalpa << "/" << maxLeistinaTalpa << " with this configuration:"<< '\n';
            for(auto i = 0; i < sackArraySize; ++i){
                cout << solution[i];
            }
            cout << '\n';
    
        }
};


class Harmony : public KnapSack {
private:
    int harmonyMemorySize = 30; // HM dydis
    int maxIterations = 10000;  // Iteraciju skaicius
    float HMCR = 0.9f;          // Harmony Memory Consideration Rate
    float PAR = 0.3f;           // Pitch Adjustment Rate

    struct HarmonySolution {
        bool* items;
        float value;
        float weight;
    };

    float calculateValue(const bool* items) {
        float value = 0.0f;

        for (int i = 0; i < sackArraySize; ++i) {
            if (items[i]) {
                value += kainos[i];
            }
        }

        return value;
    }

    float calculateWeight(const bool* items) {
        float weight = 0.0f;

        for (int i = 0; i < sackArraySize; ++i) {
            if (items[i]) {
                weight += turiai[i];
            }
        }

        return weight;
    }

    // Generuoja atsitiktini tinkama sprendini
    void generateSolution(bool* items) {
        float remainingCapacity = maxLeistinaTalpa;

        for (int i = 0; i < sackArraySize; ++i) {
            items[i] = false;
        }

        // Atsitiktine produktu pasirinkimo tvarka
        for (int i = sackArraySize - 1; i > 0; --i) {
            int j = rand() % (i + 1);
            bool temp = items[i];
            items[i] = items[j];
            items[j] = temp;
        }

        // Atsitiktinai pasirenkami produktai
        for (int i = 0; i < sackArraySize; ++i) {
            if (rand() % 2 == 1 && turiai[i] <= remainingCapacity) {
                items[i] = true;
                remainingCapacity -= turiai[i];
            }
        }
    }

    void repairSolution(bool* items) {
        float weight = calculateWeight(items);

        // Jei virsijama talpa, pasaliname atsitiktinius produktus
        while (weight > maxLeistinaTalpa) {
            int i = rand() % sackArraySize;

            if (items[i]) {
                items[i] = false;
                weight -= turiai[i];
            }
        }
    }

public:
    Harmony() : KnapSack() {}

    Harmony(float maxLeistinaTalpa,
            float kainos[],
            float turiai[],
            int sackArraySize)
        : KnapSack(maxLeistinaTalpa, kainos, turiai, sackArraySize) {}

    void solve() {
        start = clock();

        srand(static_cast<unsigned int>(time(nullptr)));

        if (sackArraySize <= 0) {
            sprendinioTalpa = 0.0f;
            sprendinioKaina = 0.0f;
            end = clock();
            return;
        }

        HarmonySolution* memory =
            new HarmonySolution[harmonyMemorySize];

        for (int i = 0; i < harmonyMemorySize; ++i) {
            memory[i].items = new bool[sackArraySize]{};

            generateSolution(memory[i].items);
            repairSolution(memory[i].items);

            memory[i].value = calculateValue(memory[i].items);
            memory[i].weight = calculateWeight(memory[i].items);
        }

        int worstIndex = 0;

        for (int iteration = 0;
             iteration < maxIterations;
             ++iteration) {

            bool* newSolution = new bool[sackArraySize]{};

            // Sukuriama nauja harmonija
            for (int i = 0; i < sackArraySize; ++i) {
                if ((static_cast<float>(rand()) / RAND_MAX) < HMCR) {
                    // Pasirenkama reiksme is harmoniju atminties
                    int selected = rand() % harmonyMemorySize;
                    newSolution[i] = memory[selected].items[i];

                    // Pitch adjustment: pakeiciama bito reiksme
                    if ((static_cast<float>(rand()) / RAND_MAX) < PAR) {
                        newSolution[i] = !newSolution[i];
                    }
                } else {
                    // Atsitiktine reiksme
                    newSolution[i] = (rand() % 2 == 1);
                }
            }

            repairSolution(newSolution);

            float newValue = calculateValue(newSolution);
            float newWeight = calculateWeight(newSolution);

            // Randamas blogiausias atminties sprendinys
            worstIndex = 0;

            for (int i = 1; i < harmonyMemorySize; ++i) {
                if (memory[i].value < memory[worstIndex].value) {
                    worstIndex = i;
                }
            }

            // Geresnis sprendinys pakeicia blogiausia
            if (newWeight <= maxLeistinaTalpa &&
                newValue > memory[worstIndex].value) {

                copy(newSolution,
                     newSolution + sackArraySize,
                     memory[worstIndex].items);

                memory[worstIndex].value = newValue;
                memory[worstIndex].weight = newWeight;
            }

            delete[] newSolution;
        }

        // Atrenkamas geriausias sprendinys
        int bestIndex = 0;

        for (int i = 1; i < harmonyMemorySize; ++i) {
            if (memory[i].value > memory[bestIndex].value) {
                bestIndex = i;
            }
        }

        copy(memory[bestIndex].items,
             memory[bestIndex].items + sackArraySize,
             solution);

        sprendinioKaina = memory[bestIndex].value;
        sprendinioTalpa = memory[bestIndex].weight;

        for (int i = 0; i < harmonyMemorySize; ++i) {
            delete[] memory[i].items;
        }

        delete[] memory;

        end = clock();
    }

    void displayStats() {
        cout << "Harmony Search method is finished.\n";
        KnapSack::displayStats();
    }
};

class Greedy: public KnapSack{
    
    private:
        static int comp(const void *a, const void *b) {
            int x = *(const int *)a;
            int y = *(const int *)b;

            if (x < y)
                return 1;
            if (x > y)
                return -1;
            return 0;
        }

    public:

        Greedy(float maxLeistinaTalpa, float kainos[], float turiai[], int sackArraySize)
        : KnapSack(maxLeistinaTalpa, kainos, turiai, sackArraySize){}
        
        void solve() {
            start = clock();

            sprendinioKaina = 0.0f;
            sprendinioTalpa = 0.0f;

            // Reset solution
            for (int i = 0; i < sackArraySize; ++i) {
                solution[i] = false;
            }

            // Store item indices in a dynamic array
            int* order = new int[sackArraySize];

            for (int i = 0; i < sackArraySize; ++i) {
                order[i] = i;
            }

            // Sort by value-to-weight ratio, highest first
            for (int i = 0; i < sackArraySize - 1; ++i) {
                for (int j = i + 1; j < sackArraySize; ++j) {
                    float ratioI = kainos[order[i]] / turiai[order[i]];
                    float ratioJ = kainos[order[j]] / turiai[order[j]];

                    if (ratioJ > ratioI) {
                        int temp = order[i];
                        order[i] = order[j];
                        order[j] = temp;
                    }
                }
        }

    float remainingCapacity = maxLeistinaTalpa;

    // Select whole items while capacity allows
    for (int i = 0; i < sackArraySize; ++i) {
        int index = order[i];

        if (turiai[index] <= remainingCapacity) {
            solution[index] = true;
            sprendinioKaina += kainos[index];
            sprendinioTalpa += turiai[index];
            remainingCapacity -= turiai[index];
        }
    }

    delete[] order;

    end = clock();
    }

        void displayStats(){
            cout << "Greedy method is finished.\n";
            KnapSack :: displayStats();
        }

};

class BruteForce: public KnapSack{
    public:

        BruteForce(float maxLeistinaTalpa, float kainos[], float turiai[], int sackArraySize)
            : KnapSack(maxLeistinaTalpa, kainos, turiai, sackArraySize) {}

        void solve(){
            cout << "Starting brute force method.\n";
            start = clock();
            long long int kiek = 1LL << sackArraySize;
            bool* parinktiProduktai = new bool[sackArraySize];

            for(auto i = 1; i < kiek; ++i){
                float dabartineKaina = 0.0f, dabartineTalpa = 0.0f;
                int kuprine = i;
                for(auto j = 0; j < sackArraySize; ++j){
                    if(kuprine % 2 == 1){
                        dabartineKaina += kainos[j];
                        dabartineTalpa += turiai[j];
                        parinktiProduktai[j] = 1;
                    }else{
                        parinktiProduktai[j] = 0;
                    }
                    kuprine /= 2;
                }
                if(dabartineKaina > sprendinioKaina && dabartineTalpa <= maxLeistinaTalpa){
                    sprendinioKaina = dabartineKaina;
                    sprendinioTalpa = dabartineTalpa;
                    copy(parinktiProduktai, parinktiProduktai + sackArraySize, solution);
                }
            }

            delete(parinktiProduktai);

            end = clock();
        }

        void displayStats(){
            cout << "Brute force method is finished.\n";
            KnapSack :: displayStats();
        }
};

#endif
