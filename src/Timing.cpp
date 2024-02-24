// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Timing.h"
#include "Grain.h"
#include "Resample.h"
#include "log2.h"

#include "bungee/Bungee.h"

namespace Bungee {

Timing::Timing(SampleRates sampleRates) :
	log2SynthesisHop(log2<true>(sampleRates.input) - 6),
	sampleRates(sampleRates)
{
}

int Timing::maxFrameCount() const
{
	static constexpr auto maxPitchOctaves = 2;
	auto maxResample = (sampleRates.output << (maxPitchOctaves + log2SynthesisHop)) / sampleRates.input;
	return maxResample + 1;
}

double Timing::calculateInputHop(const Request &request) const
{
	const double unitHop = (1 << log2SynthesisHop) * Resample::Operations().setup(sampleRates, request.resampleMode, request.pitch);
	return unitHop * request.speed;
}

void Timing::preroll(Request &request) const
{
	request.position -= 4. * calculateInputHop(request);
	request.reset = true;
}

void Timing::next(Request &request) const
{
	if (!std::isnan(request.speed) && !std::isnan(request.position))
	{
		request.position += calculateInputHop(request);
		request.reset = false;
	}
}

} // namespace Bungee
