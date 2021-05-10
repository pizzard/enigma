#ifndef ANALYSIS_IOCFITNESS_H
#define ANALYSIS_IOCFITNESS_H

#include <enigma/Rotor.h>
#include <string>

struct IoCFitness {
    float score(const std::string& text) const {
        std::array<int, 26> histogram;
        for(int& i: histogram)
          i = 0;

        for (char c : text) {
            histogram[charToIndex(c)]++;
        }

        int n = text.size();
        float total = 0.0f;

        for (int v : histogram) {
            total += (v * (v - 1));
        }

        return total / (n * (n-1));
    }

    template<class T>
    float score(const T& text) const {
        std::array<int, 26> histogram{0};

        for (int c : text) {
            histogram[c%26]++;
        }

        int n = text.size();
        float total = 0.0f;

        for (int v : histogram) {
            total += (v * (v - 1));
        }

        return total / (n * (n-1));
    }
};

#endif
