/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cxxtest/TestSuite.h>

#include "audio/audiostream.h"
#include "audio/decoders/mp3.h"
#include "common/array.h"
#include "common/memstream.h"
#include "test/system/null_osystem.h"

#if defined(USE_MAD) && !defined(__PSP__) && NULL_OSYSTEM_IS_AVAILABLE

class FailingMP3ReadStream : public Common::SeekableReadStream {
public:
	FailingMP3ReadStream(const byte *data, uint size, uint limit, bool reportError) :
		_stream(data, size), _limit(limit), _reportError(reportError), _error(false) {}

	uint32 read(void *data, uint32 size) override {
		if (_stream.pos() >= _limit) {
			_error = _reportError;
			return 0;
		}
		return _stream.read(data, MIN<uint32>(size, _limit - _stream.pos()));
	}
	bool eos() const override { return false; }
	bool err() const override { return _error; }
	void clearErr() override { _error = false; }
	int64 pos() const override { return _stream.pos(); }
	int64 size() const override { return _stream.size(); }
	bool seek(int64 offset, int whence = SEEK_SET) override {
		_error = false;
		return _stream.seek(offset, whence);
	}

private:
	Common::MemoryReadStream _stream;
	uint _limit;
	bool _reportError;
	bool _error;
};

class MP3StreamTestSuite : public CxxTest::TestSuite {
public:
	void setUp() {
#if NULL_OSYSTEM_IS_AVAILABLE
		Common::install_null_g_system();
#endif
	}

	void tearDown() {
#if NULL_OSYSTEM_IS_AVAILABLE
		Common::uninstall_null_g_system();
#endif
	}

	void testLinearFinalFrameAndSeek() {
		Common::Array<byte> data = silentFrames(128);
		Audio::SeekableAudioStream *stream = Audio::makeMP3Stream(
			new Common::MemoryReadStream(&data[0], data.size()), DisposeAfterUse::YES);
		TS_ASSERT(stream);
		if (!stream)
			return;
		TS_ASSERT_EQUALS(readSamples(*stream), 128U * 1152);
		TS_ASSERT(stream->seek(Audio::Timestamp(0, 44100)));
		TS_ASSERT_EQUALS(readSamples(*stream), 128U * 1152);
		delete stream;
	}

	void testLayerIIIFinalFrame() {
		Common::Array<byte> data = silentFrames(32);
		for (uint i = 0; i < 32; ++i) {
			data[i * 417 + 1] = 0xfb; // Layer III with zero side information.
			data[i * 417 + 2] = 0x90; // Layer III index 9 is 128 kbit/s.
		}
		Audio::SeekableAudioStream *linear = Audio::makeMP3Stream(
			new Common::MemoryReadStream(&data[0], data.size()), DisposeAfterUse::YES);
		TS_ASSERT(linear);
		if (linear) {
			TS_ASSERT_EQUALS(readSamples(*linear), 32U * 1152);
			delete linear;
		}
		Audio::PacketizedAudioStream *packets = Audio::makePacketizedMP3Stream(1, 44100);
		packets->queuePacket(new Common::MemoryReadStream(&data[0], data.size()));
		packets->finish();
		TS_ASSERT_EQUALS(readSamples(*packets), 32U * 1152);
		TS_ASSERT(packets->endOfStream());
		delete packets;
	}

	void testTruncatedFinalFrame() {
		Common::Array<byte> data = silentFrames(2);
		Audio::SeekableAudioStream *stream = Audio::makeMP3Stream(
			new Common::MemoryReadStream(&data[0], data.size() - 100), DisposeAfterUse::YES);
		TS_ASSERT(stream);
		if (stream) {
			TS_ASSERT_EQUALS(readSamples(*stream), 1152U);
			delete stream;
		}
	}

	void testPacketizedDrainAtDifferentPacketSizes() {
		const uint sizes[] = { 512, 2048, 8192, 32768 };
		Common::Array<byte> data = silentFrames(32);
		for (uint i = 0; i < ARRAYSIZE(sizes); ++i) {
			Audio::PacketizedAudioStream *stream = Audio::makePacketizedMP3Stream(1, 44100);
			for (uint pos = 0; pos < data.size(); pos += sizes[i])
				stream->queuePacket(new Common::MemoryReadStream(&data[pos], MIN(sizes[i], data.size() - pos)));
			stream->finish();
			TS_ASSERT_EQUALS(readSamples(*stream), 32U * 1152);
			TS_ASSERT(stream->endOfStream());
			delete stream;
		}
	}

