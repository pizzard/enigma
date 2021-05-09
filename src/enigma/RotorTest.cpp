#include <gtest/gtest.h>
#include <enigma/Rotor.h>

// rotor mapping to self succesfully
TEST(Enigma, IdentityRotor) {
  constexpr Rotor id = Rotor::Create(0, 0, 0);

  for(int c: identityEncoding)
  {
    EXPECT_EQ(c, id.forward(c));
    EXPECT_EQ(c, id.backward(c));
  }

}

TEST(Enigma, FirstRotor) {
  constexpr Rotor id = Rotor::Create(1, 0, 0);

  constexpr Encoding rotorwiring = makeEncoding("EKMFLGDQVZNTOWYHXUSPAIBRCJ");

  for(int i = 0; i < 26; ++i)
  {
    EXPECT_EQ(rotorwiring[i], id.forward(identityEncoding[i]));
    EXPECT_EQ(identityEncoding[i], id.backward(rotorwiring[i]));
  }

}
