#ifndef TEST_SOUND_HELPER_H
#define TEST_SOUND_HELPER_H

#include "audio/decoders/raw.h"

#include "common/stream.h"
#include "common/endian.h"

#include <math.h>
#include <limits>

template<typename T>
static T *createSine(const int sampleRate, const int time) {
	T *sine = (T *)malloc(sizeof(T) * time * sampleRate);

	const bool isUnsigned = !std::numeric_limits<T>::is_signed;
	const T xorMask = isUnsigned ? (1 << (std::numeric_limits<T>::digits - 1)) : 0;
	const T maxValue = std::numeric_limits<T>::max() ^ xorMask;

	for (int i = 0; i < time * sampleRate; ++i)
		sine[i] = ((T)(sin((double)i / sampleRate * 2 * M_PI) * maxValue)) ^ xorMask;

	return sine;
}

template<typename T>
static Common::SeekableReadStream *createPartitionStream(T *sine, const int samples, Audio::RawStreamBlockList &blockList) {
	const int block1Len = samples / 2;
	const int block1Size = block1Len * sizeof(T);
	const int block2Len = samples - block1Len;
	const int block2Size = block2Len * sizeof(T);

	const int bufferLen = samples * 2;
	const int bufferSize = bufferLen * sizeof(T);
	T *partition = (T *)calloc(1, bufferSize);

	Audio::RawStreamBlock block;

	// The will layout the buffer like the following:
	// [Zero], [Part2], [Zero], [Part1]

	// The first part of the stream is at the end of the memory buffer
	block.pos = bufferSize - block1Size;
	block.len = block1Len;
	memcpy(partition + bufferLen - block1Len, sine, block1Size);
	blockList.push_back(block);

	// The second part of the stream is near the beginning of the memory buffer
	block.pos = block2Size;
	block.len = block2Len;
	memcpy(partition + block2Len, sine + block1Len, block2Size);
	blockList.push_back(block);

	free(sine);

	return new Common::MemoryReadStream((const byte *)partition, bufferSize, DisposeAfterUse::YES);
}

template<typename T>
static Audio::SeekableAudioStream *createSineStream(const int sampleRate, const int time, int16 **comp, bool le, bool isStereo, bool makePartition = false) {
	T *sine = createSine<T>(sampleRate, time * (isStereo ? 2 : 1));

	const bool isUnsigned = !std::numeric_limits<T>::is_signed;
	const T xorMask = isUnsigned ? (1 << (std::numeric_limits<T>::digits - 1)) : 0;
	const bool is16Bits = (sizeof(T) == 2);
	assert(sizeof(T) == 2 || sizeof(T) == 1);

	const int samples = sampleRate * time * (isStereo ? 2 : 1);

	if (comp) {
		*comp = new int16[samples];
		for (int i = 0; i < samples; ++i) {
			if (is16Bits)
				(*comp)[i] = sine[i] ^ xorMask;
			else
				(*comp)[i] = (sine[i] ^ xorMask) << 8;
		}
	}

	if (is16Bits) {
		if (le) {
			for (int i = 0; i < samples; ++i)
				WRITE_LE_UINT16(&sine[i], sine[i]);
		} else {
			for (int i = 0; i < samples; ++i)
				WRITE_BE_UINT16(&sine[i], sine[i]);
		}
	}

	Audio::SeekableAudioStream *s = 0;
	if (makePartition) {
		Audio::RawStreamBlockList blockList;
		Common::SeekableReadStream *sD = createPartitionStream<T>(sine, samples, blockList);
		s = Audio::makeRawStream(sD, blockList, sampleRate,
		                             (is16Bits ? Audio::FLAG_16BITS : 0)
		                             | (isUnsigned ? Audio::FLAG_UNSIGNED : 0)
		                             | (le ? Audio::FLAG_LITTLE_ENDIAN : 0)
		                             | (isStereo ? Audio::FLAG_STEREO : 0));
	} else {
		Common::SeekableReadStream *sD = new Common::MemoryReadStream((const byte *)sine, sizeof(T) * samples, DisposeAfterUse::YES);
		s = Audio::makeRawStream(sD, sampleRate,
		                             (is16Bits ? Audio::FLAG_16BITS : 0)
		                             | (isUnsigned ? Audio::FLAG_UNSIGNED : 0)
		                             | (le ? Audio::FLAG_LITTLE_ENDIAN : 0)
		                             | (isStereo ? Audio::FLAG_STEREO : 0));
	}

	return s;
}

#endif