	void testFinishAfterUnderrun() {
		Common::Array<byte> data = silentFrames(32);
		Audio::PacketizedAudioStream *stream = Audio::makePacketizedMP3Stream(1, 44100);
		stream->queuePacket(new Common::MemoryReadStream(&data[0], data.size()));
		uint samples = readSamples(*stream);
		TS_ASSERT(!stream->endOfStream());
		stream->finish();
		samples += readSamples(*stream);
		TS_ASSERT_EQUALS(samples, 32U * 1152);
		TS_ASSERT(stream->endOfStream());
		delete stream;
	}

	void testEmptyFinishedStream() {
		Audio::PacketizedAudioStream *stream = Audio::makePacketizedMP3Stream(1, 44100);
		stream->finish();
		TS_ASSERT(stream->endOfData());
		TS_ASSERT(stream->endOfStream());
		int16 sample = 123;
		TS_ASSERT_EQUALS(stream->readBuffer(&sample, 1), 0);
		delete stream;
	}

	void testZeroReadWithoutEOF() {
		Common::Array<byte> data = silentFrames(256);
		for (uint error = 0; error < 2; ++error) {
			Audio::SeekableAudioStream *stream = Audio::makeMP3Stream(
				new FailingMP3ReadStream(&data[0], data.size(), 144 * 417 + 200, error != 0), DisposeAfterUse::YES);
			TS_ASSERT(stream);
			if (stream) {
				TS_ASSERT_EQUALS(readSamples(*stream), 144U * 1152);
				TS_ASSERT(stream->endOfData());
				delete stream;
			}
		}
	}

	void testStereoPacketizedAt48kHz() {
		Common::Array<byte> data = silentFrames(32, true);
		Audio::PacketizedAudioStream *stream = Audio::makePacketizedMP3Stream(2, 48000);
		for (uint pos = 0; pos < data.size(); pos += 2048)
			stream->queuePacket(new Common::MemoryReadStream(&data[pos], MIN<uint>(2048, data.size() - pos)));
		stream->finish();
		TS_ASSERT_EQUALS(readSamples(*stream), 32U * 1152 * 2);
		TS_ASSERT(stream->endOfStream());
		delete stream;
	}

	void testNewPacketAfterUnderrun() {
		Common::Array<byte> data = silentFrames(32);
		Audio::PacketizedAudioStream *stream = Audio::makePacketizedMP3Stream(1, 44100);
		const uint split = 16 * 417 + 3;
		stream->queuePacket(new Common::MemoryReadStream(&data[0], split));
		uint samples = readSamples(*stream);
		TS_ASSERT(!stream->endOfStream());
		stream->queuePacket(new Common::MemoryReadStream(&data[split], data.size() - split));
		TS_ASSERT(!stream->endOfData());
		stream->finish();
		samples += readSamples(*stream);
		TS_ASSERT_EQUALS(samples, 32U * 1152);
		TS_ASSERT(stream->endOfStream());
		delete stream;
	}

private:
	static Common::Array<byte> silentFrames(uint frames, bool stereo48kHz = false) {
		// MPEG-1 Layer II, 128 kbit/s, 44.1 kHz, mono. Zero bit allocations
		// produce 1152 silent samples per 417-byte frame.
		Common::Array<byte> data;
		const uint bytesPerFrame = stereo48kHz ? 384 : 417;
		data.resize(frames * bytesPerFrame);
		memset(&data[0], 0, data.size());
		for (uint i = 0; i < frames; ++i) {
			data[i * bytesPerFrame] = 0xff;
			data[i * bytesPerFrame + 1] = 0xfd;
			data[i * bytesPerFrame + 2] = stereo48kHz ? 0x84 : 0x80;
			data[i * bytesPerFrame + 3] = stereo48kHz ? 0x00 : 0xc0;
		}
		return data;
	}

	uint readSamples(Audio::AudioStream &stream) {
		uint total = 0;
		int16 buffer[442];
		const uint count = stream.isStereo() ? 442 : 441;
		uint calls = 0;
		while (!stream.endOfData() && calls++ < 10000) {
			const int samples = stream.readBuffer(buffer, count);
			TS_ASSERT(samples >= 0 && samples <= (int)count);
			total += samples;
			for (int i = 0; i < samples; ++i)
				TS_ASSERT_EQUALS(buffer[i], 0);
		}
		TS_ASSERT(stream.endOfData());
		return total;
	}
};

#endif // libmad tests with a test backend
