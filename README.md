# Enigma

This is a port inspired by the original Java Enigma at: https://github.com/mikepound/enigma

Look there for the original Implementation and explanations as to what this code does.

## Motivation 

I was surprised by the shown computing times and suspected that with an optimized implementation it should be possible to crack enigma much faster. So with some spare time on hand, I decided to give it a try. 
As my performance measurement experience is limited to C++, I had to port the implementation to C++ first. It should be emphasized that I didn't do this because C++ is inherently faster (which it isn't necessarily), but just that the profiling tools I know and use stem from the C++ world, and I didn't feel like learning a new tool ecosystem.

## Port to C++ 

This was relatively straightforward. 
I only had to add a few more tests of the components of the enigma machine, as some subtle errors in Rotor logic crept in. 
I tried to make as much as possible of the enigma machine `constexpr`, to enable compile-time checking. This resulted in replacing a lot of the strings with numbers, as compile-time string handling is somewhat unpleasant in current C++. 

Then I ported the analysis part (at least most of it) to C++. This was straightforward, except the file handling of the data files was a bit annoying, so I made them header files and stored the data statically. 

## Building and Running the program

To run the program and the test suite, cmake and a current compiler is needed. For the parallelism, I used IntelTBB which also needs to be installed. 

To build the implementation the following steps are needed
```
mkdir build
cd build 
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8 all
```

This created two executable in the build dir, enigma_test and enigma_main. 
These can then just be executed in a terminal.

## First runs. 

After a bit of debugging, my attack using IoC and Bigrams ran. Quadgrams have some errors in the port which I never rectified because bigrams work just fine.

The initial port started with an execution time of 22846.3ms compared to 31080 for the java version, pretty comparable.

I quickly was confused that the machine would find the right rotor setting, but would fail to find the right ring settings. 
I gave the algorithm the correct rotors and plugboard settings, and a plaintext score function and even given those the system would not find the correct ring settings. I compared it to the java code, and my results are identical to the java code. So I did not introduce a bug, it is a shortcoming of the original implementation.

Fir performance analysis with perf yielded predictable the result that all the time was spent in the Rotor class, so I generated a more in-depth look using valgrinds tool callgrind (evaluated with kcachegrind). 

callgrind shows that a lot of work was done in the `Rotor::Create` function where the wiring is converted from strings and the inverseWiring had to be created. Doing that every time a new Enigma is created seemed unnecessary.

By adding a static RotorMapping at compile time which is always used during runtime of the program dropped to 20936.6ms. A significant improvement, but not huge.

## Simplifying and modifying the Rotors

Further analysis shown that 75% of the total runtime is spent in the `encipher()` function in the rotor and ca. 9% (half-half) in `isAtNotch()` and `turnover()`. 99% of the total runtime is spent in the Rotor class methods. So the focus must be making the rotors more efficient to compute or making fewer rotors turns. I ended up doing both.

Detailed analysis of the `encipher()` function resulted in the `%26` operations being a major problem, as modulo of anything which is not a power of 2 will have significant cost (it is computing using a series of divisions).

The first modulo operation is when the current rotor shift and the input k gets added. To eliminate this modulo one can precompute and store the `currentRotorShift`  computed by `(rotorPosition  + 26 - ringSetting ) %26` instead of the `rotorPosition` and in every turnover, we just update the shift instead of the rotor positions. The lookup in the current wiring array happens at position `k + shift` now. In this first version, I also kept the `rotorPosition` variable around, updated it together with the shift and used it for notch checks.
We know now that both the shift and k has a value range from 0 to 26.
Instead of computing modulo 26, we can just double the wiring array size, repeating the same characters from pos 26 to 52. Doing that allows to just omit the modulo operation as the sum of both is less than 52.
This reduces the runtime to 12979.5ms.

The next `%26` operation occurs for the return value. The range of the sum of the inverted shift and the mapped value will be 0 to 52 and needs to be truncated to 26. 
Here we can just omit the modulo again if we then triple our array to allow values up to 78. Now we know that we can just return integer up to 52 safely. This means we now handle integer from 0 to 52 in the whole program, where both 0 and 26 mean A. This means all lookup tables need to be doubled, also for Reflector and Plugboard. As each mapping of the value in plugboard or reflector will return a range [0,26), and each rotor mapping yield [0,52), we never need to handle values above 52. 
As each rotor lookup maps the value back into the [0,52) range, our values stay bounded within that range. 
All conversions to printable ASCII characters now need a `%26` to be useful, luckily these are very rare.
This reduces the runtime to 6701.39ms

After these significant changes, a new analysis of the performance was conducted.
The conversion from and to ASCII chars is now a significant bottleneck, as it now involves the modulo 26 operation and a lot of string handling.
To avoid string handling the score function is changed to operate on the converted integers in a fixed-size array of integer first. (instead of a heap-allocated string). This reduces the runtime to 5561.27ms

By applying the same trick to the IoC fitness function just having a double-sized array and then summing over all the three sections eliminates the `%26` also and drops the runtime to 5096.15ms.

By adjusting the notch check to be based on the `currentRotorShift` shift (instead of `rotorPosition`), we eliminate the necessity to store and update the `rotorPosition` and end up with another reduction to 4046.31ms.

After all these specific operations, runtime started to spread out between different operations in the program. Also, a lot of the lookups in the arrays for the actual character mapping now took 37%. Currently, I'm using 64-bit integers, but the real value range of all these characters is from -1 to 78, so they will fit in an 8-bit integer. I assumed this would improve cache locality, which it does. But the whole program is not cache or memory-bound but computation-bound. 
As a result, this had a surprisingly small effect of only reducing the runtime to 3908.49ms. This is the first time in my career that a program I encountered so little effect of caching and value sizes too performance, but the program structure of a codebreaking program is also very unusual as it executes exceptionally little lines of code over and over, focussing on a trivial computation repeated millions of times.

