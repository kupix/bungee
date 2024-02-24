// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Stretcher.h"
#include "Resample.h"
#include "Synthesis.h"
#include "log2.h"

namespace Bungee {

Stretcher::Stretcher(SampleRates sampleRates, int channelCount) :
	state(new Implementation(sampleRates, channelCount))
{
}

Stretcher::~Stretcher()
{
	delete state;
}

InputChunk Stretcher::specifyGrain(const Request &request)
{
	return state->specifyGrain(request);
}

void Stretcher::next(Request &request) const
{
	state->next(request);
}

void Stretcher::preroll(Request &request) const
{
	state->preroll(request);
}

void Stretcher::analyseGrain(const float *data, intptr_t channelStride)
{
	state->analyseGrain(data, channelStride);
}

void Stretcher::synthesiseGrain(OutputChunk &outputChunk)
{
	state->synthesiseGrain(outputChunk);
}

bool Stretcher::isFlushed() const
{
	return state->grains.flushed();
}

Stretcher::Implementation::Implementation(SampleRates sampleRates, int channelCount) :
	Timing(sampleRates),
	input(log2SynthesisHop, channelCount),
	grains(4),
	output(log2SynthesisHop, channelCount, maxFrameCount(), 0.25f, {1.f, 0.5f})
{
	for (auto &grain : grains.vector)
		grain = std::make_unique<Grain>(log2SynthesisHop, channelCount);
}

InputChunk Stretcher::Implementation::specifyGrain(const Request &request)
{
	const Assert::FloatingPointExceptions floatingPointExceptions(0);

	grains.rotate();

	auto &grain = grains[0];
	auto &previous = grains[1];
	return grain.specify(request, previous, sampleRates, log2SynthesisHop);
}

void Stretcher::Implementation::analyseGrain(const float *data, std::ptrdiff_t stride)
{
	const Assert::FloatingPointExceptions floatingPointExceptions(FE_INEXACT | FE_UNDERFLOW | FE_DENORMALOPERAND);

	auto &grain = grains[0];
	grain.validBinCount = 0;
	if (grain.valid())
	{
		auto m = grain.inputChunkMap(data, stride);
		auto ref = grain.resampleInput(m, 8 << log2SynthesisHop);

		auto log2TransformLength = input.applyAnalysisWindow(ref);

		Fourier::transforms.forward(log2TransformLength, input.windowedInput, grain.transformed);

		const auto n = Fourier::binCount(grain.log2TransformLength) - 1;
		grain.validBinCount = std::min<int>(std::ceil(n / grain.resampleOperations.output.ratio), n) + 1;
		grain.transformed.middleRows(grain.validBinCount, n + 1 - grain.validBinCount).setZero();

		grain.log2TransformLength = log2TransformLength;

		for (int i = 0; i < grain.validBinCount; ++i)
		{
			const auto x = grain.transformed.row(i).sum();
			grain.energy[i] = x.real() * x.real() + x.imag() * x.imag();
			grain.phase[i] = Phase::fromRadians(std::arg(x));
		}

		Partials::enumerate(grain.partials, grain.validBinCount, grain.energy);

		if (grain.continuous)
			Partials::suppressTransientPartials(grain.partials, grain.energy, grains[1].energy);
	}
}

void Stretcher::Implementation::synthesiseGrain(OutputChunk &outputChunk)
{
	const Assert::FloatingPointExceptions floatingPointExceptions(FE_INEXACT);

	auto &grain = grains[0];
	if (grain.valid())
	{
		auto n = Fourier::binCount(grain.log2TransformLength);

		BUNGEE_ASSERT1(!grain.passthrough || grain.analysis.speed == grain.passthrough);

		Synthesis::synthesise(log2SynthesisHop, grain, grains[1]);

		BUNGEE_ASSERT2(!grain.passthrough || grain.rotation.topRows(grain.validBinCount).isZero());

		auto theta = grain.rotation.topRows(grain.validBinCount).cast<float>() * (std::numbers::pi_v<float> / 0x8000);
		auto t = theta.cos() + theta.sin() * std::complex<float>{0, 1};
		if (grain.reverse())
			grain.transformed.topRows(grain.validBinCount) = grain.transformed.topRows(grain.validBinCount).conjugate().colwise() * t;
		else
			grain.transformed.topRows(grain.validBinCount).colwise() *= t;

		Fourier::transforms.inverse(grain.log2TransformLength, output.inverseTransformed, grain.transformed);
	}

	output.applySynthesisWindow(log2SynthesisHop, grains, output.synthesisWindow);

	Output::Segment::lapPadding(grains[3].segment, grains[2].segment);

	outputChunk = grains[3].segment.resample(
		output.resampleOffset,
		grains[2].resampleOperations.output,
		grains[1].resampleOperations.output,
		output.bufferResampled);

	outputChunk.request[OutputChunk::begin] = &grains[2].request;
	outputChunk.request[OutputChunk::end] = &grains[1].request;
}

} // namespace Bungee
