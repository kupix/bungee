// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Window.h"
#include "Assert.h"
#include "Fourier.h"

#include <Eigen/Dense>

#include <cmath>

namespace Bungee::Window {

Eigen::ArrayXf fromFrequencyDomainCoefficients(int log2Size, float gain, std::initializer_list<float> coefficients)
{
	Eigen::ArrayXcf frequencyDomain(Fourier::binCount(log2Size));

	std::size_t i = 0;
	for (auto c : coefficients)
		frequencyDomain.coeffRef(i++) = c * gain;

	frequencyDomain.bottomRows(frequencyDomain.rows() - i).setZero();

	Eigen::ArrayXf window(1 << log2Size);
	Fourier::transforms.prepareInverse(log2Size);
	Fourier::transforms.inverse(log2Size, window, frequencyDomain);
	return window;
}

template <bool add>
void Apply::special(const Eigen::Ref<const Eigen::ArrayXf> &window, const Eigen::Ref<const Eigen::ArrayXXf> &input, Eigen::Ref<Eigen::ArrayXXf> output)
{
	if constexpr (add)
		output += input.colwise() * window;
	else
		output = input.colwise() * window;
}

template void Apply::special<false>(const Eigen::Ref<const Eigen::ArrayXf> &window, const Eigen::Ref<const Eigen::ArrayXXf> &input, Eigen::Ref<Eigen::ArrayXXf> output);
template void Apply::special<true>(const Eigen::Ref<const Eigen::ArrayXf> &window, const Eigen::Ref<const Eigen::ArrayXXf> &input, Eigen::Ref<Eigen::ArrayXXf> output);

} // namespace Bungee::Window
