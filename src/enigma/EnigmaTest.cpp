#include <enigma/Enigma.h>
#include <gtest/gtest.h>

#include <random>

TEST(Enigma, BasicEngimaTesting)
{
	Enigma e({1, 2, 3}, 'B', {0, 0, 0}, {0, 0, 0}, {});
	const char* input = "ABCDEFGHIJKLMNOPQRSTUVWXYZAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char* output = "BJELRQZVJWARXSNBXORSTNCFMEYHCXTGYJFLINHNXSHIUNTHEORXOPLOVFEKAGADSPNPCMHRVZCYECDAZIHVYGPITMSRZKGGHLSRBLHL";
	std::string ciphertext = encryptString(e, input);
	EXPECT_EQ(output, ciphertext);
}

TEST(Enigma, VariedRotors)
{
	Enigma e({7, 5, 4}, 'B', {10, 5, 12}, {1, 2, 3}, {});
	const char* input = "ABCDEFGHIJKLMNOPQRSTUVWXYZAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string ciphertext = encryptString(e, input);
	const char* output = "FOTYBPKLBZQSGZBOPUFYPFUSETWKNQQHVNHLKJZZZKHUBEJLGVUNIOYSDTEZJQHHAOYYZSENTGXNJCHEDFHQUCGCGJBURNSEDZSEPLQP";
	EXPECT_EQ(output, ciphertext);
}

TEST(Enigma, LongInput)
{
	Enigma e({3, 6, 8}, 'B', {3, 5, 9}, {11, 13, 19}, {});
	std::string aaa;
	aaa.resize(500);
	for (int i = 0; i < 500; i++)
		aaa[i] = 'A';
	std::string ciphertext = encryptString(e, aaa.c_str());
	const char* output =
			"YJKJMFQKPCUOCKTEZQVXYZJWJFROVJMWJVXRCQYFCUVBRELVHRWGPYGCHVLBVJEVTTYVMWKJFOZHLJEXYXRDBEVEHVXKQSBPYZN"
			"IQDCBGTDDWZQWLHIBQNTYPIEBMNINNGMUPPGLSZCBRJULOLNJSOEDLOBXXGEVTKCOTTLDZPHBUFKLWSFSRKOMXKZELBDJNRUDUCO"
			"TNCGLIKVKMHHCYDEKFNOECFBWRIEFQQUFXKKGNTSTVHVITVHDFKIJIHOGMDSQUFMZCGGFZMJUKGDNDSNSJKWKENIRQKSUUHJYMIG"
			"WWNMIESFRCVIBFSOUCLBYEEHMESHSGFDESQZJLTORNFBIFUWIFJTOPVMFQCFCFPYZOJFQRFQZTTTOECTDOOYTGVKEWPSZGHCTQRP"
			"GZQOVTTOIEGGHEFDOVSUQLLGNOOWGLCLOWSISUGSVIHWCMSIUUSBWQIGWEWRKQFQQRZHMQJNKQTJFDIJYHDFCWTHXUOOCVRCVYOHL";
	EXPECT_EQ(output, ciphertext);
}

TEST(Enigma, DecryptTest)
{
	std::mt19937 gen(176); // random number generator, fixed seed

	// Used to generate a random number in specific range [a, b]:
	std::uniform_int_distribution<int8_t> random_rotor(1, 8); // inclusive range!

	std::array<int8_t, 8> allRotors{1, 2, 3, 4, 5, 6, 7, 8};
	std::uniform_int_distribution<int8_t> random_char(0, 25); // inclusive range!
	std::string input;
	input.resize(1000);
	for (int i = 0; i < 1000; i++)
	{
		input[i] = (char)(random_char(gen) + 65);
	}

	for (int test = 0; test < 10; test++)
	{
		// Random initialisation
		std::array<int8_t, 3> rotors{random_rotor(gen), random_rotor(gen), random_rotor(gen)};

		std::array<int8_t, 3> startingPositions{random_char(gen), random_char(gen), random_char(gen)};
		std::array<int8_t, 3> ringSettings{random_char(gen), random_char(gen), random_char(gen)};

		// Machine 1 - Encryption
		Enigma e1{rotors, 'B', startingPositions, ringSettings, {}};
		std::string ciphertext = encryptString(e1, input.c_str());

		// Machine 2 - Decryption
		Enigma e2{rotors, 'B', startingPositions, ringSettings, {}};
		std::string plaintext = encryptString(e2, ciphertext.c_str());

		EXPECT_EQ(input, plaintext);
	}
}

