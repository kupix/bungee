// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Fourier.h"
#include "Assert.h"

#include "kissfft/kiss_fftr.h"

namespace Bungee::Fourier {

struct Kiss
{
	struct KernelBase
	{
		void *implementation;
		~KernelBase();
	};

	template <bool isInverse>
	struct Kernel :
		KernelBase
	{
		Kernel(int log2TransformLength);

		void forward(int log2TransformLength, float *t, std::complex<float> *f) const;
		void inverse(int log2TransformLength, float *t, std::complex<float> *f) const;
	};

	typedef Kernel<false> Forward;
	typedef Kernel<true> Inverse;
};

template <bool isInverse>
Kiss::Kernel<isInverse>::Kernel(int log2TransformLength) :
	Kiss::KernelBase{kiss_fftr_alloc(1 << log2TransformLength, isInverse, nullptr, nullptr)}
{
}

Kiss::KernelBase::~KernelBase()
{
	KISS_FFT_FREE(implementation);
}

template <bool isInverse>
void Kiss::Kernel<isInverse>::forward(int, float *t, std::complex<float> *f) const
{
	static_assert(sizeof(*f) == sizeof(kiss_fft_cpx));
	BUNGEE_ASSERT1(!isInverse);
	kiss_fftr((kiss_fftr_cfg)implementation, t, (kiss_fft_cpx *)f);
}

template <bool isInverse>
void Kiss::Kernel<isInverse>::inverse(int, float *t, std::complex<float> *f) const
{
	static_assert(sizeof(*f) == sizeof(kiss_fft_cpx));
	BUNGEE_ASSERT1(isInverse);
	kiss_fftri((kiss_fftr_cfg)implementation, (kiss_fft_cpx *)f, t);
}

static Fourier::Cache<Kiss, 16> cache;
Transforms &transforms = cache;

} // namespace Bungee::Fourier
