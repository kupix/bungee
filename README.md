
![actions workflow](https://github.com/kupix/bungee/actions/workflows/push.yml/badge.svg)

# 🎵 Bungee 

Bungee is a library for processing audio. It can:
* Adjust the playback speed of audio without affecting pitch
* Adjust pitch, or transpose, audio without affecting playback speed
* Or any combination of pitch and speed manipulation, adjusted in real time!

Bungee is unique in its controllability, allowing continually changing pitch and position with seamless support of zero and negative playback speeds. So it can be used for a "smooth scrub" or for rendering life-like audio for slow-motion videos.

## Features

* Simple, fast, with good quality audio output (hear some [comparisons](https://bungee-audio.pages.dev/compare.html) with other approaches)
* Resonably low latency (of the order of 20ms for speed and pitch controls and 40ms from audio input to output)
* Frequency-domain phase-vocoder-based algorithm
* Modern C++ for clean and resilient code
* Static library with a command-line utility that operates on WAV files
* Permissively licensed under MPL-2

## Getting started

Bungee's dependencies are managed as git submodules; so clone like this:
```
git clone --recurse-submodules https://github.com/kupix/bungee
```

Use CMake to configure and build the bungee library and command-line executable:
```
cd bungee
mkdir build && cd build
cmake ..
cmake --build .
```

After a succesful build, run the bungee executable
```
./bungee --help
```

## Using Bungee from your own code

Bungee operates on discrete, overlapping "grains" of audio, typically processing around 100 grains per second. Parameters such as position and pitch are provided on a per-grain basis so that they can be changed continuously as audio rendering progresses. This means that only minimal parameters are required for  instantiation.

### Instantiation

To instantiate, include the [Bungee.h](./bungee/Bungee.h) header file, create a `Stretcher` object and initialise a `Request` object:

``` C++
#include "Bungee.h"

Bungee::Stretcher stretcher({44100, 44100}, 2);

Bungee::Request request;
Bungee::Stretcher::defaultRequest(request);

// set intial speed, this example shows how to achieve constant 75% output speed
request.speed = 0.75;
```

### Granular loop

`Stretcher`'s processing functions are typically called from within a loop, each iteration of which corresponds to a grain of audio. For each grain, the functions `Stretcher::specifiyGrain`, `Stretcher::analyseGain` and `Stretcher::synthesiseGrain` should be called in sequence.
```C++
while (true)
{
    // ...
    // Set up request's members, for example, pitch, as required.
    // ...
 
    auto inputChunk = stretcher.specifyGrain(request);

    // ...
    // Examine inputChunk and retrieve the segment of input audio that the stretcher requires.
    // Set data and channelStride here.
    // ...

    stretcher.analyseGrain(data, channelStride);

    Bungee::OutputChunk outputChunk;
    stretcher.synthesiseGrain(outputChunk);

    // ...
    // Output the audio buffer indicated by outputChunk.
    // ...

    // Update request for next time. Position could be set anywhere within the input audio.
    request.position += inputChunk.unitHop * request.speed;
}
```

### Things to note

* Development has focussed on stereo and mono operation at sample rates of 44.1kHz and 48kHz. In principle, though, any practical number of audio channels and sample rate are supported.

* `Request::position` is a timestamp, it defines the grain centre point in terms of an input audio frame offset. It is the primary control for speed adjustments and is also the driver for seek and scrub operations. The caller is responsible for deciding  `Request::position` for each grain. 

* The caller owns the input audio buffer and must provide the audio segment indicated by `InputChunk`. Successive grains' input audio chunks may overlap. The `Stretcher` instance reads in the input chunk data when `Stretcher::analyseGrain` is called.

* The `Stretcher` instance owns the output audio buffer. It is valid from when `Stretcher::synthesiseGrain` returns up until `Stretcher::synthesiseGrain` is called for the subsequent grain. Output audio chunks do not overlap: they should be concatenated to produce an output audio stream.

* Output audio is timestamped. The original `Request` objects corresponding to the start and end of the chunk are provided by `OutputChunk`.

* Bungee works with 32-bit floating point audio samples and expects sample values in the range -1 to +1 on both input and output. The algorithm performs no clipping.

* When configured for 1x speed and no pitch adjustment, the difference between input and output signals should be very small: rounding errors only.

* Any special or non-numeric float values such as NaN and inf within the input audio may disrupt or cause loss of output audio.

## Dependencies

The Bungee library gratefully depends on:
* Eigen C++ library for buffer management and mathematical operations on vectors and arrays 
* KissFFT library for Fast Fourier Transforms

The sample `bungee` command-line utility also uses:
* cxxopts library for parsing command-line options

See this repo's [.gitmodules](.gitmodules) for versioned links to these projects.

## Bungee Pro

Bungee Pro is a closed-source commercial audio library built on Bungee's API and core philosophy. It uses novel algorithms for sharp and clear professional-grade audio and runs at least as fast as Bungee, thanks to platform-specific performance optimisations.

[Listen to Bungee Pro](https://bungee-audio.pages.dev/compare.html) and consider licensing if you need:
* Powerful, AI-enabled algorithms adaptive to all genres of speech, music and sound with subjective transparency up to infinite time stretch
* Novel frequency- and time-domain processing for crisp transients and presevation of tonal envelope, vocal and instrumental timbre
* Performance optimisations for:
    * Web AudioWorklet with SIMD128 Web Assembler
    * Arm 64-bit for Android, iOS and MacOS
    * x86-64 for Linux, Windows and MacOS
* A ready-to-use Web Audio implementation ([try it](https://bungee-audio.pages.dev/bungee-web-demo.html))

See the Bungee [web pages](https://bungee-audio.pages.dev/) for more information about Bungee Pro.
