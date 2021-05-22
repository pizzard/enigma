#ifndef ANALYSIS_KNOWNPLAINTEXTFITNESS_H
#define ANALYSIS_KNOWNPLAINTEXTFITNESS_H

#include <algorithm>
#include <string>

class KnownPlaintextFitness
{
	std::string plaintext;

public:
	KnownPlaintextFitness(const std::string& p) : plaintext(p) {}

	float score(const std::string& text) const
	{
		int length = std::min(plaintext.size(), text.size());
		int total = 0;
		for (int i = 0; i < length; i++)
		{
			if (plaintext[i] > 0)
			{
				total += plaintext[i] == text[i] ? 1 : 0;
			}
		}
		return total;
	}
};

#endif
