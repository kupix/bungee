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

	// Output audio speed. Value of 1 means speed should be unchanged relative to the input audio.
	// Used by Stretcher's internal algorithms only when it's not possible to determine speed by
	// subtracting Request::position of previous grain from Request::position of current grain.
	double speed;

	// Adjustment as a frequency multiplier with a value of 1 meaning no pitch adjustment
	double pitch;

	// Set to have the stretcher forget all previous grains and restart on this grain.
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

	int frameCount() const
	{
		return end - begin;
	}
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
};

struct Configuration;

struct Stretcher
{
	struct Implementation;
	Implementation *const state;

	Stretcher(SampleRates sampleRates, int channelCount);

	~Stretcher();

	// Returns the largest number of frames that might be requested by specifyGrain()
	// This helps the caller to allocate large enough buffers because it is guaranteed that
	// InputChunk::frameCount() will not exceed this number.
	int maxInputFrameCount() const;

	// This function adjusts request.position so that the stretcher has a run in of a few
	// grains before hitting the requested position. Without preroll, the first milliseconds
	// of audio might sound weak or initial transients might be lost.
	void preroll(Request &request) const;

	// This function prepares request.position and request.reset for the subsequent grain.
	// Typically called within a granular loop where playback at constant request.speed is desired.
	void next(Request &request) const;

	// Specify a grain of audio and compute the necessary segment of input audio.
	// After calling this function, call analyseGrain.
	InputChunk specifyGrain(const Request &request);

	// Begins processing the grain. The audio data should correspond to the range
	// specified by specifyGrain's return value. After calling this function, call synthesiseGrain.
	void analyseGrain(const float *data, intptr_t channelStride);

	// Complete processing of the grain of audio that was previously set up with calls to specifyGrain and analyseGrain.
	void synthesiseGrain(OutputChunk &outputChunk);

	// Returns true if every grain in the stretcher's pipeline is invalid (its Request::position was NaN).
	bool isFlushed() const;
};

} // namespace Bungee
