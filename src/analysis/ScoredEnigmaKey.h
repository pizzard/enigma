#ifndef ANALYSIS_SCOREDENIGMAKEY_H
#define ANALYSIS_SCOREDENIGMAKEY_H

#include <array>
#include <enigma/Plugboard.h>

class ScoredEnigmaKey
{
public:
	std::array<int8_t, 3> rotors{0, 0, 0};
	std::array<int8_t, 3> indicators{0, 0, 0};
	std::array<int8_t, 3> rings{0, 0, 0};
	Plugboard plugboard;
	float score{1e-10};

	constexpr ScoredEnigmaKey() = default;
	constexpr ScoredEnigmaKey(std::array<int8_t, 3> rotors,
			std::array<int8_t, 3> indicators,
			std::array<int8_t, 3> rings,
			Plugboard plugboardConnections)
		: rotors(rotors), indicators(indicators), rings(rings), plugboard(plugboardConnections)
	{}
};

#endif
