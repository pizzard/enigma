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
  // the selection of a fitness function was patched out during optimization
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

  auto t2 = high_resolution_clock::now();

  for(int i = 0 ; i < 1 ; ++i)
  {
    std::cout << "Current Rotors: "
             << (int)rotorConfigurations[i].rotors[0] << " "
             << (int)rotorConfigurations[i].rotors[1] << " "
             << (int)rotorConfigurations[i].rotors[2 ] << "\n";
    // Next find the best ring settings for the top 10 configurations
    ScoredEnigmaKey ringConfig = EnigmaAnalysis::findRingSettings(rotorConfigurations[i], ciphertext, ioc);

    std::cout << "Best ring - startpos difference:" <<
        (int)ringConfig.rings[0] << " " << (int)ringConfig.rings[1] << " " << (int)ringConfig.rings[2] << "\n";
    {
      const ScoredEnigmaKey& key = ringConfig;
      Enigma e(key.rotors,
          'B',
          key.indicators,
          key.rings,
          key.plugboard);
      std::cout << "Current decryption:" <<
        encryptString(e, ciphertext) << "\n";
    }

    // Next find the best ring settings for the top 10 configurations
    ScoredEnigmaKey startConfig = EnigmaAnalysis::findStartingPositions(ringConfig, ciphertext, bigrams);

    std::cout << "Best starting positions settings:" <<
        (int)startConfig.indicators[0] << " " << (int)startConfig.indicators[1] << " " << (int)startConfig.indicators[2] << "\n";
    std::cout << "Best ring settings:" <<
           (int)startConfig.rings[0] << " " << (int)startConfig.rings[1] << " " << (int)startConfig.rings[2] << "\n";

    {
      const ScoredEnigmaKey& key = startConfig;
      Enigma e(key.rotors,
          'B',
          key.indicators,
          key.rings,
          key.plugboard);
      std::cout << "Current decryption:" <<
        encryptString(e, ciphertext) << "\n";
    }

    // Finally, perform hill climbing to find plugs one at a time
    ScoredEnigmaKey optimalKeyWithPlugs = EnigmaAnalysis::findPlugs(startConfig, 5, ciphertext, kpt);
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
  }

  auto t4 = high_resolution_clock::now();
  std::cout << "RotorConfiguration " << std::chrono::duration<double, std::milli>(t2 - t1).count() << "ms\n";
  std::cout << "Ring Settings + Plugs " << std::chrono::duration<double, std::milli>(t4 - t2).count() << "ms\n";
  return 0;
}
