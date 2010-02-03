#include <cxxtest/TestSuite.h>

#include "sound/decoders/raw.h"

#include "common/stream.h"
#include "common/endian.h"

#include <math.h>
#include <limits>

class RawStreamTestSuite : public CxxTest::TestSuite
{
public:
	template<typename T>
	T *createSine(int sampleRate, int time) {
		T *sine = (T *)malloc(sizeof(T) * time * sampleRate);

		const T maxValue = std::numeric_limits<T>::max();

		for (int i = 0; i < time * sampleRate; ++i)
			sine[i] = (T)(sin((double)i / sampleRate * 2 * PI) * maxValue);

		// Convert to BE
		if (sizeof(T) == 2) {
			for (int i = 0; i < time * sampleRate; ++i)
				WRITE_BE_UINT16(&sine[i], sine[i]);
		}

		return sine;
	}

	template<typename T>
	Audio::SeekableAudioStream *createSineStream(int sampleRate, int time, const T **sineP) {
		T *sine = createSine<T>(sampleRate, time);

		Common::SeekableReadStream *sD = new Common::MemoryReadStream((const byte *)sine, sizeof(T) * sampleRate * time, DisposeAfterUse::YES);
		Audio::SeekableAudioStream *s = Audio::makeRawStream(sD, sampleRate, (sizeof(T) == 2) ? Audio::FLAG_16BITS : 0);

		if (sineP)
			*sineP = sine;

		return s;
	}

	void test_read_buffer_8_bit_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		const int8 *sine = 0;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, &sine);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);

		for (int i = 0; i < sampleRate * time; ++i)
			TS_ASSERT_EQUALS(buffer[i], sine[i] << 8);

		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}

	void test_read_buffer_16_bit_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		const int16 *sine = 0;
		Audio::SeekableAudioStream *s = createSineStream<int16>(sampleRate, time, &sine);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);

		for (int i = 0; i < sampleRate * time; ++i)
			TS_ASSERT_EQUALS(buffer[i], (int16)READ_BE_UINT16(&sine[i]));

		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}
};
