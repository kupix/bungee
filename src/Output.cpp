// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Output.h"
#include "Grains.h"
#include "Window.h"

namespace Bungee {

int Output::maxFrameCount(int log2SynthesisHop, SampleRates sampleRates)
{
	static constexpr auto maxPitchOctaves = 2;
	auto maxResample = (sampleRates.input << (maxPitchOctaves + log2SynthesisHop)) / sampleRates.output;
	return maxResample + 1;
}

Output::Output(int log2SynthesisHop, int channelCount, int maxOutputChunkSize, float windowGain, std::initializer_list<float> windowCoefficients) :
	synthesisWindow{Window::fromFrequencyDomainCoefficients(log2SynthesisHop + 2, windowGain, windowCoefficients)},
	inverseTransformed(8 << log2SynthesisHop, channelCount),
	bufferResampled(maxOutputChunkSize, channelCount)
{
	Fourier::transforms.prepareInverse(log2SynthesisHop + 3);
}

void Output::applySynthesisWindow(int log2SynthesisHop, Grains &grains, const Eigen::Ref<const Eigen::ArrayXf> &window)
{
	const auto quadrantSize = window.rows() / 4;
	const auto hopsPerTransform = 1 << (grains[0].log2TransformLength - log2SynthesisHop);

	grains[0].segment.bufferLapped.frameCount = 0;
	grains[0].segment.bufferLapped.allZeros = true;

	for (int i = 0; i < 4; ++i)
	{
		auto &quandrant = grains[3 - i].segment.bufferLapped;

		if (grains[0].valid())
		{
			auto windowSegment = window.segment(quadrantSize * (i ^ 2), quadrantSize);

			auto j = (i + hopsPerTransform - 2) % hopsPerTransform;
			auto inputSegment = inverseTransformed.middleRows(quadrantSize * j, quadrantSize);

			const bool add = quandrant.frameCount != 0;
			dispatchApply[add](windowSegment, inputSegment, quandrant.unpadded().topRows(quadrantSize));
			quandrant.allZeros = false;
		}
		else
		{
			if (!quandrant.frameCount)
				quandrant.unpadded().topRows(quadrantSize).setZero();
		}

		quandrant.frameCount = quadrantSize;
	}

	grains[2].segment.needsResample =
		grains[1].resampleOperationOutput.function ||
		grains[0].resampleOperationOutput.function;
}

Output::Segment::Segment(int log2FrameCount, int channelCount) :
	bufferLapped(1 << log2FrameCount, channelCount)
{
}

void Output::Segment::lapPadding(Segment &current, Segment &next)
{
	constexpr auto n = Resample::Padded::padding;

	if (current.needsResample)
	{
		if (next.bufferLapped.allZeros)
			current.bufferLapped.array.middleRows(n + current.bufferLapped.frameCount, n).setZero();
		else
			current.bufferLapped.array.middleRows(n + current.bufferLapped.frameCount, n) = next.bufferLapped.array.middleRows(n, n);
	}

	if (current.needsResample || next.needsResample)
	{
		if (current.bufferLapped.allZeros)
			next.bufferLapped.array.topRows(n).setZero();
		else
			next.bufferLapped.array.topRows(n) = current.bufferLapped.array.middleRows(current.bufferLapped.frameCount, n);
	}
}

inline OutputChunk Output::Segment::outputChunk(Eigen::Ref<Eigen::ArrayXXf> ref, bool allZeros)
{
	if (allZeros)
		ref.setZero();

	OutputChunk outputChunk{};
	outputChunk.data = ref.data();
	outputChunk.frameCount = ref.rows();
	outputChunk.channelStride = ref.stride();
	return outputChunk;
}

OutputChunk Output::Segment::resample(float &resampleOffset, Resample::Operation resampleOperationBegin, Resample::Operation resampleOperationEnd, Eigen::Ref<Eigen::ArrayXXf> bufferResampled)
{
	if (!resampleOperationBegin.function)
		resampleOperationBegin.ratio = 1.f;

	if (!resampleOperationEnd.function)
	{
		resampleOperationEnd.ratio = 1.f;
		resampleOperationEnd.function = resampleOperationBegin.function;
	}

	if (resampleOperationEnd.function)
	{
		if (bufferLapped.allZeros)
			resampleOperationEnd.function = &Resample::resample<Resample::FixedToVariable, Resample::None>;

		const bool alignEnd = resampleOperationEnd.ratio == 1.f;
		const auto frameCount = resampleOperationEnd.function(bufferLapped, resampleOffset, bufferResampled, resampleOperationBegin.ratio, resampleOperationEnd.ratio, alignEnd);

		return outputChunk(bufferResampled.topRows(frameCount), bufferLapped.allZeros);
	}
	else
	{
		return outputChunk(bufferLapped.unpadded().topRows(bufferLapped.frameCount), bufferLapped.allZeros);
	}
}

} // namespace Bungee
