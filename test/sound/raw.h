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
	static T *createSine(const int sampleRate, const int time) {
		T *sine = (T *)malloc(sizeof(T) * time * sampleRate);

		const bool isUnsigned = !std::numeric_limits<T>::is_signed;
		const T xorMask = isUnsigned ? (1 << (std::numeric_limits<T>::digits - 1)) : 0;
		const T maxValue = std::numeric_limits<T>::max() ^ xorMask;

		for (int i = 0; i < time * sampleRate; ++i)
			sine[i] = ((T)(sin((double)i / sampleRate * 2 * PI) * maxValue)) ^ xorMask;

		return sine;
	}

	template<typename T>
	static Audio::SeekableAudioStream *createSineStream(const int sampleRate, const int time, int16 **comp, bool le) {
		T *sine = createSine<T>(sampleRate, time);

		const bool isUnsigned = !std::numeric_limits<T>::is_signed;
		const T xorMask = isUnsigned ? (1 << (std::numeric_limits<T>::digits - 1)) : 0;
		const bool is16Bits = (sizeof(T) == 2);
		assert(sizeof(T) == 2 || sizeof(T) == 1);

		if (comp) {
			*comp = new int16[sampleRate * time];
			for (int i = 0; i < sampleRate * time; ++i) {
				if (is16Bits)
					(*comp)[i] = sine[i] ^ xorMask;
				else
					(*comp)[i] = (sine[i] ^ xorMask) << 8;
			}
		}

		if (is16Bits) {
			if (le) {
				for (int i = 0; i < sampleRate * time; ++i)
					WRITE_LE_UINT16(&sine[i], sine[i]);
			} else {
				for (int i = 0; i < sampleRate * time; ++i)
					WRITE_BE_UINT16(&sine[i], sine[i]);
			}
		}

		Common::SeekableReadStream *sD = new Common::MemoryReadStream((const byte *)sine, sizeof(T) * sampleRate * time, DisposeAfterUse::YES);
		Audio::SeekableAudioStream *s = Audio::makeRawStream(sD, sampleRate,
		                                                     (is16Bits ? Audio::FLAG_16BITS : 0)
		                                                     | (isUnsigned ? Audio::FLAG_UNSIGNED : 0)
		                                                     | (le ? Audio::FLAG_LITTLE_ENDIAN : 0));

		return s;
	}

	void test_read_buffer_8_bit_signed_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, &sine, false);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sizeof(int16) * sampleRate * time), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}

	void test_read_buffer_8_bit_unsigned_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<uint8>(sampleRate, time, &sine, false);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sizeof(int16) * sampleRate * time), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}

	void test_read_buffer_16_bit_signed_be_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<int16>(sampleRate, time, &sine, false);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sizeof(int16) * sampleRate * time), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}

	void test_read_buffer_16_bit_unsigned_be_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<uint16>(sampleRate, time, &sine, false);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sizeof(int16) * sampleRate * time), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}

	void test_read_buffer_16_bit_signed_le_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<int16>(sampleRate, time, &sine, true);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sizeof(int16) * sampleRate * time), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}

	void test_read_buffer_16_bit_unsigned_le_mono() {
		const int sampleRate = 11025;
		const int time = 2;

		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<uint16>(sampleRate, time, &sine, true);

		int16 *buffer = new int16[sampleRate * time];
		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sizeof(int16) * sampleRate * time), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}

	void test_partial_read() {
		const int sampleRate = 11025;
		const int time = 4;

		int16 *sine;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, &sine, false);
		int16 *buffer = new int16[sampleRate * time];

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate), sampleRate);
		TS_ASSERT_EQUALS(memcmp(sine, buffer, sampleRate), 0);
		TS_ASSERT_EQUALS(s->endOfData(), false);

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * 2), sampleRate * 2);
		TS_ASSERT_EQUALS(memcmp(sine + sampleRate, buffer, sampleRate * 2), 0);
		TS_ASSERT_EQUALS(s->endOfData(), false);

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate), sampleRate);
		TS_ASSERT_EQUALS(memcmp(sine + sampleRate * 3, buffer, sampleRate), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] sine;
		delete[] buffer;
		delete s;
	}

	void test_read_after_end() {
		const int sampleRate = 11025;
		const int time = 1;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, 0, false);
		int16 *buffer = new int16[sampleRate * time];

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), 0);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}

	void test_rewind() {
		const int sampleRate = 11025;
		const int time = 2;
		Audio::SeekableAudioStream *s = createSineStream<int8>(sampleRate, time, 0, false);
		int16 *buffer = new int16[sampleRate * time];

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		s->rewind();
		TS_ASSERT_EQUALS(s->endOfData(), false);

		TS_ASSERT_EQUALS(s->readBuffer(buffer, sampleRate * time), sampleRate * time);
		TS_ASSERT_EQUALS(s->endOfData(), true);

		delete[] buffer;
		delete s;
	}

	void test_length() {
		const int sampleRate = 11025;
		const int time = 4;

		Audio::SeekableAudioStream *s = 0;

		s = createSineStream<int8>(sampleRate, time, 0, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;

		s = createSineStream<uint16>(sampleRate, time, 0, false);
		TS_ASSERT_EQUALS(s->getLength().totalNumberOfFrames(), sampleRate * time);
		delete s;
	}
};
