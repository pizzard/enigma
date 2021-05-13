#include <analysis/EnigmaAnalysis.h>
#include <analysis/fitness/IoCFitness.h>
#include <analysis/fitness/KnownPlaintextFitness.h>
#include <analysis/fitness/BigramFitness.h>
#include <analysis/fitness/QuadgramFitness.h>

#include <chrono>
#include <iostream>

int main() {
  using std::chrono::high_resolution_clock;

  IoCFitness ioc;
  BigramFitness bigrams;
  QuadgramFitness quadgrams;
  KnownPlaintextFitness kpt("IPROPOSETOCONSIDERTHEQUESTIONCANMACHINESTHINKTHISSHOULDBEGINWITHDEFINITIONSOFTHEMEANINGOFTHETERMSMACHINEANDTHINKTHEDEFINITIONSMIGHTBEFRAMEDSOASTOREFLECTSOFARASPOSSIBLETHENORMALUSEOFTHEWORDSBUTTHISATTITUDEISDANGEROUSIFTHEMEANINGOFTHEWORDSMACHINEANDTHINKARETOBEFOUNDBYEXAMININGHOWTHEYARECOMMONLYUSEDITISDIFFICULTTOESCAPETHECONCLUSIONTHATTHEMEANINGANDTHEANSWERTOTHEQUESTIONCANMACHINESTHINKISTOBESOUGHTINASTATISTICALSURVEYSUCHASAGALLUPPOLLBUTTHISISABSURDINSTEADOFATTEMPTINGSUCHADEFINITIONISHALLREPLACETHEQUESTIONBYANOTHERWHICHISCLOSELYRELATEDTOITANDISEXPRESSEDINRELATIVELYUNAMBIGUOUSWORDS");
  auto t1 = high_resolution_clock::now();

  // For those interested, these were the original settings
  // II V III / 7 4 19 / 12 2 20 / AF TV KO BL RW
  const char* ciphertext = "OZLUDYAKMGMXVFVARPMJIKVWPMBVWMOIDHYPLAYUWGBZFAFAFUQFZQISLEZMYPVBRDDLAGIHIFUJDFADORQOOMIZPYXDCBPWDSSNUSYZTJEWZPWFBWBMIEQXRFASZLOPPZRJKJSPPSTXKPUWYSKNMZZLHJDXJMMMDFODIHUBVCXMNICNYQBNQODFQLOGPZYXRJMTLMRKQAUQJPADHDZPFIKTQBFXAYMVSZPKXIQLOQCVRPKOBZSXIUBAAJBRSNAFDMLLBVSYXISFXQZKQJRIQHOSHVYJXIFUZRMXWJVWHCCYHCXYGRKMKBPWRDBXXRGABQBZRJDVHFPJZUSEBHWAEOGEUQFZEEBDCWNDHIAQDMHKPRVYHQGRDYQIOEOLUBGBSNXWPZCHLDZQBWBEWOCQDBAFGUVHNGCIKXEIZGIZHPJFCTMNNNAUXEVWTWACHOLOLSLTMDRZJZEVKKSSGUUTHVXXODSKTFGRUEIIXVWQYUIPIDBFPGLBYXZTCOQBCAHJYNSGDYLREYBRAKXGKQKWJEKWGAPTHGOMXJDSQKYHMFGOLXBSKVLGNZOAXGVTGXUIVFTGKPJU";
  std::array<int8_t, 584> msg;
  for(int i = 0; i < msg.size(); ++i)
  {
    msg[i] = charToIndex(ciphertext[i]);
  }

  // Begin by finding the best combination of rotors and start positions (returns top n)
  std::vector<ScoredEnigmaKey> rotorConfigurations = EnigmaAnalysis::findRotorConfiguration<584>(
      msg,
      EnigmaAnalysis::AvailableRotors::FIVE,
      Plugboard{},
      ioc);

  std::cout << "\nTop 10 rotor configurations: \n";
  for (const ScoredEnigmaKey& key : rotorConfigurations) {
    std::cout <<
        (int)key.rotors[0] << " " << (int)key.rotors[1] << " " <<  (int)key.rotors[2 ] << " / " <<
        (int)key.indicators[0] << " " << (int)key.indicators[1] << " " << (int)key.indicators[2] << " / " <<
        key.score << "\n";
  }
  {
    const ScoredEnigmaKey& key = rotorConfigurations[0];
    Enigma e(key.rotors,
        'B',
        key.indicators,
        key.rings,
        key.plugboard);
    std::cout << "Current decryption: " <<
        encryptString(e, ciphertext) << "\n";
  }

  // Next find the best ring settings for the best configuration (index 0)
  ScoredEnigmaKey rarConfig = EnigmaAnalysis::findRingSettings(rotorConfigurations[0], ciphertext, bigrams);

  std::cout << "Best ring settings:" <<
      (int)rarConfig.rings[0] << " " << (int)rarConfig.rings[1] << " " << (int)rarConfig.rings[2] << "\n";
  {
    const ScoredEnigmaKey& key = rarConfig;
    Enigma e(key.rotors,
        'B',
        key.indicators,
        key.rings,
        key.plugboard);
    std::cout << "Current decryption:" <<
      encryptString(e, ciphertext) << "\n";
  }

  // Finally, perform hill climbing to find plugs one at a time
  ScoredEnigmaKey optimalKeyWithPlugs = EnigmaAnalysis::findPlugs(rarConfig, 5, ciphertext, kpt);
  std::cout << "Best plugboard: " << optimalKeyWithPlugs.plugboard.ToString() << "\n";
  {
    const ScoredEnigmaKey& key = optimalKeyWithPlugs;
    Enigma e(key.rotors,
        'B',
        key.indicators,
        key.rings,
        key.plugboard);
    std::cout << "Current decryption:" <<
      encryptString(e, ciphertext) << "\n";
  }

  auto t2 = high_resolution_clock::now();
  /* Getting number of milliseconds as a double. */
  std::chrono::duration<double, std::milli> ms_double = t2 - t1;
  std::cout << "Total execution time " << ms_double.count() << "ms";

  return 0;
}
