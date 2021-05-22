# Enigma

This is a port inspired by the original Java Enigma at: https://github.com/mikepound/enigma

Look there for the original Implementation and explanations as to what this code does.

## Motivation 

I was suprised by the shown durations and suspected that with a optimized implementation it should be possible to crack enigma much faster. So I decided to give it a try. 
As my performance measurement experience is limited to C++, I had to port the implmentation to C++ first. It should be emphasized that I didnt do this because C++ is inherently faster (which it isnt necessarily), but just that the profiling tools I know and use stem from the C++ world, and I didn't feel like learning a new tool ecosystem.

## Port to C++ 

This was relatively straightforward. 
I only had to add a few more test of the components of the enigma machine, as some subtle errors in Rotor logic crept in. 
I tried to make as much as possible of the enigma machine constexpr, to enable compile time checking. This resulted in replacing alot of the strings with numbers, as compile time string handling is somewhat unpleasant in current C++. 

Then I ported the analysis part (at least most of it) to C++. This was straight forward, except the file handling of the data files was a bit annoying, so I made them header files and stored the data statically. 

## Building and Running the program

To run the program an the test suite, cmake and a current compiler is needed. For the parallelism, I used IntelTBB which also needs to be installed. 

To build the implementaion the following steps are needed
```
mkdir build
cd build 
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8 all
```

This created two executable in the build dir, enigma_test and enigma_main. 
These can then just be executed in a terminal.

## First runs. 

After a bit of debugging, my attack using IoC and Bigrams ran. Quadgrams have some errors in the implementation which I never rectified, because bigrams work just fine.

The initial port started with an execution time of 22846.3ms compared to 31080 for the java version, pretty comparable.

I quickly was confused that the machine would find the right rotor setting, but would fail to find the right ring settings. 
I gave the algorithm the correct rotors and plugboard settings, and a plaintext score function and even given those the system would not find the correct ring settings. 
I suppose there might be a shortcoming regarding the search of these. I compared to the java code, and my results are identical to the java code. So i assumed the problem did not arise in the port.

Fir performance analysis with perf yielded predictable the result that all the time was spend in the rotor function, so a more in depth look was genereated using valgrind tool callgrind (evaluted with kcachegrind). 

Callgrind shows that a lot of work was done in the Rotor::Create function where the wiring were converted from strings and the inverseWiring had to be created. Doing that every time a new Enigma is created seemed not necessary.

By adding a static RotorMapping which is always consulted the runtime of the program dropped to 20936.6ms. A significant improvement, but not so huge.

## Simplifying and modifing the Rotors

Further analysis shown that 75% of the total runtime is spent in the encipher() function in the rotor and ca. 9% (half-half) in isAtNotch() and turnover(). 99% of the total runtime is spent in the Rotor class methods. So the focus must be making the rotors more efficent to compute, or making less rotors turns. I ended up doing both.

Detailed analysis of the encipher() function resulted in the modulo 26 operations being a major problem, as module of something which is not a power of 2 will have significant cost. 

The first modulo operation is when the current rotor shift and the input k gets added. To eliminate this modulo whe precompute and store the currentRotorShift  as (rotorPosition  + 26 - ringSetting ) %26 and in every turnover we just update the shift in parallel. The lookup in the current wiring array happens at position k + shift now. 
We know now that bot the shift and k can only be up to 26.
Instead of computing modulo 26, we can just double the wiring array size, repeating the same charcters from pos 26 to 52. Doing that allows to just omit the modulo operation as there will always be a valid lookup position. 
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

After all these specific operations, rumtine started to spread out between different operations in the program. Also a lot of the lookups in the arrays for the actual character mapping now took 37%. Currently im using 64 bit integers, but the real value range of all these characters is from -1 to 78, so they will fit in a 8bit-integer. This will make them squeeze into caches better.
This had a surprisingly small effect of only reducing the runtime to 3908.49ms.

Realistically the left wheel of the enigma does not rotate that much. So as long the left has not rotated, it is a static mapping. So the forward() translation of the left wheel through the plugboard and the backward translation can be represented by another static plugboard. This needs to be updated whenever the left wheel turns, but this event is rare enough for it to be worth it. 
This reduced the runtime down to 2919.83ms.

The next realization was that is was using the same encoding arrays for plugboards and reflector as for the roters, but the value range for these was only 0-52 with my current setup. 
So I splitted them out by using only 52 charcters big landing pads for these. 
This also led me to realize that in the rotor, instead of adding 26 to the mapped value before subtracting the shift I just could add 26 to the mapped value. 
This further brought runtime down to 2452.77ms.

At this point further optimizations become much less obvious. 
So I looked at the multithreading optimizations. Including the reset logic form 
the optimized branch did not actually improve performance at all. 
Precomputing the rotor combinations does not change performance too, but is the 
foundation for using C++ parallel STL algorithms. 

