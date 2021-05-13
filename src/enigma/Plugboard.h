#ifndef ENIGMA_PLUGBOARD_H
#define ENIGMA_PLUGBOARD_H

#include <enigma/Rotor.h>

class Plugboard {

public:
    Encoding wiring{identityEncoding};
    std::array<bool, 26> plugged{false};

    constexpr Plugboard() = default;
    constexpr Plugboard(std::initializer_list<const char*> list)
      : wiring(identityEncoding)
    {
      for(const char * plug : list)
      {
        // little error checking here, we assume always
        // two characters follows by a space.
        // if that doesn't hold up, all bets are off.
        int8_t first = charToIndex(plug[0]);
        int8_t second = charToIndex(plug[1]);
        addPlug(first, second);
      }
    }

    constexpr Plugboard(const Encoding& e): wiring(e){}
    constexpr int8_t forward(int8_t c) const {
        return wiring[c];
    }

    constexpr void addPlug(int8_t first, int8_t second)
    {
      wiring[first] = second;
      wiring[second] = first;
      wiring[first+26] = second;
      wiring[second+26] = first;
      wiring[first+52] = second;
      wiring[second+52] = first;
      if (plugged[first] || plugged[second])
        throw "already plugged";

      plugged[first] = true;
      plugged[second] = true;
    }

    std::string ToString() const
    {
      std::string s = "";
      for(int8_t i = 0; i < 26; ++i)
      {
        if(wiring[i] <= i)
          continue;
        s += indexToChar(i);
        s += indexToChar(wiring[i]);
        s += ' ';
      }
      return s;
    }
};

#endif /* ENIGMA_PLUGBOARD_H */
