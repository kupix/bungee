// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"
#include "Dispatch.h"

#include <Eigen/Dense>

#include <initializer_list>

namespace Bungee::Window {

Eigen::ArrayXf fromFrequencyDomainCoefficients(int log2Size, float gain, std::initializer_list<float> coefficients);

template <int index>
struct Apply
{
	static constexpr bool add = !!index;
	static void receive(const Eigen::Ref<const Eigen::ArrayXf> &window, const Eigen::Ref<const Eigen::ArrayXXf> &input, Eigen::Ref<Eigen::ArrayXXf> output);
};

typedef Dispatch<Apply, 2> DispatchApply;

} // namespace Bungee::Window