Reading about the original bombe, it had only two rotors. This abuses the fact that the left wheel of the enigma does not rotate for long periods. So as long the left rotor has not yet rotated, it is a static mapping. So the `forward()` translation of the left wheel, the reflector and the `backward()` translation can be represented by a modified reflector. This needs to be updated whenever the left wheel turns, but this event is rare enough for it to be worth it. 
This reduced the runtime down to 2919.83ms.

The next realization was that I was using the same array typedef for plugboards and reflector as for the rotors, but the value range for these was only 0-52 with my current setup. 
So I split them out by using only 52 characters big landing pads for these. 
This also led me to realize that in the `encipher()` function, instead of adding 26 to the mapped value before subtracting the shift I just could add 26 to the mapped value, making this computation static.
This further brought runtime down to 2452.77ms.

At this point, further optimizations become much less obvious. 
So I looked at the multithreading optimizations. Including the reset logic from the optimised branch did not improve performance at all. 
Precomputing the rotor combinations does not change performance too but is the foundation for using C++ parallel STL algorithms. 

I installed Intel TBB to use with gcc9.3 to use the C++17 parallel STL.
A simple for each with parallel execution policy on my 12 core CPU (6 cores + HT) brings the execution time down 430ms. As this seems to be a heavily computation-bound algorithm, the hyperthreading does not help at all.
The problem arises that starting the program, making the outputs and setting up the thread pool takes so long it makes accurate measurements of small run-time improvements more and more difficult.

## Performance optimizations summary

The original runtime of the java version was 31080ms single-threaded and around 5180ms multi-threaded on my machine. 
The final version needed 2452ms, a 12.7x speedup, for single-threaded operation and 430ms, a 12x improvement, for the multithreaded version. 

Analysing the performance metrics and assembly it is clear that the compiler successfully optimized away classes, functions and all of that. The common thought model around computing "let's just think about Big(O) and everything else the compiler will figure out" is not true though. Tweaking matters. Getting 10x performance improvements just from tweaking the implementation is pretty common. It is important to note, that if done well (which was already mostly the case in the java version), classes, functions and parameters are zero cost. Really expensive things are allocating heap memory, deleting it, missing caches and any division or modulo operations. 

An example of this can be found in the turnover function. 
The version 
```
if(++currentRotorShift >= 26)
  currentRotorShift = 0;
```
is drastically faster than 
```
currentRotorShift = (currentRotorShift+1)%26;
```
The transformation from the below to the above version is only valid and correct if we can be certain that the currentRotorShift is smaller than 26 before this function. 
As this is the only place modifying this variable, this is the case. Because modern compilers first transform to IR and strip away all classes, this invariant (currentRotorShift can not be modified anywhere else) is lost before the optimizer comes in. So the optimizer can not know what the value range of currentRotorShift is before this call and thus is not allowed to make this transformation.

## Finding the correct starting positions and ring settings.

As I ran out of ideas on how to improve runtime, and with 0.4 seconds further optimization becomes hard, I focussed on fixing the ring positions results. It bothered me that the ring setting did not find the correct solution.
It currently runs into a local maximum quite quickly and does not at all attempt to search for the right rotor offsets. Given that the runtime is currently only a few milliseconds I upgraded it to be more of a brute force search, hoping to overcome that local maximum in the fitness function. 
By just choosing the starting positions as {0,0,0} and changing the ring settings does not help. The problem is that for this wrong starting position, a wrong set of ring settings scores better than the correct solution.
When trying each right rotor combination ({0,0,n}) for each ring setting, the local maximums are no longer an issue to the same degree. And using the parallelism and operations from above, it still only needs around 400ms to do that. (As this is the same as checking 26 different rotor combinations.)

## Thoughts on more plugs.

The last frontier for me is the plugboard settings. While the performance in handling more plugboard settings is not an issue for the last step of finding them, they still destroy the quality of the fitness function. Interestingly, the mapping of the plugboard setting on the output at the end does not affect IoC as it changes only which character is frequent and this does not matter for IoC. The problem is the initial application on the way into the machine. Due to the fact the rotors turn, this dilutes the signal and makes the IoC noisier with each plug.

As the plugboard settings still retain a significant IoC up to roundabout 5 plugs at the chosen message length, upwards at 6-7 plugs it quickly becomes just noise. When we want to crack enigma with 10 plugs, just IoC will not cut it.

When 10 unknown plugboard settings are chosen the probability that my random choice matches them is:
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
which is equal to 1/150738274937250. Given that each check of a rotor to starting position combination is 0.4 seconds, this will not work.

What helps is that it is well behaved in the way that each correct plug increases the IoC more than an incorrect plug reduces it, after the initial threshold was passed. In experimenting with this, when selecting 10 plugboard connections and then choosing some of them correct, I found that it is only necessary to have 5-6 plugboard setting correctly to see the IoC metric become useful, even when the other plugs are wrong.

This changes the situation to when only 5 out of 10 need to be correct: 
```
    20/26. * 1/25. 
  * 18/24. * 1/23. 
  * 16/22. * 1/21. 
  * 14/20. * 1/19. 
  * 12/18. * 1/17. 
```
which is equal to 1/15096510. So "only" 15 million combinations need to be checked on average until a suitable setting is found. 5 is right on the edge where the IoC is still usable, 6 correct would be better, but is much less likely.
I wrote an efficient generator for random plugboard settings and it can generate plugboard settings without adding significant runtime. Still, this would take on average 6038604 seconds to complete, which is 1670 hours. This is beyond the limits of my patience, the actual crib-based search the code-crackers in Bletchley park used is much more efficient here.
