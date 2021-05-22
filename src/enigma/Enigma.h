#ifndef ENIGMA_ENIGMA_H
#define ENIGMA_ENIGMA_H

#include <enigma/Plugboard.h>
#include <enigma/Reflector.h>
#include <enigma/Rotor.h>

#include <cstring>

class Enigma
{
	// 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
	// A B C D E F G H I J K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
	Rotor leftRotor;
	Rotor middleRotor;
	Rotor rightRotor;
	Reflector reflector;
	Plugboard plugboard;

	// as the left rotor nearly never turns, lets cache the rotor -> plugboard -> rotor
	Reflector leftRotorPlusReflector;

public:
	Enigma(std::array<int8_t, 3> rotors,
			char reflectorId,
			std::array<int8_t, 3> rotorPositions,
			std::array<int8_t, 3> ringSettings,
			const Plugboard& plugboardConnections)
		: leftRotor(Rotor::Create(rotors[0], rotorPositions[0], ringSettings[0]))
		, middleRotor(Rotor::Create(rotors[1], rotorPositions[1], ringSettings[1]))
		, rightRotor(Rotor::Create(rotors[2], rotorPositions[2], ringSettings[2]))
		, reflector(Reflector::Create(reflectorId))
		, plugboard(plugboardConnections)
		, leftRotorPlusReflector(createCombinedReflector())
	{}

	constexpr Reflector createCombinedReflector()
	{
		ReflectorEncoding encoding{};
		for (int8_t i = 0; i < 26; ++i)
		{
			const int8_t c3 = leftRotor.forward(identityEncoding[i]);
			const int8_t c4 = reflector.forward(c3);
			const int8_t c5 = leftRotor.backward(c4);
			encoding[i] = c5;
		}
		for (int8_t i = 26; i < 26 * 2; ++i)
			encoding[i] = encoding[i - 26];

		return Reflector{encoding};
	}

	constexpr void resetRotorPositions(int8_t a, int8_t b, int8_t c)
	{
		leftRotor.resetPosition(a);
		middleRotor.resetPosition(b);
		rightRotor.resetPosition(c);
		leftRotorPlusReflector = createCombinedReflector();
	}

	constexpr void rotate()
	{
		// If middle rotor notch - double-stepping
		if (middleRotor.isAtNotch())
		{
			middleRotor.turnover();
			leftRotor.turnover();
			leftRotorPlusReflector = createCombinedReflector();
		}
		// If left-rotor notch
		else if (rightRotor.isAtNotch())
		{
			middleRotor.turnover();
		}

		// Increment right-most rotor
		rightRotor.turnover();
	}

	constexpr int8_t encrypt(int8_t c)
	{
		rotate();

		// Plugboard in
		c = plugboard.forward(c);

		// Right to left
		const int8_t c1 = rightRotor.forward(c);
		const int8_t c2 = middleRotor.forward(c1);
		const int8_t c5 = leftRotorPlusReflector.forward(c2);
		const int8_t c6 = middleRotor.backward(c5);
		const int8_t c7 = rightRotor.backward(c6);

		// Plugboard out
		return plugboard.forward(c7);
	}

	template<size_t sz>
	constexpr std::array<int8_t, sz> encrypt(const std::array<int8_t, sz>& input)
	{
		std::array<int8_t, sz> out{};
		for (int i = 0; i < sz; i++)
			out[i] = encrypt(input[i]);
		return out;
	}
};

inline std::string encryptString(Enigma& e, const char* input)
{
	int sz = std::strlen(input);
	std::string out;
	out.resize(sz);
	for (int i = 0; i < sz; i++)
	{
		out[i] = indexToChar(e.encrypt(charToIndex(input[i])));
	}
	return out;
}

#endif /* ENIGMA_ENIGMA_H */
