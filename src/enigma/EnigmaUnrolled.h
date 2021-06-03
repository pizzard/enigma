#ifndef ENIGMA_ENIGMA_UNROLLED_H
#define ENIGMA_ENIGMA_UNROLLED_H

#include <enigma/Plugboard.h>
#include <enigma/Reflector.h>
#include <enigma/Rotor.h>
#include <analysis/ScoredEnigmaKey.h>

#include <cstring>
#include <cassert>

class EnigmaUnrolled
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
	EnigmaUnrolled(std::array<int8_t, 3> rotors,
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

	template<class Array>
	constexpr void precomputeRightRotorOffsets(int8_t shift, Array& offsets) const
	{
		for (int8_t& offset: offsets)
		{
			if(++shift >= 26)
				shift = 0;
			offset = shift;
		}
	}

	template<class Iter, class Array>
	constexpr void precomputeMiddleRotorOffsets(Iter right_offset,
			Array& middle_offsets, Array& middle_turnovers) const
	{
		int8_t middleShift = middleRotor.currentRotorShift;
		for (auto it = middle_offsets.begin(),
				to_it = middle_turnovers.begin();
				it != middle_offsets.end();
				++it,
				++to_it,
				++right_offset)
		{
			if(rightRotor.isOneAfterNotch(*right_offset))
			{
				if(++middleShift >= 26)
					middleShift = 0;
			}
			else if(middleRotor.isANotchPosition(middleShift))
			{
				if(++middleShift >= 26)
					middleShift = 0;
				*to_it = 1;
			}
			*it =  middleShift;
		}
	}

	template<class Iter>
	constexpr void applyRotor(Iter begin, Iter end, Iter offset_begin, const Encoding& wiring) const
	{
		for (Iter it = begin; it != end; ++it, ++offset_begin)
		{
			const int8_t mapped_pos = (*it + *offset_begin);
			*it =  wiring[mapped_pos] - *offset_begin;
		}

	}

	template<int sz, class FitnessFunction>
	constexpr ScoredEnigmaKey tryAllPositions(std::array<int8_t, sz> ciphertext,
			FitnessFunction f)
	{
		std::array<int8_t, sz> ciphertext_with_plugs = ciphertext;
		// Plugboard in
		plugboard.forward(ciphertext_with_plugs.begin(), ciphertext_with_plugs.end());
		float maxFitness = -1e30f;
		ScoredEnigmaKey bestKey;
		for (int8_t i = 0; i < 26; i += 1)
		{
			rightRotor.resetStartingPosition(i);

			// precompute right rotor offsets;
			std::array<int8_t, sz> right_rotor_offsets{};
			// the shift if only equal i here because we know the ring settings are 0.
			precomputeRightRotorOffsets(rightRotor.currentRotorShift, right_rotor_offsets);

			// apply right rotor
			std::array<int8_t, sz> right_rotor_fw = ciphertext_with_plugs;
			applyRotor(right_rotor_fw.begin(), right_rotor_fw.end(),
					right_rotor_offsets.begin(),  rightRotor.mapping.forwardWiring);

			for (int8_t j = 0; j < 26; j++)
			{
				middleRotor.resetStartingPosition(j);
				std::array<int8_t, sz> middle_rotor_offsets{};
				std::array<int8_t, sz> middle_rotor_turnovers{0};
				precomputeMiddleRotorOffsets(right_rotor_offsets.begin(), middle_rotor_offsets,
						middle_rotor_turnovers);
				std::array<int8_t, sz> middle_rotor_applied = right_rotor_fw;
				applyRotor(middle_rotor_applied.begin(),  middle_rotor_applied.end(),
						middle_rotor_offsets.begin(), middleRotor.mapping.forwardWiring);

				for (int8_t k = 0; k < 26; k += 1)
				{
					leftRotor.resetStartingPosition(k);

					std::array<int8_t, sz> result = middle_rotor_applied;
					auto begin = result.begin();
					auto end = result.end();

					int8_t right_shift = rightRotor.currentRotorShift;
					int8_t middleShift = middleRotor.currentRotorShift;

					for (auto it = begin,
							mrto = middle_rotor_turnovers.begin();
							it != end;
							++it,
							++mrto)
					{
						if(*mrto == 1)
						{
							leftRotor.turnover();
							leftRotorPlusReflector = createCombinedReflector();
						}
						*it = leftRotorPlusReflector.forward(*it);
					}


					applyRotor(begin, end,
							middle_rotor_offsets.begin(),  middleRotor.mapping.backwardWiring);

					// apply right rotor
					applyRotor(begin, end,
							right_rotor_offsets.begin(),  rightRotor.mapping.backwardWiring);


					// Plugboard out
					 plugboard.forward(begin, end);

					float fitness = f.score(result);
					if (fitness > bestKey.score)
					{
						bestKey = ScoredEnigmaKey({0,0,0}, {i, j, k}, {0, 0, 0}, Plugboard{});
						bestKey.score = fitness;
					}
				}
			}
		}
		return bestKey;
	}


	template<class Iter>
	constexpr void encrypt(Iter begin, Iter end)
	{
		plugboard.forward(begin, end);

		std::vector<int8_t> right_rotor_offsets;
		right_rotor_offsets.resize(std::distance(begin,end));
		precomputeRightRotorOffsets(rightRotor.currentRotorShift, right_rotor_offsets);
		applyRotor(begin, end, right_rotor_offsets.begin(),  rightRotor.mapping.forwardWiring);

		const int8_t rightStartPos = rightRotor.currentRotorShift;
		const int8_t middleStartPos = middleRotor.currentRotorShift;

		std::vector<int8_t> middle_rotor_offsets;
		middle_rotor_offsets.resize(std::distance(begin,end));
		std::vector<int8_t> middle_rotor_turnovers(std::distance(begin,end), 0);
		precomputeMiddleRotorOffsets(right_rotor_offsets.begin(),
				middle_rotor_offsets, middle_rotor_turnovers);

		applyRotor(begin, end, middle_rotor_offsets.begin(), middleRotor.mapping.forwardWiring);

		int right_shift = rightStartPos;
		int middleShift = middleStartPos;
		for (auto it = begin; it != end; ++it)
		{
			if (rightRotor.isANotchPosition(right_shift))
			{
				if (++middleShift >= 26)
					middleShift = 0;
			}
			else if (middleRotor.isANotchPosition(middleShift))
			{
				if (++middleShift >= 26)
					middleShift = 0;
				leftRotor.turnover();
				leftRotorPlusReflector = createCombinedReflector();
			}
			if (++right_shift >= 26)
				right_shift = 0;
			*it = leftRotorPlusReflector.forward(*it);
		}

		applyRotor(begin, end, middle_rotor_offsets.begin(), middleRotor.mapping.backwardWiring);
		applyRotor(begin, end, right_rotor_offsets.begin(), rightRotor.mapping.backwardWiring);
		// Plugboard out
		plugboard.forward(begin, end);
	}

};

inline std::string encryptString(EnigmaUnrolled& e, const char* input)
{
	int sz = std::strlen(input);

	std::vector<int8_t> vector(sz);

	for (int i = 0; i < sz; i++)
		vector[i] = charToIndex(input[i]);

	e.encrypt(vector.begin(), vector.end());

	std::string out;
	out.resize(sz);
	for (int i = 0; i < sz; i++)
		out[i] = indexToChar(vector[i]);

	return out;
}


#endif /* ENIGMA_ENIGMA_UNROLLED_H */
