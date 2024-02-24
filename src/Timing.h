// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "bungee/Bungee.h"

namespace Bungee {

struct Timing
{
	const int log2SynthesisHop;
	const SampleRates sampleRates;

	Timing(SampleRates sampleRates);

	int maxFrameCount() const;

	double calculateInputHop(const Request &request) const;

	void preroll(Request &request) const;

	void next(Request &request) const;
};

} // namespace Bungee
