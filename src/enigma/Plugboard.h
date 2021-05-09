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
        int first = charToIndex(plug[0]);
        int second = charToIndex(plug[1]);
        addPlug(first, second);
      }
    }

    constexpr Plugboard(const Encoding& e): wiring(e){}
    constexpr int forward(int c) const {
        return wiring[c];
    }

    constexpr void addPlug(int first, int second)
    {
      wiring[first] = second;
      wiring[second] = first;
      if (plugged[first] || plugged[second])
        throw "already plugged";

      plugged[first] = true;
      plugged[second] = true;
    }

    std::string ToString() const
    {
      std::string s = "";
      for(int i = 0; i < wiring.size(); ++i)
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
