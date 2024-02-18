// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <array>

namespace Bungee {

template <class Target, int n>
struct Dispatch
{
	typedef decltype(&Target::template special<0>) FunctionPointer;

	std::array<FunctionPointer, n> table;

	template <class T, int i = 0>
	inline constexpr void populateTable()
	{
		table[i] = &T::template special<i>;
		if constexpr (i + 1 < n)
			populateTable<T, i + 1>();
	}

	constexpr Dispatch()
	{
		populateTable<Target>();
	}

	inline auto operator[](int i) const
	{
		return table[i];
	}
};

} // namespace Bungee