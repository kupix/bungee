// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"
#include "bungee/Bungee.h"

#include <Eigen/Dense>

#include <type_traits>

namespace Bungee::Resample {

typedef Eigen::Ref<Eigen::ArrayXXf> Ref;
typedef Eigen::Block<Ref, 1, Eigen::Dynamic, false> Row;

struct FixedToVariable
{
	static inline float applyGain(float coefficient, float)
	{
		return coefficient;
	}

	template <bool first>
	static inline void tap(float fixed, float &__restrict variable, float coefficient)
	{
		if constexpr (first)
			variable = fixed * coefficient;
		else
			variable += fixed * coefficient;
	}
};

struct VariableToFixed
{
	static inline float applyGain(float coefficient, float gain)
	{
		return coefficient * gain;
	}

	template <bool>
	static inline void tap(float &__restrict fixed, float variable, float coefficient)
	{
		fixed += variable * coefficient;
	}
};

struct None
{
	template <class Mode>
	static inline void step(float, Ref, Row, float)
	{
	}
};

struct Nearest
{
	template <class Mode>
	static inline void step(float offset, Ref fixed, Row variable, float gain)
	{
		int x = int(offset + 0.5f);
		for (int c = 0; c < fixed.cols(); ++c)
			Mode::template tap<true>(fixed(x, c), variable(0, c), Mode::applyGain(1.f, gain));
	}
};

struct Bilinear
{
	template <class Mode>
	static inline void step(float offset, Ref fixed, Row variable, float gain)
	{
		int x = int(offset);
		float k = offset - x;
		for (int c = 0; c < fixed.cols(); ++c)
		{
			Mode::template tap<true>(fixed(x + 1, c), variable(0, c), Mode::applyGain(k, gain));
			Mode::template tap<false>(fixed(x, c), variable(0, c), Mode::applyGain(1.f - k, gain));
		}
	}
};

struct Padded
{
	static constexpr auto align = std::max<int>(EIGEN_DEFAULT_ALIGN_BYTES / sizeof(float), 1);
	static constexpr auto padding = (6 + align - 1) / align * align;

	Eigen::ArrayXXf array;
	int frameCount{};
	bool allZeros{true};

	Padded(int maxFrameCount, int channelCount) :
		array(padding + maxFrameCount + padding, channelCount)
	{
	}

	inline Eigen::Ref<Eigen::ArrayXXf> ref(int pad = 0)
	{
		return array.middleRows(padding - pad, array.rows() - 2 * (padding - pad));
	}

	inline Eigen::Ref<Eigen::ArrayXXf> unpadded()
	{
		return array.middleRows(padding, array.rows() - 2 * padding);
	}
};

template <class Mode, class Interpolation, bool ratioChange>
inline void resampleInner(int variableFrameCount, Padded &fixedBuffer, float &fixedBufferOffset, Ref variableBuffer, float ratioBegin, float ratioEnd)
{
	const float ratioGradient = (ratioEnd - ratioBegin) / variableFrameCount;
	BUNGEE_ASSERT1(ratioChange || ratioGradient == 0.f);

	if constexpr (std::is_same_v<Mode, VariableToFixed>)
		fixedBuffer.array.setZero();

	const auto offset = Padded::padding + fixedBufferOffset;
	float ratio = ratioBegin;
	for (int i = 0; i < variableFrameCount; ++i)
		if constexpr (ratioChange)
		{
			const float x = offset + i * (ratioBegin + ratio) * .5f;
			ratio = ratioBegin + ratioGradient * i;
			Interpolation::template step<Mode>(x, fixedBuffer.array, variableBuffer.row(i), ratio);
		}
		else
		{
			const float x = offset + i * ratio;
			Interpolation::template step<Mode>(x, fixedBuffer.array, variableBuffer.row(i), ratio);
		}

	fixedBufferOffset += variableFrameCount * (ratioBegin + ratio) * .5f;
	fixedBufferOffset -= fixedBuffer.frameCount;
}

template <class Mode, class Interpolation>
inline int resample(Padded &fixedBuffer, float &fixedBufferOffset, Ref variableBuffer, float ratioBegin, float ratioEnd, bool alignEnd)
{
	int variableFrameCount = (int)std::round(2 * (fixedBuffer.frameCount + ratioEnd - fixedBufferOffset) / (ratioBegin + ratioEnd) - 1);

	const bool truncate = variableFrameCount > variableBuffer.rows();
	if (truncate)
	{
		BUNGEE_ASSERT1(!"Resample::resample: variableBuffer is too short");
		variableFrameCount = variableBuffer.rows();
	}

	if (alignEnd)
	{
		const float dividend = 2 * (fixedBuffer.frameCount - fixedBufferOffset) - (variableFrameCount + 1) * ratioBegin;
		const float divisor = variableFrameCount - 1;
		ratioEnd = dividend / divisor;
	}

	if (ratioBegin != ratioEnd)
		resampleInner<Mode, Interpolation, true>(variableFrameCount, fixedBuffer, fixedBufferOffset, variableBuffer, ratioBegin, ratioEnd);
	else
		resampleInner<Mode, Interpolation, false>(variableFrameCount, fixedBuffer, fixedBufferOffset, variableBuffer, ratioBegin, ratioEnd);

	if (!(truncate || std::abs(fixedBufferOffset) < (alignEnd ? 1e-2f : (ratioBegin + ratioEnd) * 0.3f)))
	{
		BUNGEE_ASSERT1(!"resample landed badly");
		fixedBufferOffset = 0.f;
	}

	return variableFrameCount;
}

typedef decltype(&resample<FixedToVariable, Nearest>) Function;

struct Operation
{
	Function function;
	float ratio;
};

struct Operations
{
	Operation input, output;

	double setup(const SampleRates &sampleRates, ResampleMode resampleMode, double pitch)
	{
		const double resampleRatio = pitch * sampleRates.input / sampleRates.output;
		input.ratio = 1.f / resampleRatio;
		output.ratio = resampleRatio;

		if constexpr (true)
		{
			input.function = &resample<VariableToFixed, Bilinear>;
			output.function = &resample<FixedToVariable, Bilinear>;
		}
		else
		{
			input.function = &resample<VariableToFixed, Nearest>;
			output.function = &resample<FixedToVariable, Nearest>;
		}

		if (resampleMode == ResampleMode::forceOut)
			input.function = nullptr;
		else if (resampleMode == ResampleMode::forceIn)
			output.function = nullptr;
		else if (resampleRatio == 1.)
			input.function = output.function = nullptr;
		else if (resampleMode == ResampleMode::autoIn)
			output.function = nullptr;
		else if (resampleMode == ResampleMode::autoOut)
			input.function = nullptr;
		else if (resampleMode == ResampleMode::autoInOut && resampleRatio > 1.)
			output.function = nullptr;
		else if (resampleMode == ResampleMode::autoInOut && resampleRatio < 1.)
			input.function = nullptr;
		else
		{
			BUNGEE_ASSERT1(false);
			input.function = nullptr;
		}

		if (!input.function)
			input.ratio = 1.f;
		if (!output.function)
			output.ratio = 1.f;

		return ((double)sampleRates.input / sampleRates.output) / output.ratio;
	}
};

} // namespace Bungee::Resample
