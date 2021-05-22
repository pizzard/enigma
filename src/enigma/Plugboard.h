#ifndef ENIGMA_PLUGBOARD_H
#define ENIGMA_PLUGBOARD_H

#include <enigma/Reflector.h>
#include <random>

class Plugboard {

public:
    ReflectorEncoding wiring{ReflectorIdentity};
    std::array<bool, 26> plugged{false};
    bool has_plugs{false};

    constexpr Plugboard() = default;
    constexpr Plugboard(std::initializer_list<const char*> list)
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
    template<class Generator>
    Plugboard(int num_plugs, Generator& gen)
      : has_plugs(true)
    {
      std::uniform_int_distribution<int8_t> random_plug_gen(0, 25); // inclusive range!
      // now we have a massively invalid setup which we need to fix up.
      int plugs = 0;
      while(plugs < num_plugs)
      {
        //find a random plug to use
        int8_t random_plug = random_plug_gen(gen);
        // that one is already chosen and fixed, so take the next one.
        while(plugged[random_plug])
        {
          random_plug = (++random_plug)%26;
        }
        int8_t target =  random_plug_gen(gen);
        while(plugged[target] || target == random_plug)
        {
          target = (++target)%26;
        }
        plugged[random_plug] = true;
        plugged[target] = true;
        // create symmetry
        wiring[target] = random_plug;
        wiring[random_plug] = target;
        plugs++;
      }
      for(int8_t i = 26; i < 26*2; ++i)
        wiring[i] = wiring[i-26];

    }

    constexpr Plugboard(const ReflectorEncoding& e): wiring(e){}
    constexpr int8_t forward(int8_t c) const {
        return has_plugs?wiring[c]:c;
    }

    constexpr void addPlug(int8_t first, int8_t second)
    {

      wiring[first] = second;
      wiring[second] = first;
      wiring[first+26] = second;
      wiring[second+26] = first;
      if (plugged[first] || plugged[second])
        throw "already plugged";

      plugged[first] = true;
      plugged[second] = true;
      has_plugs = true;
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
