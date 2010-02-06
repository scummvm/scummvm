#include <cxxtest/TestSuite.h>

#include "sound/audiostream.h"

#include "helper.h"

class AudioStreamTestSuite : public CxxTest::TestSuite
{
public:
	void test_convertTimeToStreamPos() {
		const Audio::Timestamp a = Audio::convertTimeToStreamPos(Audio::Timestamp(500, 1000), 11025, true);
		// The last bit has to be 0 in any case for a stereo stream.
		TS_ASSERT_EQUALS(a.totalNumberOfFrames() & 1, 0);

		// TODO: This test is rather hacky... actually converTimeToStreamPos might also return 11026
		// instead of 11024 and it would still be a valid sample position for ~500ms.
		TS_ASSERT_EQUALS(a.totalNumberOfFrames(), 11024);

		const Audio::Timestamp b = Audio::convertTimeToStreamPos(Audio::Timestamp(500, 1000), 11025, false);
		TS_ASSERT_EQUALS(b.totalNumberOfFrames(), 500 * 11025 / 1000);

		// Test Audio CD positioning

		// for 44kHz and stereo
		const Audio::Timestamp c = Audio::convertTimeToStreamPos(Audio::Timestamp(0, 50, 75), 44100, true);
		TS_ASSERT_EQUALS(c.totalNumberOfFrames(), 50 * 44100 * 2 / 75);

		// for 11kHz and mono
		const Audio::Timestamp d = Audio::convertTimeToStreamPos(Audio::Timestamp(0, 50, 75), 11025, false);
		TS_ASSERT_EQUALS(d.totalNumberOfFrames(), 50 * 11025 / 75);

		// Some misc test
		const Audio::Timestamp e = Audio::convertTimeToStreamPos(Audio::Timestamp(1, 1, 4), 11025, false);
		TS_ASSERT_EQUALS(e.totalNumberOfFrames(), 5 * 11025 / 4);
	}

private:
	void testLoopingAudioStreamFixedIter(const int sampleRate, const bool isStereo) {
		const int secondLength = sampleRate * (isStereo ? 2 : 1);

		int16 *sine = 0;
		Audio::SeekableAudioStream *s = createSineStream<int16>(sampleRate, 1, &sine, false, isStereo);
		Audio::LoopingAudioStream *loop = new Audio::LoopingAudioStream(s, 7);

		int16 *buffer = new int16[secondLength * 3];

		// Check parameters
		TS_ASSERT_EQUALS(loop->isStereo(), isStereo);
		TS_ASSERT_EQUALS(loop->getRate(), sampleRate);
		TS_ASSERT_EQUALS(loop->endOfData(), false);
		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)0);

		// Read one second
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength), secondLength);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, secondLength * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)1);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// Read two seconds
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength * 2), secondLength * 2);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, secondLength * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(memcmp(buffer + secondLength, sine, secondLength * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)3);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// Read three seconds
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength * 3), secondLength * 3);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, secondLength * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(memcmp(buffer + secondLength, sine, secondLength * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(memcmp(buffer + secondLength * 2, sine, secondLength * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)6);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// Read the last second
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength), secondLength);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, secondLength * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)7);
		TS_ASSERT_EQUALS(loop->endOfData(), true);

		// Try to read beyond the end of the stream
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength), 0);
		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)7);
		TS_ASSERT_EQUALS(loop->endOfData(), true);

		delete[] buffer;
		delete loop;
		delete[] sine;
	}

public:
	void test_looping_audio_stream_mono_fixed_iter() {
		testLoopingAudioStreamFixedIter(22050, false);
	}

	void test_looping_audio_stream_stereo_fixed_iter() {
		testLoopingAudioStreamFixedIter(22050, true);
	}
};

