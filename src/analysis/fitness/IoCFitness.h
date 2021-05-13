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
        std::array<int, 26+26+26> histogram{0};

        for (int c : text) {
            histogram[c]++;
        }

        int n = text.size();
        float total = 0.0f;

        for (size_t i = 0; i < 26; ++i) {
          int v = histogram[i] + histogram[i + 26] + histogram[i + 52];
          total += (v * (v - 1));
        }

        return total / (n * (n-1));
    }
};


struct IoCFitnessInterleaved {

    constexpr void score(int8_t c){
      histogram[c]++;
    }

    constexpr float sumScores(int text_size) const
    {
      float total = 0.0f;
      for (size_t i = 0; i < 26; ++i) {
        int v = histogram[i] + histogram[i + 26] + histogram[i + 52];
        total += (v * (v - 1));
      }
      return total / (text_size * (text_size-1));
    }


    std::array<int, 26+26+26> histogram{0};

};

#endif
