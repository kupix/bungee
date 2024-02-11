// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"

#include <Eigen/Dense>

#include <cstdint>
#include <vector>

namespace Bungee::Partials {

struct Partial
{
	int16_t peak;
	int16_t valley;
};

void enumerate(std::vector<Partial> &partials, int n, Eigen::Ref<Eigen::ArrayX<float>> energy);

} // namespace Bungee::Partials
