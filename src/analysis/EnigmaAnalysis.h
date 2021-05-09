#ifndef ANALYSIS_ENIGMAANALYSIS_H
#define ANALYSIS_ENIGMAANALYSIS_H

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

#include <enigma/Enigma.h>
#include <analysis/ScoredEnigmaKey.h>

int floorMod(const int& a, const int& b)
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

  template<class FitnessFunction>
  static std::vector<ScoredEnigmaKey> findRotorConfiguration(
      const char* ciphertext,
      AvailableRotors rotors,
      Plugboard plugboard,
      const FitnessFunction& f) {
    std::vector<int> availableRotorList;

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

    std::string result;
    std::vector<ScoredEnigmaKey> keySet ;
    keySet.reserve(100);

    for (int rotor1 : availableRotorList) {
      for (int rotor2 : availableRotorList) {
        if (rotor1 == rotor2)
          continue;
        for (int rotor3 : availableRotorList) {
          if ((rotor1 == rotor3) || (rotor2 == rotor3))
            continue;
          std::cout << rotor1 << " " << rotor2 << " " << rotor3 << "\n";

          float maxFitness = -1e30f;
          ScoredEnigmaKey bestKey;
          for (int i = 0; i < 26; i++) {
            for (int j = 0; j < 26; j++) {
              for (int k = 0; k < 26; k++) {
                Enigma e { { rotor1, rotor2, rotor3 }, 'B', { i, j, k }, { 0, 0, 0 }, plugboard };
                result = encryptString(e, ciphertext);
                float fitness = f.score(result);
                if (fitness > bestKey.score) {
                  bestKey = ScoredEnigmaKey( { rotor1, rotor2, rotor3 }, { i, j, k }, {0,0,0},
                      plugboard);
                  bestKey.score = fitness;
                }
              }
            }
          }
          keySet.emplace_back(std::move(bestKey));
        }
      }
    }

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
    int rightRotorIndex = 2, middleRotorIndex = 1;

    // Optimise right rotor
    int optimalIndex = findRingSetting(key, ciphertext, rightRotorIndex, f);
    key.rings[rightRotorIndex] = optimalIndex;
    key.indicators[rightRotorIndex] = (key.indicators[rightRotorIndex] + optimalIndex) % 26;

    // Optimise middle rotor
    optimalIndex = findRingSetting(key, ciphertext, middleRotorIndex, f);
    key.rings[middleRotorIndex] = optimalIndex;
    key.indicators[middleRotorIndex] = (key.indicators[middleRotorIndex] + optimalIndex) % 26;

    // Calculate fitness and return scored key
    Enigma e(key.rotors, 'B', key.rings, key.indicators, key.plugboard);
    std::string result = encryptString(e, ciphertext);
    key.score = f.score(result);
    return key;
  }

  template<class FitnessFunction>
  static int findRingSetting(const ScoredEnigmaKey& key, const char* ciphertext, int rotor, FitnessFunction f) {
    auto originalIndicators = key.indicators;
    auto originalRingSettings = key.rings;

    int optimalRingSetting = 0;
    float maxFitness = -1e30f;
    std::string result;
    for (int i = 0; i < 26; i++) {
      auto currentStartingPositions = originalIndicators;
      auto currentRingSettings = originalRingSettings;

      currentStartingPositions[rotor] = floorMod(currentStartingPositions[rotor] + i, 26);
      currentRingSettings[rotor] = i;

      Enigma e{key.rotors,
          'B',
          currentStartingPositions,
          currentRingSettings,
          key.plugboard};
      result = encryptString(e, ciphertext);
      float fitness = f.score(result);
      if (fitness > maxFitness) {
        maxFitness = fitness;
        optimalRingSetting = i;
      }
    }
    return optimalRingSetting;
  }

  // this modifies the plugs on the fly
  template<class FitnessFunction>
  static void findPlug(
      ScoredEnigmaKey& key,
      const char* ciphertext,
      FitnessFunction f) {

    std::string result;
    ScoredEnigmaKey newKey = key;
    for (int i = 0; i < 26; ++i) {
      // if this character is already plugged.
      if(newKey.plugboard.plugged[i] == true)
        continue;
      // only search further down the plugboard
      for (int j = i; j < 26; ++j) {
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
     ScoredEnigmaKey key, int maxPlugs,
     const char* ciphertext,
     FitnessFunction f) {
   for (int i = 0; i < maxPlugs; i++) {
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
