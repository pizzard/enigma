#ifndef ANALYSIS_ENIGMAANALYSIS_H
#define ANALYSIS_ENIGMAANALYSIS_H

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

#include <enigma/Enigma.h>
#include <analysis/ScoredEnigmaKey.h>

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


  template<int T, class FitnessFunction>
  static std::vector<ScoredEnigmaKey> findRotorConfiguration(
      std::array<int8_t, T> ciphertext,
      AvailableRotors rotors,
      Plugboard plugboard,
      const FitnessFunction& f) {
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

    std::vector<std::array<int8_t, 3>> rotorComb;
    for (int8_t rotor1 : availableRotorList) {
      for (int8_t rotor2 : availableRotorList) {
        if (rotor1 == rotor2)
          continue;
        for (int8_t rotor3 : availableRotorList) {
          if ((rotor1 == rotor3) || (rotor2 == rotor3))
            continue;
          rotorComb.emplace_back(std::array<int8_t, 3>{rotor1, rotor2, rotor3});
        }
      }
    }

    std::vector<ScoredEnigmaKey> keySet ;
    keySet.resize(rotorComb.size());

    for(size_t i = 0; i < rotorComb.size(); ++i )
    {
      const std::array<int8_t, 3>& rotors = rotorComb[i];
      std::cout << (int)rotors[0] << " " << (int)rotors[1] << " " << (int)rotors[2] << "\n";
      Enigma e { rotors, 'B', { 0, 0, 0 }, { 0, 0, 0 }, plugboard };
      float maxFitness = -1e30f;
      ScoredEnigmaKey bestKey;
      for (int8_t i = 0; i < 26; i++) {
        for (int8_t j = 0; j < 26; j++) {
          for (int8_t k = 0; k < 26; k++) {
            e.resetRotorPositions(i,j,k);
            std::array<int8_t, T> result = e.encrypt(ciphertext);
            float fitness = f.score(result);
            if (fitness > bestKey.score) {
              bestKey = ScoredEnigmaKey( rotors, { i, j, k }, {0,0,0},
                  plugboard);
              bestKey.score = fitness;
            }
          }
        }
      }
      keySet[i] = std::move(bestKey);
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
    int8_t rightRotorIndex = 2, middleRotorIndex = 1;

    // Optimise right rotor
    int8_t optimalIndex = findRingSetting(key, ciphertext, rightRotorIndex, f);
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
  static int8_t findRingSetting(const ScoredEnigmaKey& key, const char* ciphertext, int8_t rotor, FitnessFunction f) {
    auto originalIndicators = key.indicators;
    auto originalRingSettings = key.rings;

    int8_t optimalRingSetting = 0;
    float maxFitness = -1e30f;
    std::string result;
    for (int8_t i = 0; i < 26; i++) {
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
