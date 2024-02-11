// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Grains.h"
#include "Input.h"
#include "Output.h"

namespace Bungee {

struct Stretcher::Implementation
{
	int log2SynthesisHop;
	SampleRates sampleRates;
	Input input;
	Grains grains;
	Output output;

	static int maxFrameCount(SampleRates sampleRates);

	Implementation(SampleRates sampleRates, int channelCount);

	InputChunk specifyGrain(const Request &request);

	void analyseGrain(const float *inputAudio, std::ptrdiff_t stride);

	void synthesiseGrain(OutputChunk &outputChunk);

	bool isFlushed() const;
};

} // namespace Bungee
