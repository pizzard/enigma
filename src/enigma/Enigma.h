#ifndef ENIGMA_ENIGMA_H
#define ENIGMA_ENIGMA_H

#include <enigma/Rotor.h>
#include <enigma/Reflector.h>
#include <enigma/Plugboard.h>

#include <cstring>

class Enigma {
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
    // A B C D E F G H I J K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
    Rotor leftRotor;
    Rotor middleRotor;
    Rotor rightRotor;
    Reflector reflector;
    Plugboard plugboard;


public:
    Enigma(std::array<int, 3> rotors,
        char reflectorId,
        std::array<int, 3> rotorPositions,
        std::array<int, 3> ringSettings,
        Plugboard plugboardConnections)
         : leftRotor(Rotor::Create(rotors[0], rotorPositions[0], ringSettings[0]))
         , middleRotor(Rotor::Create(rotors[1], rotorPositions[1], ringSettings[1]))
         , rightRotor(Rotor::Create(rotors[2], rotorPositions[2], ringSettings[2]))
         , reflector(Reflector::Create(reflectorId))
         , plugboard(std::move(plugboardConnections))
    {
    }


//    public Enigma(EnigmaKey key) {
//        this(key.rotors, "B", key.indicators, key.rings, key.plugboard);
//    }

    constexpr void rotate() {
        // If middle rotor notch - double-stepping
        if (middleRotor.isAtNotch()) {
            middleRotor.turnover();
            leftRotor.turnover();
        }
        // If left-rotor notch
        else if (rightRotor.isAtNotch()) {
            middleRotor.turnover();
        }

        // Increment right-most rotor
        rightRotor.turnover();
    }

    constexpr int encrypt(int c) {
        rotate();

        // Plugboard in
        c = plugboard.forward(c);

        // Right to left
        char c1 = rightRotor.forward(c);
        char c2 = middleRotor.forward(c1);
        char c3 = leftRotor.forward(c2);

        // Reflector
        char c4 = reflector.forward(c3);

        // Left to right
        char c5 = leftRotor.backward(c4);
        char c6 = middleRotor.backward(c5);
        char c7 = rightRotor.backward(c6);

        // Plugboard out
        c7 = plugboard.forward(c7);

        return c7;
    }

    // encrypts already converted integer sequences
    template<int T>
    constexpr std::array<int, T> encrypt(const std::array<int, T>& input) {
      std::array<char, T> out;
        for (int i = 0; i < T; i++) {
          out[i] = encrypt(input[i]);
        }
        return out;
    }
};


inline std::string encryptString(Enigma& e, const char * input) {
  int sz = std::strlen(input);
  std::string out;
  out.resize(sz);
    for (int i = 0; i < sz; i++) {
      out[i] = indexToChar(e.encrypt(charToIndex(input[i])));
    }
    return out;
}
#endif /* ENIGMA_ENIGMA_H */
