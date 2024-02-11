// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Partials.h"

namespace Bungee::Partials {

void enumerate(std::vector<Partial> &partials, int n, Eigen::Ref<Eigen::ArrayX<float>> energy)
{
	float undo[] = {-1.f, 0.f};
	std::swap(energy[n], undo[0]);
	std::swap(energy[n + 1], undo[1]);

	partials.resize(partials.capacity());

	int count = 0;
	int m = 1;
	do
	{
		while ((energy[m] < energy[m + 1]))
			m++;
		partials[count].peak = m++;

		while (!(energy[m] < energy[m + 1]))
			m++;
		partials[count++].valley = m++;

	} while (m < n + 1);

	partials.resize(count);

	BUNGEE_ASSERT1(partials.back().valley == n);

	std::swap(energy[n], undo[0]);
	std::swap(energy[n + 1], undo[1]);
}

} // namespace Bungee::Partials