TEST(Enigma, PlugBoardTest4)
{
	// Simple test - 4 plugs
	Enigma e({1, 2, 3}, 'B', {0, 0, 0}, {0, 0, 0}, {"AC", "FG", "JY", "LW"});
	const char* input = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
	std::string output = encryptString(e, input);
	const char* expectedOutput = "QREBNMCYZELKQOJCGJVIVGLYEMUPCURPVPUMDIWXPPWROOQEGI";
	EXPECT_EQ(expectedOutput, output);
}

TEST(Enigma, PlugBoardTest6)
{
	// Simple test - 6 plugs
	Enigma e({4, 6, 3}, 'B', {0, 10, 6}, {0, 0, 0}, {"BM", "DH", "RS", "KN", "GZ", "FQ"});
	const char* input = "WRBHFRROSFHBCHVBENQFAGNYCGCRSTQYAJNROJAKVKXAHGUZHZVKWUTDGMBMSCYQSKABUGRVMIUOWAPKCMHYCRTSDEYTNJLVWNQY";
	std::string output = encryptString(e, input);
	const char* expectedOutput = "FYTIDQIBHDONUPAUVPNKILDHDJGCWFVMJUFNJSFYZTSPITBURMCJEEAMZAZIJMZAVFCTYTKYORHYDDSXHBLQWPJBMSSWIPSWLENZ";
	EXPECT_EQ(expectedOutput, output);
}

TEST(Enigma, PlugBoardTest10)
{
	// 10 plugs
	Enigma e({1, 2, 3}, 'B', {0, 1, 20}, {5, 5, 4}, {"AG", "HR", "YT", "KI", "FL", "WE", "NM", "SD", "OP", "QJ"});
	const char* input = "RNXYAZUYTFNQFMBOLNYNYBUYPMWJUQSBYRHPOIRKQSIKBKEKEAJUNNVGUQDODVFQZHASHMQIHSQXICTSJNAUVZYIHVBBARPJADRH";
	const char* expectedOutput = "CFBJTPYXROYGGVTGBUTEBURBXNUZGGRALBNXIQHVBFWPLZQSCEZWTAWCKKPRSWOGNYXLCOTQAWDRRKBCADTKZGPWSTNYIJGLVIUQ";
	std::string output = encryptString(e, input);
	EXPECT_EQ(expectedOutput, output);
}

