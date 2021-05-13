#ifndef ENIGMA_REFLECTOR_H
#define ENIGMA_REFLECTOR_H

#include <enigma/Rotor.h>

using ReflectorEncoding = std::array<int8_t, 26+26>;

constexpr ReflectorEncoding makeReflectorEncoding(const char* chars)
{
  ReflectorEncoding encoding{};
  for(int8_t i = 0; i < 26; ++i)
    encoding[i] = charToIndex(chars[i]);
  for(int8_t i = 26; i < 26*2; ++i)
    encoding[i] = encoding[i-26];

  return encoding;
}


constexpr ReflectorEncoding ReflectorIdentity = makeReflectorEncoding("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
class Reflector {
public:
    constexpr Reflector(const ReflectorEncoding& encoding)
      : forwardWiring(encoding)
    {
    }

    static constexpr Reflector Create(char name) {
        switch (name) {
            case 'A':
                return Reflector(makeReflectorEncoding("ZYXWVUTSRQPONMLKJIHGFEDCBA"));
            case 'B':
                return Reflector(makeReflectorEncoding("YRUHQSLDPXNGOKMIEBFZCWVJAT"));
            case 'C':
                return Reflector(makeReflectorEncoding("FVPJIAOYEDRZXWGCTKUQSBNMHL"));
            default:
              return Reflector(ReflectorIdentity);
        }
    }

    constexpr int8_t forward(int8_t c) const {
      // c is up to 52 big
      return forwardWiring[c];
    }

private:
    ReflectorEncoding forwardWiring;
};

#endif /* ENIGMA_REFLECTOR_H */
