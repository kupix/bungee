// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Fourier.h"
#include "Resample.h"
#include "Window.h"

#include "bungee/Bungee.h"

#include <Eigen/Dense>

#include <initializer_list>

namespace Bungee {

struct Grains;

struct Output
{
	Eigen::ArrayXf synthesisWindow;
	Eigen::ArrayXXf inverseTransformed;
	Eigen::ArrayXXf bufferResampled;
	float resampleOffset = 0.f;
	Window::DispatchApply dispatchApply;

	static int maxFrameCount(int log2SynthesisHop, SampleRates sampleRates);

	Output(int log2SynthesisHop, int channelCount, int maxOutputChunkSize, float windowGain, std::initializer_list<float> windowCoefficients);

	void applySynthesisWindow(int log2SynthesisHop, Grains &grains, const Eigen::Ref<const Eigen::ArrayXf> &window);

	struct Segment
	{
		Resample::Padded bufferLapped;
		bool needsResample;

		Segment(int log2FrameCount, int channelCount);
		static inline OutputChunk outputChunk(Eigen::Ref<Eigen::ArrayXXf> ref, bool allZeros);
		static void lapPadding(Segment &current, Segment &next);
		OutputChunk resample(float &resampleOffset, Resample::Operation resampleOperationBegin, Resample::Operation resampleOperationEnd, Eigen::Ref<Eigen::ArrayXXf> bufferResampled);
	};
};

} // namespace Bungee
