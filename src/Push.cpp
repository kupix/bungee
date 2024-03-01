// Copyright(C) 2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "bungee/Push.h"
#include "Assert.h"

namespace Bungee::Push {

struct InputBuffer::Implementation
{
	std::vector<float> vector;
	int maxInputFrameCount;
	int begin = 0;
	int end = -1;
	int endRequired = 0;
};

InputBuffer::InputBuffer(int maxInputFrameCount, int channelCount) :
	state(new Implementation)
{
	state->vector.resize(maxInputFrameCount * channelCount);
	state->maxInputFrameCount = maxInputFrameCount;
}

InputBuffer::~InputBuffer()
{
	delete state;
}

void InputBuffer::grain(const InputChunk &inputChunk)
{
	const bool firstCall = state->end - state->begin < 0;
	if (firstCall)
	{
		state->begin = inputChunk.begin;
		state->end = 0;
	}

	const int overlap = state->end - inputChunk.begin;
	if (overlap <= 0)
	{
		state->begin = state->end = inputChunk.begin;
	}
	else
	{
		const int offset = inputChunk.begin - state->begin;

		// loop over channels, move lapped segment to start of buffer
		for (int x = 0; x < (int)state->vector.size(); x += stride())
			std::move(
				&state->vector[x + offset],
				&state->vector[x + offset + overlap],
				&state->vector[x]);

		state->begin = inputChunk.begin;
	}
	state->endRequired = inputChunk.end;

	BUNGEE_ASSERT1(inputFrameCountRequired() <= inputFrameCountMax());
	BUNGEE_ASSERT1(inputFrameCountMax() >= 0);
}

void InputBuffer::deliver(int frameCount)
{
	BUNGEE_ASSERT1(frameCount >= 0);
	BUNGEE_ASSERT1(frameCount <= inputFrameCountMax());
	state->end += frameCount;
}

float *InputBuffer::inputData()
{
	return &state->vector[state->end - state->begin];
}

int InputBuffer::inputFrameCountRequired() const
{
	return state->endRequired - state->end;
}

int InputBuffer::inputFrameCountMax() const
{
	return stride() - (state->end - state->begin);
}

const float *InputBuffer::outputData() const
{
	return state->vector.data();
}

int InputBuffer::stride() const
{
	return state->maxInputFrameCount;
}

} // namespace Bungee::Push