TEST(Enigma, PlugBoardTest5O)
{
	// 5 plugs
	Enigma e({2, 5, 3}, 'B', {7, 4, 19}, {12, 2, 20}, {"AF", "TV", "KO", "BL", "RW"});
	const char* input = "IPROPOSETOCONSIDERTHEQUESTIONCANMACHINESTHINKTHISSHOULDBEGINWITHDEFINITIONSOFTHEMEANINGOFTHETERMSMACHINEANDTHINKTHEDEFINITIONSMIGHTBEFRAMEDSOASTOREFLECTSOFARASPOSSIBLETHENORMALUSEOFTHEWORDSBUTTHISATTITUDEISDANGEROUSIFTHEMEANINGOFTHEWORDSMACHINEANDTHINKARETOBEFOUNDBYEXAMININGHOWTHEYARECOMMONLYUSEDITISDIFFICULTTOESCAPETHECONCLUSIONTHATTHEMEANINGANDTHEANSWERTOTHEQUESTIONCANMACHINESTHINKISTOBESOUGHTINASTATISTICALSURVEYSUCHASAGALLUPPOLLBUTTHISISABSURDINSTEADOFATTEMPTINGSUCHADEFINITIONISHALLREPLACETHEQUESTIONBYANOTHERWHICHISCLOSELYRELATEDTOITANDISEXPRESSEDINRELATIVELYUNAMBIGUOUSWORDS";
	const char* expectedOutput = "OZLUDYAKMGMXVFVARPMJIKVWPMBVWMOIDHYPLAYUWGBZFAFAFUQFZQISLEZMYPVBRDDLAGIHIFUJDFADORQOOMIZPYXDCBPWDSSNUSYZTJEWZPWFBWBMIEQXRFASZLOPPZRJKJSPPSTXKPUWYSKNMZZLHJDXJMMMDFODIHUBVCXMNICNYQBNQODFQLOGPZYXRJMTLMRKQAUQJPADHDZPFIKTQBFXAYMVSZPKXIQLOQCVRPKOBZSXIUBAAJBRSNAFDMLLBVSYXISFXQZKQJRIQHOSHVYJXIFUZRMXWJVWHCCYHCXYGRKMKBPWRDBXXRGABQBZRJDVHFPJZUSEBHWAEOGEUQFZEEBDCWNDHIAQDMHKPRVYHQGRDYQIOEOLUBGBSNXWPZCHLDZQBWBEWOCQDBAFGUVHNGCIKXEIZGIZHPJFCTMNNNAUXEVWTWACHOLOLSLTMDRZJZEVKKSSGUUTHVXXODSKTFGRUEIIXVWQYUIPIDBFPGLBYXZTCOQBCAHJYNSGDYLREYBRAKXGKQKWJEKWGAPTHGOMXJDSQKYHMFGOLXBSKVLGNZOAXGVTGXUIVFTGKPJU";
	std::string output = encryptString(e, input);
	EXPECT_EQ(expectedOutput, output);
}

TEST(Enigma, PlugBoardTest10O)
{
	// 10 plugs
	Enigma e({2, 5, 3}, 'B', {7, 4, 19}, {12, 2, 20}, {"AG", "HR", "YT", "KI", "FL", "WE", "NM", "SD", "OP", "QJ"});
	const char* input = "IPROPOSETOCONSIDERTHEQUESTIONCANMACHINESTHINKTHISSHOULDBEGINWITHDEFINITIONSOFTHEMEANINGOFTHETERMSMACHINEANDTHINKTHEDEFINITIONSMIGHTBEFRAMEDSOASTOREFLECTSOFARASPOSSIBLETHENORMALUSEOFTHEWORDSBUTTHISATTITUDEISDANGEROUSIFTHEMEANINGOFTHEWORDSMACHINEANDTHINKARETOBEFOUNDBYEXAMININGHOWTHEYARECOMMONLYUSEDITISDIFFICULTTOESCAPETHECONCLUSIONTHATTHEMEANINGANDTHEANSWERTOTHEQUESTIONCANMACHINESTHINKISTOBESOUGHTINASTATISTICALSURVEYSUCHASAGALLUPPOLLBUTTHISISABSURDINSTEADOFATTEMPTINGSUCHADEFINITIONISHALLREPLACETHEQUESTIONBYANOTHERWHICHISCLOSELYRELATEDTOITANDISEXPRESSEDINRELATIVELYUNAMBIGUOUSWORDS";
	const char* expectedOutput = "KSKAXWRVGKNPSCZTWDYNDTYJXDQTFNZLLCTIGQDVADLCTCNUHGEYZSYRTRHMRYGXBJRUIDGFULWRLRKQUWZKKMYBEVZYLGGDDAEMYOPCXUBDGLBQPRUYICDYNXUPDLPUNMUVCIHBAATQKCECCPNXEDZDWYDUWORMHZODTOIDKAKWSHVFTPDAYJJYQYZHLATGFLRKTUZNXLRDEPHKFONMYKHDWXPGULDJYDRLBNSYOXSJFTNIQXCLROHMJXRWXINJESYAQYSQYSNGMCUBMRPLFTZFATBYAKTLAWMZEQLLHDLAZASJWOPNHOPWWDFUFWYAOJLAERSPUXCRVYGJLXFKOLLIOUZLFKLGFINYELFROZNPRRSACRAMCXKKESQKCUURECJXYHDIHWZWTJGCZKNYQXHBFUGQVAOCQFIKZLSYTBSFRVTGUJBEXMERROWZMOFIFSUTPEWCYPMFUYHTTOEJZBXXCFSKPDGDHMXROXREFONJTEDGGRAPUXCKJKXYNJXTZIIZOREMWYHILARLNUDLRFFGIACXSQRMNCDNJODUERUBNTAIQQUUWPEWXYTLQAVKFGAYUJBW";
	std::string output = encryptString(e, input);
	EXPECT_EQ(expectedOutput, output);
}

