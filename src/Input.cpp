// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Input.h"
#include "Grain.h"
#include "log2.h"

#include <numbers>

namespace Bungee {

namespace {
static constexpr float pi = std::numbers::pi_v<float>;
static constexpr float gain = (3 * pi) / (3 * pi + 8);
} // namespace

Input::Input(int log2SynthesisHop, int channelCount) :
	analysisWindowBasic(Window::fromFrequencyDomainCoefficients(log2SynthesisHop + 3, gain / (8 << log2SynthesisHop), {1.f, 0.5f})),
	windowedInput{(8 << log2SynthesisHop), channelCount}
{
	windowedInput.setZero();
	Fourier::transforms.prepareForward(log2SynthesisHop + 3);
}

int Input::applyAnalysisWindow(const Eigen::Ref<const Eigen::ArrayXXf> &input)
{
	const auto half = analysisWindowBasic.rows() / 2;
	Window::Apply<0>::receive(analysisWindowBasic.head(half), input.bottomRows(input.rows() / 2).topRows(half), windowedInput.topRows(half));
	Window::Apply<0>::receive(analysisWindowBasic.tail(half), input.topRows(input.rows() / 2).bottomRows(half), windowedInput.bottomRows(half));
	return Bungee::log2(windowedInput.rows());
}

} // namespace Bungee
