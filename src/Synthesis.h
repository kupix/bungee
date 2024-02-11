// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Grain.h"

namespace Bungee::Synthesis {

void synthesise(int log2SynthesisHop, Grain &grain, Grain &previous);

} // namespace Bungee::Synthesis
