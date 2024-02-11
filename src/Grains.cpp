// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Grains.h"

#include "log2.h"

namespace Bungee {

bool Grains::flushed() const
{
	for (auto &grain : vector)
		if (!std::isnan(grain->request.position))
			return false;
	return true;
}

void Grains::rotate()
{
	std::unique_ptr<Grain> grain = std::move(vector.front());
	for (int i = 0; i < vector.size() - 1; ++i)
		vector[i] = std::move(vector[i + 1]);
	vector.back() = std::move(grain);
}

} // namespace Bungee
