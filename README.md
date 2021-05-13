# Enigma

This is a port inspired by the original Java Enigma at: https://github.com/mikepound/enigma

Loop there for the original Implemntation and explanations.

## Motivation 

I was suprised by the shown durations and suspected that with a optimized implementation it should be possible to crack enigma much faster. So I decided to give it a try. 
As my performance measurement experience is limited to C++, I had to port the implmentation to C++ first. 

## Port to C++ 

This was relatively straightforward. 
I only had to add a few more test of the components of the enigma machine, as some subtle errors in Rotor logic crept in. 
I tried to make as much as possible of the enigma machine constexpr, to enable compile time checking. This resulted in replacing alot of the strings with numbers, as compile time string handling is somewhat unpleasant in current C++. 

Then I ported the analysis part (at least most of it) to C++. This was straight forward, except the file handling of the data files was a bit annoying, so I made them header files and stored the data statically. 

## First runs. 

After a bit of debugging, my attack using IoC and Bigrams ran. 

The initial port started with an execution time of 22846.3ms compared to 31080 for the java version, pretty comparable.

I quickly was confused that the machine would find the right rotor setting, but would fail to find the right ring settings. 
I gave the algorithm the correct rotors and plugboard settings, and a plaintext score function and even given those the system would not find the correct ring settings. 
I suppose there might be a bug regarding the search of these. I compared to the java code, and my result are identical to the java code. So i assumed the problem did not arise in the port.

TODO: why are my quadgrams off, where bigrams and wkt work?

[perf-kernel.svg]

Fir performance analysis with perf yielded predictable the result that all the time was spend in the rotor function, so a more in depth look was genereated using valgrind tool callgrind. 

Callgrind shows that a lot of work was done in the Rotor::Create function where the wiring were converted from strings and the inverseWiring had to be created. Doing that every time seemed not necessary.

By adding a static RotorMapping which is always consulted the runtime of the program dropped to 20936.6ms. A significant improvement, but not so huge.

Further analysis shown that 75% of the total runtime is spent in the encipher() function in the rotor and 10% (half-half) in isAtNotch() and turnover().

Detailed analysis of the encipher() function resulted in the modulo 26 operations being a major problem, as module of something which is not a power of 2 will have significant cost. 

The first modulo operation is when the current rotor shift and the input k gets added. To eliminate this modulo whe precompute adn store the currentRotorShift  as (rotorPosition  + 26 - ringSetting ) %26 and in every turnover we just update the shift in parallel. This pushed extra work into turnover() now updating two states, but now the shift does not need to be calcualted in every cipher operation.
The lookup in the current wiring array happens at position k + shift now. 
We know now that bot the shift and k can only be up to 26.
Instead of computing modulo 26, we can just double the wiring array size, repeating the charcters from pos 26 to 52. 
Doing that allow to just omit the modulo operation as there will always be a valid lookup position. 
This reduces the runtime to 12979.5ms.

The next %26 occurs for the return value. Here the range of the value can be 0 to 52 and needs to be truncated to 26. 
Here we can just omit the modulo again, if we then triple our array to allow values up to 78. No we know that we can just return integer up to 52 safely. 
We basically use the mapping to control our value sizes. All conversions to printable ascii characters to now need a modulo 26 to be useful, luckily thats not hard (and quite rare).
This recudes the runtime to 6701.39ms

After this significant changes, a new analysis of the performance is conducted.
The the conversion from and to ascii chars is now a significant bottleneck, as it now involves the modulo 26 operation. 
In order to minimize this dependency the score function is changed to operate 
on the converted integers in a fixed size array, so the string representation is not need there.
This recudes the runtime to 5561.27ms

By applying the same trick to the IoC fitness function just having a triple sized array and then run over all the three section eliminates another modulo and drop the runtime to 5096.15ms.

By adjusting the notch check to actually be based on the rotor shift (instead of position), we simplify the check and end up with another reduction to 4046.31ms.

After all these specific operations, rumtine started to spread out between different operations in the program. Also a lot of the lookups in the arrays for the actual character mapping now took 37%. Currently im using 64 bit integers, but hte real value range of all these characters is from -1 to 78, so they will fit in a 8bit-integer. This will make them squeeze into caches better.
This had a surprisingly small effect of only reducing the runtime to 3823.17ms.

At this point further optimizations become much less obvious. 
