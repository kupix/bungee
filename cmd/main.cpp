// Copyright (C) 2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "bungee/../src/log2.h"
#include "bungee/Bungee.h"
#include "bungee/CommandLine.h"

int main(int argc, const char *argv[])
{
	using namespace Bungee;

	Request request{};

	constexpr int log2SynthesisHop = 9;

	CommandLine::Options options;
	CommandLine::Parameters parameters{options, argc, argv, request};
	CommandLine::Processor processor{parameters, request};

	Stretcher stretcher(processor.sampleRates, processor.channelCount);

	processor.restart(request);
	stretcher.preroll(request);

	const int pushFrameCount = parameters["push"].as<int>();
	if (pushFrameCount)
	{
		// This code exists only to demonstrate the usage of the Bungee stretcher with the Push::InputBuffer
		// See the else part of the code for an example of the native "pull" API.

		std::cout << "Using Push::InputBuffer with " << pushFrameCount << " frames per push\n";

		InputChunk inputChunk = stretcher.specifyGrain(request);

		Push::InputBuffer pushInputBuffer(stretcher.maxInputFrameCount() + pushFrameCount, processor.channelCount);

		pushInputBuffer.grain(inputChunk);

		bool done = false;
		for (int position = 0; !done; position += pushFrameCount)
		{
			// Here we loop over segments of input audio, each with pushFrameCount audio frames.

			// First get pushFrameCount frames of audio from the input
			processor.getInputAudio(pushInputBuffer.inputData(), pushInputBuffer.stride(), position, pushFrameCount);

			// The following function and loop delivers pushFrameCount to Bungee
			// Zero or more output audio chunks will be emitted and we concatenate these.
			pushInputBuffer.deliver(pushFrameCount);
			while (pushInputBuffer.inputFrameCountRequired() <= 0)
			{
				stretcher.analyseGrain(pushInputBuffer.outputData(), pushInputBuffer.stride());

				OutputChunk outputChunk;
				stretcher.synthesiseGrain(outputChunk);

				stretcher.next(request);
				done = processor.write(outputChunk);

				inputChunk = stretcher.specifyGrain(request);
				pushInputBuffer.grain(inputChunk);
			}
		}
	}
	else
	{
		// Regular pull API

		for (bool done = false; !done;)
		{
			InputChunk inputChunk = stretcher.specifyGrain(request);

			stretcher.analyseGrain(processor.getInputAudio(inputChunk), processor.inputChannelStride);
			OutputChunk outputChunk;
			stretcher.synthesiseGrain(outputChunk);

			stretcher.next(request);

			done = processor.write(outputChunk);
		}
	}

	processor.writeOutputFile();

	return 0;
}
