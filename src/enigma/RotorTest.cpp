#include <gtest/gtest.h>
#include <enigma/Rotor.h>

// rotor mapping to self succesfully
TEST(Enigma, IdentityRotor) {
  constexpr Rotor id = Rotor::Create(0, 0, 0);

  for(int i = 0; i < 26; ++i)
  {
    EXPECT_EQ(indexToChar(identityEncoding[i]), indexToChar(id.forward(identityEncoding[i])));
    EXPECT_EQ(indexToChar(identityEncoding[i]), indexToChar(id.backward(identityEncoding[i])));
  }

}

TEST(Enigma, FirstRotor) {
  constexpr Rotor id = Rotor::Create(1, 0, 0);

  constexpr Encoding rotorwiring = makeEncoding("EKMFLGDQVZNTOWYHXUSPAIBRCJ");

  for(int i = 0; i < 26; ++i)
  {
    EXPECT_EQ(indexToChar(rotorwiring[i]), indexToChar(id.forward(identityEncoding[i])));
    EXPECT_EQ(indexToChar(identityEncoding[i]), indexToChar(id.backward(rotorwiring[i])));
  }

}
