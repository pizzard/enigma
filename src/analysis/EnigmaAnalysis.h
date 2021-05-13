#ifndef ANALYSIS_ENIGMAANALYSIS_H
#define ANALYSIS_ENIGMAANALYSIS_H

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <execution>
#include <numeric>

#include <enigma/Enigma.h>
#include <analysis/ScoredEnigmaKey.h>
#include <analysis/fitness/IoCFitness.h>

int8_t floorMod(const int8_t& a, const int8_t& b)
{
    return (a % b + b) % b;
}

class EnigmaAnalysis {
public:
    enum AvailableRotors {
        THREE,
        FIVE,
        EIGHT
  };

  template <class FitnessFunction, int T>
  static std::vector<ScoredEnigmaKey> findRotorConfiguration(
      std::array<int8_t, T> ciphertext,
      AvailableRotors rotors,
      Plugboard plugboard) {
    std::vector<int8_t> availableRotorList;

    switch (rotors) {
      case THREE:
      availableRotorList = {1,2,3};
      break;
      case FIVE:
      availableRotorList = {1,2,3,4,5};
      break;
      case EIGHT:
      default:
      availableRotorList = {1,2,3,4,5,6,7,8};
      break;
    }

    std::vector<std::pair<int, std::array<int8_t, 3>>> rotorComb;
    int i{0};
    for (int8_t rotor1 : availableRotorList) {
      for (int8_t rotor2 : availableRotorList) {
        if (rotor1 == rotor2)
          continue;
        for (int8_t rotor3 : availableRotorList) {
          if ((rotor1 == rotor3) || (rotor2 == rotor3))
            continue;
          rotorComb.emplace_back(std::make_pair(i++, std::array<int8_t, 3>{rotor1, rotor2, rotor3}));
        }
      }
    }

    std::vector<ScoredEnigmaKey> keySet ;
    keySet.resize(rotorComb.size());

    std::for_each(std::execution::seq, rotorComb.cbegin(), rotorComb.cend(), [&] (const auto& comb)
    {
      const std::array<int8_t, 3>& rotors = comb.second;
      Enigma e { rotors, 'B', { 0, 0, 0 }, { 0, 0, 0 }, plugboard };
      float maxFitness = -1e30f;
      ScoredEnigmaKey bestKey;
      for (int8_t i = 0; i < 26; i += 1) {
        for (int8_t j = 0; j < 26; j++) {
          for (int8_t k = 0; k < 26; k += 1) {
            e.resetRotorPositions(i,j,k);
            FitnessFunction f;
            for (int i = 0; i <ciphertext.size(); i++) {
              f.score(e.encrypt(ciphertext[i]));
             }
            float fitness = f.sumScores(ciphertext.size());
            if (fitness > bestKey.score) {
              bestKey = ScoredEnigmaKey( rotors, { i, j, k }, {0,0,0},
                  plugboard);
              bestKey.score = fitness;
            }
          }
        }
      }
      keySet[comb.first] = std::move(bestKey);
    });

    // Sort keys by best performing (highest fitness score)
    std::sort(keySet.begin(), keySet.end(),
        [](const ScoredEnigmaKey& l, const ScoredEnigmaKey& r) {
          return l.score > r.score;
    });
    return keySet;
  }

  template<class FitnessFunction>
  static ScoredEnigmaKey findRingSettings(
      ScoredEnigmaKey key,
      const char* ciphertext,
      FitnessFunction f) {

    // Optimise both rotors
    key.rings = findRingSetting(key, ciphertext, f);
    // now we know the correct difference between rotors and starting positions.
    // Calculate fitness and return scored key
    Enigma e(key.rotors, 'B', key.rings, key.indicators, key.plugboard);
    std::string result = encryptString(e, ciphertext);
    key.score = f.score(result);
    return key;
  }


