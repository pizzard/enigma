#include <gtest/gtest.h>
#include <enigma/Plugboard.h>

// rotor mapping to self succesfully
TEST(Enigma, IdentityPlugboard) {
  constexpr Plugboard id{};

  for(char c: identityEncoding)
  {
    EXPECT_EQ(c, id.forward(c));
  }

}

// rotor mapping to self succesfully
TEST(Enigma, OnePlug) {
  constexpr Plugboard id{{"AB"}};

  ReflectorEncoding result = makeReflectorEncoding("BACDEFGHIJKLMNOPQRSTUVWXYZ");

  for(int i = 0; i < 26; ++i)
  {
    EXPECT_EQ(result[i], id.forward(identityEncoding[i]));
  }

}
// rotor mapping to self succesfully
TEST(Enigma, MultiplePlugs) {
  constexpr Plugboard id{{"AB", "CD", "EF", "GH", "IJ"}};

  ReflectorEncoding result = makeReflectorEncoding("BADCFEHGJIKLMNOPQRSTUVWXYZ");

  for(int i = 0; i < 26; ++i)
  {
    EXPECT_EQ(result[i], id.forward(identityEncoding[i]));
  }

  EXPECT_EQ(id.ToString(), "AB CD EF GH IJ ");
}
