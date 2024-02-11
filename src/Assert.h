// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cfenv>
#include <complex>
#include <cstdlib>
#include <vector>

#ifndef BUNGEE_SELF_TEST
#	define BUNGEE_SELF_TEST 0 // no checks
// #define BUNGEE_SELF_TEST 1 // fast checks only
// #define BUNGEE_SELF_TEST 2 // all checks
#endif

namespace Bungee::Assert {

static constexpr int level = BUNGEE_SELF_TEST;

void fail(int level, const char *m2, const char *file, int line);

#define BUNGEE_ASSERT(l, condition) \
	do \
	{ \
		if constexpr (l <= Bungee::Assert::level) \
			if (!(condition)) \
				Bungee::Assert::fail(l, #condition, __FILE__, __LINE__); \
	} while (false)

// Checks with cost O(N) (for tests applied a small number of times per grain)
#define BUNGEE_ASSERT1(condition) BUNGEE_ASSERT(1, condition)

// Checks with cost O(N*N) (for tests applied a small number of times per bin or per sample)
#define BUNGEE_ASSERT2(condition) BUNGEE_ASSERT(2, condition)

#ifndef FE_INEXACT
#	define FE_INEXACT 0
#	define FE_UNDERFLOW 0
#	define FE_INVALID 0
#	define FE_OVERFLOW 0
#	define FE_DIVBYZERO 0
#endif
#ifndef FE_DENORMALOPERAND
#	define FE_DENORMALOPERAND 0
#endif

struct FloatingPointExceptions
{
#if BUNGEE_SELF_TEST
	int allowed;
	std::fenv_t original;

	FloatingPointExceptions(int allowed);
	~FloatingPointExceptions();
#else
	inline FloatingPointExceptions(int) {}
#endif
};

static constexpr auto active = BUNGEE_SELF_TEST == 2;

template <typename T, bool enabled>
struct BufferCopyBase
{
};

template <typename T>
struct BufferCopyBase<T, true>
{
	std::vector<T> data;
};

template <typename T, bool enabled = BUNGEE_SELF_TEST == 2>
struct BufferCopy : BufferCopyBase<T, enabled>
{
	T *p{};
	int rows = 1;
	int stride{};

	inline BufferCopy()
	{
	}

	inline BufferCopy(T *t, int rows, [[maybe_unused]] int cols = 1, int stride = 0) :
		p(t),
		rows(rows),
		stride(stride)
	{
		if constexpr (enabled)
			for (int c = 0; c < cols; ++c)
				BufferCopyBase<T, enabled>::data.insert(BufferCopyBase<T, enabled>::data.end(), t + c * stride, t + c * stride + rows);
	}

	inline void restore() const
	{
		if constexpr (enabled)
			if (p)
				for (unsigned c = 0; c < BufferCopyBase<T, enabled>::data.size() / rows; ++c)
					std::copy(&BufferCopyBase<T, enabled>::data[c * rows], &BufferCopyBase<T, enabled>::data[(c + 1) * rows], &p[c * stride]);
	}

	template <typename Callback>
	inline void iterate(Callback callback) const
	{
		if constexpr (enabled)
			for (int c = 0; c < (int)BufferCopyBase<T, enabled>::data.size() / rows; ++c)
				for (int r = 0; r < rows; ++r)
				{
					const auto &copy = BufferCopyBase<T, enabled>::data[r + c * rows];
					const auto &current = p[r + c * stride];
					callback(r, c, copy, current);
				}
	}

	inline float worstAbsError() const
	{
		using namespace std;
		float worst{};
		iterate([&](int, int, T copy, T current) {
			worst = max(worst, abs<float>(current - copy));
		});
		return worst;
	}

	inline float worstPrecision(float minimumDivisor = 1e-6f) const
	{
		using namespace std;
		float worst{};
		iterate([&](int, int, T copy, T current) {
			const auto dividend = abs<float>(current - copy);
			const auto divisor = max(max(abs<float>(copy), abs<float>(current)), minimumDivisor);
			worst = max<float>(worst, dividend / divisor);
		});
		return worst;
	}

	inline bool doRunGeneric() const
	{
		return enabled || !p;
	}
};

} // namespace Bungee::Assert