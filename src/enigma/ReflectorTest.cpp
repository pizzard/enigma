#include <gtest/gtest.h>
#include <enigma/Reflector.h>

// rotor mapping to self succesfully
TEST(Enigma, Reflector) {
  constexpr Reflector id = Reflector::Create('A');

  constexpr ReflectorEncoding result = makeReflectorEncoding("ZYXWVUTSRQPONMLKJIHGFEDCBA");

  for(int i = 0; i < 26; ++i)
  {
    EXPECT_EQ(result[i], id.forward(identityEncoding[i]));
  }

}
