#include <enigma/Reflector.h>
#include <gtest/gtest.h>

// rotor mapping to self succesfully
TEST(EnigmaReflector, Reflector)
{
	constexpr Reflector id = Reflector::Create('A');

	constexpr ReflectorEncoding result = makeReflectorEncoding("ZYXWVUTSRQPONMLKJIHGFEDCBA");

	for (int i = 0; i < 26; ++i)
	{
		EXPECT_EQ(result[i], id.forward(identityEncoding[i]));
	}
}
