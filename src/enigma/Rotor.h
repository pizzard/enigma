#ifndef ENIGMA_ROTOR_H
#define ENIGMA_ROTOR_H

#include <array>

using Encoding = std::array<int, 26+26+26>;

constexpr int charToIndex(const char k)
{
  return k - 65;
}
constexpr char indexToChar(const int k)
{

  return (k%26) + 65;
}

constexpr Encoding makeEncoding(const char* chars)
{
  Encoding encoding{};
  for(int i = 0; i < 26; ++i)
    encoding[i] = charToIndex(chars[i]);
  for(int i = 26; i < 26*2; ++i)
    encoding[i] = encoding[i-26];
  for(int i = 52; i < 26*3; ++i)
    encoding[i] = encoding[i-26];

  return encoding;
}
constexpr Encoding identityEncoding =  makeEncoding("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

struct RotorMapping{

  int rotorNum;
  Encoding forwardWiring;
  Encoding backwardWiring;

  constexpr RotorMapping(int num, const Encoding& forward)
    : rotorNum(num)
    , forwardWiring(forward)
    , backwardWiring(inverseWiring(forward))
  {

  }
private:
  constexpr Encoding inverseWiring(const Encoding& wiring) const {
      Encoding inverse{};
      for (int i = 0; i < 26; i++) {
          int forward = wiring[i];
          inverse[forward] = i;
      }
      for(int i = 26; i < 26*2; ++i)
        inverse[i] = inverse[i-26];
      for(int i = 52; i < 26*3; ++i)
        inverse[i] = inverse[i-26];

      return inverse;
  }
};

class Rotor {
    const RotorMapping& mapping;
  //  int rotorPosition;
    int notchPosition;
    int secondaryNotchPosition;
    int ringSetting;
    int currentRotorShift;
    int adjustedNotchPosition;
    int adjustedSecondaryNotchPosition;

public:
    constexpr Rotor(const RotorMapping& mapping, int rotorPosition, int notchPosition, int secondaryNotchPosition, int ringSetting)
      : mapping(mapping)
    //  , rotorPosition(rotorPosition)
      , notchPosition(notchPosition)
      , secondaryNotchPosition(secondaryNotchPosition)
      , ringSetting(ringSetting)
      , currentRotorShift((rotorPosition  + 26 - ringSetting ) %26 )
      , adjustedNotchPosition((notchPosition + 26 - ringSetting) %26)
      , adjustedSecondaryNotchPosition(secondaryNotchPosition == -1 ? -1
          :(secondaryNotchPosition + 26 - ringSetting) %26)
    {
    }

    static constexpr RotorMapping map1{1, makeEncoding("EKMFLGDQVZNTOWYHXUSPAIBRCJ")};
    static constexpr RotorMapping map2{2, makeEncoding("AJDKSIRUXBLHWTMCQGZNPYFVOE")};
    static constexpr RotorMapping map3{3, makeEncoding("BDFHJLCPRTXVZNYEIWGAKMUSQO")};
    static constexpr RotorMapping map4{4, makeEncoding("ESOVPZJAYQUIRHXLNFTGKDCMWB")};
    static constexpr RotorMapping map5{5, makeEncoding("VZBRGITYUPSDNHLXAWMJQOFECK")};
    static constexpr RotorMapping map6{6, makeEncoding("JPGVOUMFYQBENHZRDKASXLICTW")};
    static constexpr RotorMapping map7{7, makeEncoding("NZJHGRCXMYSWBOUFAIVLPEKQDT")};
    static constexpr RotorMapping map8{8, makeEncoding("FKQHTLXOCBJSPDZRAMEWNIUYGV")};
    static constexpr RotorMapping map0{0, identityEncoding};

    static constexpr Rotor Create(int rotorNum, int rotorPosition, int ringSetting)
    {
      if(rotorNum ==  1)
          return Rotor(map1, rotorPosition, 16, -1,  ringSetting);
      if(rotorNum ==  2)
          return Rotor(map2 , rotorPosition, 4, -1, ringSetting);
      if(rotorNum ==  3)
          return Rotor(map3, rotorPosition, 21, -1, ringSetting);
      if(rotorNum ==  4)
          return Rotor(map4, rotorPosition, 9, -1, ringSetting);
      if(rotorNum ==  5)
          return Rotor(map5, rotorPosition, 25, -1, ringSetting);
      if(rotorNum ==  6)
          return Rotor(map6, rotorPosition, 25, 12, ringSetting);
      if(rotorNum == 7)
          return Rotor(map7, rotorPosition, 25, 12, ringSetting);
      if(rotorNum == 8)
          return Rotor(map8, rotorPosition, 25, 12, ringSetting);
      else
          return Rotor(map0, rotorPosition, 0, -1, ringSetting);
    }

    constexpr int getName() const {
        return mapping.rotorNum;
    }

    constexpr int forward(int c)  const {
        return encipher(c, mapping.forwardWiring);
    }

    constexpr int backward(int c) const {
        return encipher(c, mapping.backwardWiring);
    }

    constexpr bool isAtNotch() {
      return adjustedNotchPosition == currentRotorShift
          || adjustedSecondaryNotchPosition == currentRotorShift;
    }

    constexpr void turnover() {
        if(++currentRotorShift >= 26)
            currentRotorShift = 0;
    }

private:
    // return value can go up to 52 to avoid modulo operations
    constexpr int encipher(int k, const Encoding& mapping) const {
        const int shift = currentRotorShift;
        // as we have a 78 character wide landing pad
        // and the upper limit for k is 52 and for shift 26
        const int mapped_pos = (k + shift);
        return mapping[mapped_pos] +26 - shift;
    }

};


#endif /* ENIGMA_ROTOR_H */