TEST(Enigma, PlugBoardTest0O)
{
	// 0 plugs
	Enigma e({2, 5, 3}, 'B', {7, 4, 19}, {12, 2, 20}, {}); // {"AF", "TV", "KO", "BL", "RW"});
	const char* input = "IPROPOSETOCONSIDERTHEQUESTIONCANMACHINESTHINKTHISSHOULDBEGINWITHDEFINITIONSOFTHEMEANINGOFTHETERMSMACHINEANDTHINKTHEDEFINITIONSMIGHTBEFRAMEDSOASTOREFLECTSOFARASPOSSIBLETHENORMALUSEOFTHEWORDSBUTTHISATTITUDEISDANGEROUSIFTHEMEANINGOFTHEWORDSMACHINEANDTHINKARETOBEFOUNDBYEXAMININGHOWTHEYARECOMMONLYUSEDITISDIFFICULTTOESCAPETHECONCLUSIONTHATTHEMEANINGANDTHEANSWERTOTHEQUESTIONCANMACHINESTHINKISTOBESOUGHTINASTATISTICALSURVEYSUCHASAGALLUPPOLLBUTTHISISABSURDINSTEADOFATTEMPTINGSUCHADEFINITIONISHALLREPLACETHEQUESTIONBYANOTHERWHICHISCLOSELYRELATEDTOITANDISEXPRESSEDINRELATIVELYUNAMBIGUOUSWORDS";
	const char* expectedOutput = "KZEQDMFOESMPTATFWQYJIOTRPWLHRMWIDJYPBFYUDGLZFJAFAUQFZAIHBEZMUPVLWDKBFGNHRAUNLWFDKWKKKMIGXAXDZLSRDSCNUSYZMJEPZPRQRRLMIVQXWEFPZBKPPZEVORHXPSVXKHUEDKOQIZZOHWPRPUMMVAKDYMUHTCXSZIQFYQLAJADASYTGPGYNFJMVICNOMFUQJPFWHDZUSIOVYBAXFYJTSZPKSXQBHMZTWPLKLZSXQULUFJLQBCFWQDBPVTSYTISAHQZOQJWIJCSSHTLZXIJUZLMDRJTRHCKYHCXKPWOMMGXLWDLHXWYFLQZZWYDTHUPNZSQSLHRFKKGEUDAZDELGCRGDAOQCDMHOPWVYFQGEDYPIKEKBUFGLSHXRYESHBHZQLMLECKFPGLFLGUGKNGXCOXEIZGZZUPRFTVMNLJCWXOVRVRFCOPBKGSBVMFWBJPVWWYSSGNUVHTXXKYSOJAGWZECIXTRCFECPIERAPRBLYXZVPKBLMFDJCJSGFIBWEYLWFJNGOWOWJFZJGFNQHEJMXJDSQOBHMAGKBXNSAOOGEZAFXGPVJXUIBAVUKFJU";
	std::string output = encryptString(e, input);
	EXPECT_EQ(expectedOutput, output);
}
