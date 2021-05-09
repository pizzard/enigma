#ifndef ANALYSIS_QUADGRAMFITNESS_H
#define ANALYSIS_QUADGRAMFITNESS_H

#include <data/quadgrams.h>

struct QuadgramFitness{

    float score(const std::string& text) const {
        float fitness = 0;
        int current = 0;
        int next1 = text[0] - 65;
        int next2 = text[1] - 65;
        int next3 = text[2] - 65;
        for (int i = 3; i < text.size(); i++) {
            current = next1;
            next1 = next2;
            next2 = next3;
            next3 = text[i] - 65;
            fitness += quadgram_scores.score(current, next1, next2, next3);
        }
        return fitness;
    }
};


#endif
