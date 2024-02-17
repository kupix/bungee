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

	for (bool done = false; !done;)
	{
		InputChunk inputChunk = stretcher.specifyGrain(request);

		stretcher.analyseGrain(processor.getInputAudio(inputChunk), processor.inputChannelStride);

		OutputChunk outputChunk;
		stretcher.synthesiseGrain(outputChunk);

		stretcher.next(request);

		done = processor.write(outputChunk);
	}

	processor.writeOutputFile();

	return 0;
}
