#include <cxxtest/TestSuite.h>

#include "sound/audiostream.h"

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
	}
};

