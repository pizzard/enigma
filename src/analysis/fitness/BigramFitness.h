#ifndef ANALYSIS_BIGRAMFITNESS_H
#define ANALYSIS_BIGRAMFITNESS_H

#include <data/bigrams.h>
#include <string>


 struct BigramFitness {

    float score(const std::string& text) {
        float fitness = 0;
        int current = 0;
        int next = text[0] - 65;
        for (int i = 1; i < text.size(); i++) {
            current = next;
            next = text[i] - 65;
            fitness += bigram_scores.scoreBigram(current, next);
        }
        return fitness;
    }
};

#endif