  template<class FitnessFunction>
  static std::array<int8_t, 3> findRingSetting(const ScoredEnigmaKey& key, const char* ciphertext, FitnessFunction f) {
    std::array<int8_t, 3> originalIndicators = key.indicators;

    // this now find the optimal differerence between starting positions and ring settings
    std::array<std::pair<std::array<int8_t, 3>, float>, 26> optimalRingsResults{};
    std::array<int8_t, 26> ints{};
    std::iota(ints.begin(), ints.end(), 0);
    std::for_each(std::execution::seq, ints.cbegin(), ints.cend(), [&] (const auto& i)
    {
      float maxFitness = -1e30f;
      std::array<int8_t, 3> optimalRings{};
      std::string result;
      for (int8_t j = 0; j < 26; j++) {
        for (int8_t k = 0; k < 26; k++) {
          for (int8_t l = 0; l < 26; l++) {
            std::array<int8_t, 3> currentRings{k,j,i};
            Enigma e{key.rotors,
              'B',
              {0,0,l},
              currentRings,
              key.plugboard};
            result = encryptString(e, ciphertext);
            float fitness = f.score(result);
            if (fitness > maxFitness) {
              maxFitness = fitness;
              optimalRings = currentRings;
              optimalRings[2] -= l;
            }
          }
        }
      }
      optimalRingsResults[i].first = optimalRings;
      optimalRingsResults[i].second = maxFitness;
    });

    auto it = std::max_element(optimalRingsResults.begin(), optimalRingsResults.end(),
        [](const auto &l, const auto &r) {
          return l.second < r.second;
        });
    return it->first;
  }


  template<class FitnessFunction>
  static ScoredEnigmaKey findStartingPositions(
      ScoredEnigmaKey key,
      const char* ciphertext,
      FitnessFunction f) {

    float maxFitness = -1e30f;
    std::string result;
    std::array<int8_t, 3> optimalStarts{};
    // now try all starting positions
    for (int8_t i = 0; i < 26; i++) {
      for (int8_t j = 0; j < 26; j++) {
        for (int8_t k = 0; k < 26; k++) {
          std::array<int8_t, 3> currentIndicators{i,j,k};
          std::array<int8_t, 3> currentRings = key.rings;
          currentRings[0] = (currentRings[0] + i) %26;
          currentRings[1] = (currentRings[1] + j) %26;
          currentRings[2] = (currentRings[2] + k) %26;
          Enigma e{key.rotors,
              'B',
              currentIndicators,
              currentRings,
              key.plugboard};
          result = encryptString(e, ciphertext);
          float fitness = f.score(result);
          if (fitness > maxFitness) {
            maxFitness = fitness;
            optimalStarts = currentIndicators;
          }
        }
      }
    }
    key.indicators = optimalStarts;
    key.rings[0] = (optimalStarts[0] + key.rings[0]) %26;
    key.rings[1] = (optimalStarts[1] + key.rings[1]) %26;
    key.rings[2] = (optimalStarts[2] + key.rings[2]) %26;

    // Calculate fitness and return scored key
    Enigma e(key.rotors, 'B', key.rings, key.indicators, key.plugboard);
    result = encryptString(e, ciphertext);
    key.score = f.score(result);
    return key;
  }
  // this modifies the plugs on the fly
  template<class FitnessFunction>
  static void findPlug(
      ScoredEnigmaKey& key,
      const char* ciphertext,
      FitnessFunction f) {

    std::string result;
    ScoredEnigmaKey newKey = key;
    for (int8_t i = 0; i < 26; ++i) {
      // if this character is already plugged.
      if(newKey.plugboard.plugged[i] == true)
        continue;
      // only search further down the plugboard
      for (int8_t j = i; j < 26; ++j) {
        // check if this one is plugged
        if(newKey.plugboard.plugged[j] == true)
               continue;

        Plugboard newBoard = key.plugboard;
        newBoard.addPlug(i,j);

        Enigma e(key.rotors,
            'B',
            key.indicators,
            key.rings,
            newBoard);
        result = encryptString(e, ciphertext);
        float fitness = f.score(result);
        if (fitness > newKey.score) {
          newKey.score = fitness;
          newKey.plugboard = newBoard;
        }
      }
    }

    // just add the best candidate
    key = newKey;
  }

 template<class FitnessFunction>
 static ScoredEnigmaKey findPlugs(
     ScoredEnigmaKey key, int8_t maxPlugs,
     const char* ciphertext,
     FitnessFunction f) {
   for (int8_t i = 0; i < maxPlugs; i++) {
      findPlug(key, ciphertext, f);
   }

   // Calculate fitness and return scored key
   Enigma e(key.rotors, 'B', key.rings, key.indicators, key.plugboard);
   std::string result = encryptString(e, ciphertext);
   key.score = f.score(result);
    return key;
  }
};

#endif
