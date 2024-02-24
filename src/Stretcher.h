// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Grains.h"
#include "Input.h"
#include "Output.h"
#include "Timing.h"

namespace Bungee {

struct Stretcher::Implementation :
	Timing
{
	Input input;
	Grains grains;
	Output output;

	Implementation(SampleRates sampleRates, int channelCount);

	InputChunk specifyGrain(const Request &request);

	void analyseGrain(const float *inputAudio, std::ptrdiff_t stride);

	void synthesiseGrain(OutputChunk &outputChunk);

	bool isFlushed() const;
};

} // namespace Bungee
