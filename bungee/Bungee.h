// Copyright (C) 2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Modes.h"

#include <cmath>
#include <cstdint>

namespace Bungee {

const char *version();

struct Request
{
	// Frame-offset within the input audio of the centre-point of this audio grain.
	// NaN signifies an invalid grain that produces no audio output and may be used for flushing.
	double position;

	// Only used when inter-grain hop can't be calculated by comparing position of each grain
	double speed;

	// Adjustment as a frequency multiplier with a value of 1 meaning no pitch adjustment
	double pitch;

	// Set to have the stretcher reset and restart on this grain.
	bool reset;

#define X_BEGIN(Type, type) Type##Mode type##Mode;
#define X_ITEM(Type, type, mode, description)
#define X_END(Type, type)
	BUNGEE_MODES
#undef X_BEGIN
#undef X_ITEM
#undef X_END
};

// Information to describe a chunk of  audio required as input
struct InputChunk
{
	// Sample positions relative to the start of the audio track
	int begin, end;

	// Add unitHop to Request::position for 1x playback speed. Adjust according to desired speed.
	double unitHop;
};

// Describes a chunk of audio output
// Output chunks do not overlap and can be appended for seamless playback
struct OutputChunk
{
	float *data; // audio output data, not aligned
	int frameCount;
	intptr_t channelStride;

	static constexpr int begin = 0, end = 1;
	Request *request[2 /* 0=begin, 1=end */];
};

struct SampleRates
{
	int input;
	int output;
	int log2DecimationFactor;
};

struct Configuration;

struct Stretcher
{
	struct Implementation;
	Implementation *const state;

	Stretcher(SampleRates sampleRates, int channelCount);

	~Stretcher();

	static void defaultRequest(Request &request);

	// Specify the next grain of audio and compute the necessary segment of input audio
	// After calling this function, call analyseGrain.
	InputChunk specifyGrain(const Request &request);

	// Perform first steps of processing the grain. The audio data should correspond to the range
	// specified by specifyGrain's return value. After calling this function, call synthesiseGrain.
	void analyseGrain(const float *data, intptr_t channelStride);

	// Complete processing of the grain of audio that was previously set up with calls to specifyGrain and analyseGrain.
	void synthesiseGrain(OutputChunk &outputChunk);

	bool isFlushed() const;
};

} // namespace Bungee