I installed Intel TBB to use with gcc9.3 to use the C++17 parallel STL.
A simple for each with parallel exceution policy on my 12 core CPU (6 cores + HT) brings the execution time down 430ms. As this seems to be a heavily computations bound operation, the hyperthreading does not help at all.
The problem arises here that starting the program, making the outputs and setting up the threadpool takes so long it makes accurate measurements of small improvements more and more difficult.

## Performance optimizations summary

The original runtime of the java version was 31080ms single threaded and around 5180ms mulltithreaded for me. 
The final version needed 2452ms, a 12.7x speedup for singlethreaded operation and 430ms, a 12x improvment for the multithreaded version. 

Analysing the performance metrics and assembly it is clear that the compiler successfully optimized away classes, functions and all of that. The common thought model around computing "lest just think about Big(O) and everything else the compiler will figure out is not true though. Tweaking matters. Getting 10x performance improvements just from tweaking the implmentation is pretty common. It is important to note, that is done well (which was already mostly the case in the java version), classes, functions and paramters are absolutely zero cost. Things that are actually expensive are allocating heap memory, deleting it, missing caches and any division or modulo operations. 

An example for that can be found in the turnover function. 
The version 
```
if(++currentRotorShift >= 26)
  currentRotorShift = 0;
```
is drastically faster than 
```
currentRotorShift = (currentRotorShift+1)%26;
```
The transformation from the below to above version is only valid and correct if we can be certain, that the currentRotorShift is smaller than 26 prior to this function. 
As this iis the only place modifing this variable, this can be certain. Because modern compilers first transform to IR and strip away all classes, this invariant (currentRotorShift can not be modified anywhere else) is lost before the optimizer comes in. So the optimizer can not know what the value range of currentRotorShift is prior to this call and thus is not allowed to make this transformation.

## Finding the correct the starting positions and ring settings.

As there are currently no more ideas to improve runtime, and with 0.4 seconds further optimization becomes hard, I focussed on fixing the ring positions issue. 
It bothered me that the ring setting did not find the correct solution.
It currently runs into a local maximum quite quickly and does not at all attempt to search for the right rotors. Given that the runtime is currently only a few milliseconds I upgraded it to be more of brute force search, hoping to overcome that local maximum in the fitness function. 
By just choosing the starting positions as {0,0,0} and changing the ring settings one can get very close. The problem is that for this worng starting position, a wrong set of ring settings actually scores better than the correct solution.
When actually trying each right rotor combination ({0,0,n}), the local maximums are no longer an issue to the same degree. And using the parallelism and operations from above, it still only needs around 400ms to do that. (As this is the same as checking 26 different rotor combinations.)

## Thoughts on more Plugs.

The last frontier for me is the plugboard settings. While the performance in handling more plugboard settings is not really an issue, they still destroy the fitness function. Interestingly, the mapping of the plugboard setting on the output at the end does not really affect IoC as as it changes only which character is frequent and this does not matter for IoC. The problem is the initial application on the way into enigma. Due to the fact the rotors turn, this dilutes the signal and makes the IoC more noisy.

As the plugboard settings still retain a significant IoC up to roundabout 5 rotors at the chosen message length, upwards at 6-7 plugs it quickly becomes anyones guess. 
When we want to crack enigma with 10 plugs, just IoC will not cut it.

When 10 unknown plugboard settings are choosen the probability that my random choice matches them is:
```
    20/26. * 1/25. 
  * 18/24. * 1/23. 
  * 16/22. * 1/21. 
  * 14/20. * 1/19. 
  * 12/18. * 1/17. 
  * 10/16. * 1/15. 
  * 8/14.  * 1/13. 
  * 6/12.  * 1/11. 
  * 4/10.  * 1/9. 
  * 2/8.   * 1/7. 
```
which is equal to 1/150738274937250. Given that each check of a rotor to starting position combination is 0.4 seconds, this will never finish.

What helps is that it is actually well behaved in the way that each correct plugboard seeing increases the IoC more than an incorrect plugbord reduces it, after the initial threshold was passed. In experimenting with this, when selecting 10 plugboard connections and then choosing some if them correct, I found that it is only necessary to have 5-6 plugboard setting correctly to actually see the IoC metric become useful, even when the other plugs are wrong.

This changes the situation when only 5 need to be correct: 
```
    20/26. * 1/25. 
  * 18/24. * 1/23. 
  * 16/22. * 1/21. 
  * 14/20. * 1/19. 
  * 12/18. * 1/17. 
```
which is equal to 1/15096510. So "only" 15 million combinations need to be checked on average until a suitable settings is found. 5 is right on the egde where the IoC is still usable, 6 correct would be better, but is much less likely.
I wrote a efficient generator for random plugboard settings and it can generate plugboard settings without adding significant runtime. Still this would take on average 6038604 seconds to complete, which is 1670 hours. 
This is beyond the limits of my patience, the actual crib-based search the codecracker in Bletchley park used is much more efficient here.
