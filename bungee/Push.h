// Copyright (C) 2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cassert> // remove
#include <vector>

#include "Bungee.h"

namespace Bungee::Push {

// Bungee::Push::InputBuffer is an optional component that assists users of Bungee::Stretcher
// n applications where function calls "push" audio downstream.
//
// There are two fundamental philosophies for how function calls propagate audio data
// when processing streamed audio data:
// "Pull": processing nodes in the pipeline call upstream to request chunks of audio data
// "Push": processing nodes call downstream to deliver chunks of audio data
//
// Bungee's native API is pull-based. It has a number of advantages:
// * it permits ultimate flexibility in frozen or reverse play,
// * it simplifies real-time, low-latency operation,
// * it minimises the need for buffers and copying audio data,
// * it allows clear association of metadata and timestamps with audio chunks, and,
// * it allows granular, low latency control of speed and pitch.
//
// That said, many existing audio pipeline designs use a push philosophy. This adapter is
// provided to assist developers integrate Bungee into an application where function calls
// are used to push audio downstream ("push" operation). In such situations, Bungee
// cannot reverse through the audio stream, it can only progress forwards although
// speed and pitch controls will work as usual.
//
// This adapter buffers audio in order to provide the overlapping input grains
// required by Bungee::Stretcher. Example usage may be found in ../cmd/main.cpp.
//
struct InputBuffer
{
	struct Implementation;
	Implementation *const state;

	InputBuffer(int maxInputFrameCount, int channelCount);
	~InputBuffer();

	void grain(const InputChunk &inputChunk);

	void deliver(int frameCount);

	float *inputData();
	int inputFrameCountRequired() const;
	int inputFrameCountMax() const;
	const float *outputData() const;
	int stride() const;
};

} // namespace Bungee::Push
