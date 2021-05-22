#include <enigma/Plugboard.h>
#include <gtest/gtest.h>

#include <random>

TEST(Enigma, IdentityPlugboard)
{
	constexpr Plugboard id{};

	for (char c : identityEncoding)
	{
		EXPECT_EQ(c, id.forward(c));
	}
}

TEST(Enigma, OnePlug)
{
	constexpr Plugboard id{{"AB"}};

	ReflectorEncoding result = makeReflectorEncoding("BACDEFGHIJKLMNOPQRSTUVWXYZ");

	for (int i = 0; i < 26; ++i)
	{
		EXPECT_EQ(result[i], id.forward(identityEncoding[i]));
	}
}
TEST(Enigma, MultiplePlugs)
{
	constexpr Plugboard id{{"AB", "CD", "EF", "GH", "IJ"}};

	ReflectorEncoding result = makeReflectorEncoding("BADCFEHGJIKLMNOPQRSTUVWXYZ");

	for (int i = 0; i < 26; ++i)
	{
		EXPECT_EQ(result[i], id.forward(identityEncoding[i]));
	}

	EXPECT_EQ(id.ToString(), "AB CD EF GH IJ ");
}

// rotor mapping to self succesfully
TEST(Enigma, RandomPlugboard)
{
	std::mt19937 gen(54673765765); // random number generator, fixed seed
	Plugboard id{10, gen};
	EXPECT_EQ(id.ToString(), "AF BU CK DP HZ JQ LT MO NW RS ");
}
