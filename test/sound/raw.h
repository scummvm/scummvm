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
	static T *createSine(int sampleRate, int time) {
		T *sine = (T *)malloc(sizeof(T) * time * sampleRate);

		const T maxValue = std::numeric_limits<T>::max();

		for (int i = 0; i < time * sampleRate; ++i)
			sine[i] = (T)(sin((double)i / sampleRate * 2 * PI) * maxValue);

		return sine;
	}

	static Audio::SeekableAudioStream *createSineStream8Bit(int sampleRate, int time, const int8 **sineP) {
		int8 *sine = createSine<int8>(sampleRate, time);

		Common::SeekableReadStream *sD = new Common::MemoryReadStream((const byte *)sine, sampleRate * time, DisposeAfterUse::YES);
		Audio::SeekableAudioStream *s = Audio::makeRawStream(sD, sampleRate, 0);

		if (sineP)
			*sineP = sine;

		return s;
	}

	static Audio::SeekableAudioStream *createSineStream16Bit(int sampleRate, int time, bool le, const int16 **sineP) {
		int16 *sine = createSine<int16>(sampleRate, time);

		if (le) {
			for (int i = 0; i < sampleRate * time; ++i)
				WRITE_LE_UINT16(&sine[i], sine[i]);
		} else {
			for (int i = 0; i < sampleRate * time; ++i)
				WRITE_BE_UINT16(&sine[i], sine[i]);
		}

		Common::SeekableReadStream *sD = new Common::MemoryReadStream((const byte *)sine, sizeof(int16) * sampleRate * time, DisposeAfterUse::YES);
		Audio::SeekableAudioStream *s = Audio::makeRawStream(sD, sampleRate, Audio::FLAG_16BITS | (le ? Audio::FLAG_LITTLE_ENDIAN : 0));

		if (sineP)
			*sineP = sine;

		return s;
	}

	void test_read_buffer_8_bit_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		const int8 *sine;
		Audio::SeekableAudioStream *s = createSineStream8Bit(sampleRate, time, &sine);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);

		for (int i = 0; i < sampleRate * time; ++i)
			TS_ASSERT_EQUALS(buffer[i], sine[i] << 8);

		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}

	void test_read_buffer_16_bit_be_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		const int16 *sine = 0;
		Audio::SeekableAudioStream *s = createSineStream16Bit(sampleRate, time, false, &sine);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);

		for (int i = 0; i < sampleRate * time; ++i)
			TS_ASSERT_EQUALS(buffer[i], (int16)READ_BE_UINT16(&sine[i]));

		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}

	void test_read_buffer_16_bit_le_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		const int16 *sine = 0;
		Audio::SeekableAudioStream *s = createSineStream16Bit(sampleRate, time, true, &sine);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);

		for (int i = 0; i < sampleRate * time; ++i)
			TS_ASSERT_EQUALS(buffer[i], (int16)READ_LE_UINT16(&sine[i]));

		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}
};
