#ifndef ENIGMA_REFLECTOR_H
#define ENIGMA_REFLECTOR_H


#include <enigma/Rotor.h>

class Reflector {
public:
    constexpr Reflector(const Encoding& encoding)
      : forwardWiring(encoding)
    {
    }

    static constexpr Reflector Create(char name) {
        switch (name) {
            case 'A':
                return Reflector(makeEncoding("ZYXWVUTSRQPONMLKJIHGFEDCBA"));
            case 'B':
                return Reflector(makeEncoding("YRUHQSLDPXNGOKMIEBFZCWVJAT"));
            case 'C':
                return Reflector(makeEncoding("FVPJIAOYEDRZXWGCTKUQSBNMHL"));
            default:
              return Reflector(identityEncoding);
        }
    }

    constexpr int forward(int c) const {
        return forwardWiring[c];
    }

private:
    Encoding forwardWiring;
};

#endif /* ENIGMA_REFLECTOR_H */
