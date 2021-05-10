# Java Enigma

This is a Java implementation of an Enigma machine, along with code that attempts to break the encryption. This code is associated with the Computerphile video on [cracking enigma](https://www.youtube.com/watch?v=RzWB5jL5RX0).

An enigma machine is a mechanical encryption device that saw a lot of use before and during WW2. This code simulates a 3 rotor enigma, including the 8 rotors commonly seen during the war. 

## Installing and Usage
You can compile and run this code yourself if you have Java installed. For convenience I recommend using [IntelliJ](https://www.jetbrains.com/idea/) or a similar IDE. The community edition is free, and it'll make editing, compiling and running the code a lot easier. If you'd like to run it yourself, first install java, then follow the instructions below. This assumes you've installed java and git.

### Windows
Clone and traverse into the enigma directory
```
git clone https://github.com/mikepound/enigma.git
cd enigma
```

Compile all the java files from src into bin
```
javac -d bin -sourcepath src src\com\mikepound\Main.java
```

Copy the n-gram statistics into the bin folder too
```
xcopy resources\data bin\data\
```

Run the Enigma code in main
```
java -cp bin com.mikepound.Main
```

### Linux/Unix
Clone and traverse into the enigma directory
```
git clone https://github.com/mikepound/enigma.git
cd enigma
```

Compile all the java files from src into bin
```
javac -d bin -sourcepath src src/com/mikepound/Main.java
```

Copy the n-gram statistics into the bin folder too
```
cp -r resources/data bin/data
```

Run the Enigma code in main
```
java -cp bin com.mikepound.Main
```

## The Enigma Machine
The code for the enigma machine can be found in the `enigma` package. In the `analysis` package is the code to perform attacks on ciphertext. The attack uses various fitness functions that attempt to measure the effectiveness of a test decryption, found within the `analysis.fitness` package. Finally, the `Main.java` file is where you'll find the actual attack I performed in the video, and so it also contains a lot of examples you can use to run your own attacks.

### Creating a Java Enigma
The code itself is fairly straightforward. You can create a new enigma machine using a constructor, for example this code will create a new object called `enigmaMachine` with the settings provided:

```java
enigmaMachine = new Enigma(new String[] {"VII", "V", "IV"}, "B", new int[] {10,5,12}, new int[] {1,2,3}, "AD FT WH JO PN");
```

Rotors and the reflector are given by their common names used in the war, with rotors labelled as `"I"` through to `"VIII"`, and reflectors `"B"` and `"C"`. I've not implemented every variant, such as the thin reflectors seen in naval 4-rotor enigma. You could easily add these if you liked. Starting positions and ring settings are given as integers 0-25 rather than the A-Z often seen, this is just to avoid unnecessary mappings. The majority of the code here treats letters as 0-25 to aid indexing. Plugs are given as a string of character pairs representing steckered partners. If you don't wish to use a plugboard, `""` or `null` is fine.

### Encrypting and Decrypting
Given an enigma instance like the `enigmaMachine` above, encryption or decryption is performed on character arrays of capital letters [A-Z]. Simply to save time I\'ve not done a lot of defensive coding to remove invalid characters, so be careful to only use uppercase, or to strip unwanted characters out of strings. Here is an encryption example using the enigma machine above:

```java
char[] plaintext = "ABCDEFGHIJKLMNOPQRSTUVWXYZ".toCharArray();
char[] ciphertext = enigmaMachine.encrypt(plaintext);
String s = new String(ciphertext); // UJFZBOKXBAQSGCLDNUTSNTASEF
```
You can quickly check everything is working by running the tests found in the `EnigmaTest.java` file.

### How it works
Throughout the enigma machine, letters A-Z are represented as integers 0-25. Most of the components, the rotors, reflector and plugboard are treated as arrays that map values 0-25 to a different set of values 0-25. Encrypting or decrypting is simply a case of passing a value through these arrays in turn. What makes enigma trickier is that the arrays rotate, and that they can have different starting or ring positions. For efficiency in this implementation I keep the arrays fixed, and simulate rotation by shifting the index in and out of each rotor. Before each character is encrypted the rotors rotate, sometimes causing the neighbouring rotors to also rotate, this is handled by the `rotate()` function. Enigma has a quirk whereby the middle rotors moves twice when it turns the left-most rotor. This is called double stepping, and is also implemented here.

## Breaking a Code
Breaking an enigma message here comes down to decrypting a ciphertext with all possible rotor configurations and seeing which output looks the best. We measure best here using a fitness function.

### Fitness functions
The code makes a number of fitness functions available that can be used to measure how close a test decryption is to English text. Each works similarly, some work better than others. You can test to see which work best for a given message. The fitness functions are:
* **Index of coincidence**. The probability of any random two letters being identical. Tends to be higher for proper sentences than for random encrypted text
* **Single / Bi / Tri / Quad grams**. The probability of a sentence measured based on the probability of constituent sequences of characters. Bigrams are pairs, such as AA or ST. Trigrams are triplets, e.g. THE, and so on.
* **Plaintext Fitness**. This function is a known plaintext attack, comparing the decryption against all or portions of a suspected real plaintext. This is by far the most effective solution, even a few words of known plaintext will substantially increase your odds of a break even with a number of plugboard swaps.

### Ciphertext Analysis
The basic approach to the attack is as follows:
1. Decrypt the ciphertext with every possible rotor in each position, and rotated to each starting position. All rotor ring settings are set to 0. No plugboard. For each decryption, measure the text fitness using one of the available fitness functions. Save the best performing rotor configuration.
2. Fix the rotors, and iterate through all possible ring settings for the middle and right rotors, again testing the fitness of the decryption. You do not have to use the same fitness function as before.
3. Fix all settings, and then use a hill climbing approach to find the best performing plugboard swaps, again measured using a fitness function.

## Notes
* The code is fairly efficient, Enigma boils down to a lot of array indexing into different rotors. This said, I didn't worry too much about speed, it's plenty fast enough. I used classes and functions rather than doing things inline, for example. Modern compilers will optimise a lot of it anyway.
* I've added a more optimised and multi-threaded version in a branch called [optimised](https://github.com/mikepound/enigma/tree/optimised). I've managed to get the code to break a message in under 4 seconds on one of our servers! I'm keeping this code separate to the main branch to keep the main branch clean and based off the original video.
* Similarly, in the brute force key search code, for simplicity I create new enigma machines as required rather than implementing a number of specific reinitialisation functions that would be faster.
* I've not written any kind of command line parsing here. You're welcome to add this, but i felt for a tutorial on enigma and breaking it, a step by step procedure in main was fine.

## Resources
1. For more details on enigma, the [wikipedia articles](https://en.wikipedia.org/wiki/Enigma_machine) are a great resource.

2. This attack is a variant of that originally proposed by James Gillogly. His work on this is still available via the web archive [here](https://web.archive.org/web/20060720040135/http://members.fortunecity.com/jpeschel/gillog1.htm).

3. If you'd like a more visual example of both encryption and cracking enigma codes, the Cryptool project is a great tool for this. [Cryptool 2](https://www.cryptool.org/en/) has a great visualiser and can run cracking code similar to my own. I used cryptool to write the tests I used to make sure my own enigma implementation was working.


## Motivation 

I was suprised by the shown durations and suspected that with a optimized implmentation it should be possible to crack enigma much faster. SO I decided to give it a try. 
As my performance measurement experience is limited to C++, I had to port the implmentation to C++ first. 

## Port to C++ 

This was relatively straightforward. 
I only had to add a few more test of the components of the enigma machine, as some subtle errors in Rotor logic crept in. 
I tried to make as much as possible of the enigma machine constexpr, to enable compile time checking. This resulted in replacing alot of the strings with numbers, as compile time string handling is somewhat unpleasant in current C++. 

Then I ported the analysis part (at least most of it) to C++. This was straight forward, except the file handling of the data files was a bit annoying, so I made them header files and stored the data statically. 

## First runs. 

After a bit of debugging, my attack using IoC and Bigrams ran. 

The intital port started with an execution time of 22846.3ms compred to 31080 for the java version, pretty comparable.

I quickly was confused that the machine would find the right rotor setting, but would fail to find the rights ring settings. 
I gave the algorithm the correct rotors and plugboard settings, and a plaintext score function and even given those the system would not find the correct ring settings. 
I suppose there might be a bug regarding the search of these. I compared to the java code, and my result are identical to the java code. So i assumed the problem did not arise in the port.

TODO: why are my quadgrams off, where bigrams and wkt work?

[perf-kernel.svg]
Fir performance analysis with perf yielded predictable the result that all the time was spend in the rotor function, so a more in depth look was genereated using valgrind tool callgrind. 

Callgrind shows that a lot of work was done in the Rotor::Create function where the wiring were converted from strings and the inverseWiring had to be created. Doing that every time seemed not necessary.

By adding a static RotorMapping which is always consulted the runtime of the program dropped to 20936.6ms. A signifcant improvement, but not so big.

Further analysis shown that 75% of the total runtime is spent in the encipher() function in the rotor and 10% (half-half) in isAtNotch() and turnover().

Deetailed analysis of the encipher() function resulted in the module operations being a major problem, as module of something which is not a power of 2 will have significant cost. 

The first modulo operation is when the cuurrent rotor shift and the input k gets added. to eliminate this modulo whe precompute adn store the currentRotorShift  as (rotorPosition  + 26 - ringSetting ) %26 and in every turnover we just update the shift in parallel. This pushed extra work into turnover() now updating two states, but now the shift does not need to be calcualted in every cipher operation.
The lookup in the current wiring array happens at position k + shift now. 
We know now that bot the shift and k can only be up to 26.
Instead of computing modulo 26, we can just double the wiring array size, repeating the charcters from pos 26 to 52. 
Doing that allow to just omit the modulo operation as there will always be a valid lookup position. 
This reduces the runtime to 12979.5ms.

The next %26 occurs for the return value. Here the range of the value can be 0 to 52 and needs to be truncated to 26. 
Here we can just omit the module again, if we then triple our array to allow values up to 78. No we know that we can just mit integer up to 52 safely. 
We basically use the mapping to control our value sizes. All conversions to printable ascii characters to now need a modulo 26 to be useful, luckily thats not hard.
This recudes the runtime to 6701.39ms

After this significant changes, a new analysis of the performance is conducted.
The the conversion from and to ascii chars is now a significant bottleneck, as it now involves the modulo 26 operation. 
In order to minimize this dependency the score function is changed to operate 
on the converted integers in a fixed size array, so the string representation is not need there.
This recudes the runtime to 5290.51ms
