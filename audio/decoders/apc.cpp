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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/inttypes.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/decoders/adpcm_intern.h"
#include "audio/decoders/apc.h"
#include "audio/decoders/raw.h"

namespace Audio {

class APCStreamImpl : public APCStream {
public:
	// These parameters are completely optional and only used in HNM videos to make sure data is consistent
	APCStreamImpl(uint32 sampleRate = uint(-1), byte stereo = byte(-1));

	bool init(Common::SeekableReadStream &header) override;

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) override { return _audStream->readBuffer(buffer, numSamples); }
	bool isStereo() const override { return _audStream->isStereo(); }
	int getRate() const override { return _audStream->getRate(); }
	bool endOfData() const override { return _audStream->endOfData(); }
	bool endOfStream() const override { return _audStream->endOfStream(); }

	// PacketizedAudioStream API
	void queuePacket(Common::SeekableReadStream *data) override;
	void finish() override { _audStream->finish(); }

private:
	struct Status {
		int32 last;
		uint32 stepIndex;
		int16 step;
	};

	FORCEINLINE static int16 decodeIMA(byte code, Status *status);

	Common::ScopedPtr<QueuingAudioStream> _audStream;
	byte _stereo;
	uint32 _sampleRate;

	Status _status[2];
};

APCStreamImpl::APCStreamImpl(uint32 sampleRate, byte stereo) :
	_sampleRate(sampleRate), _stereo(stereo) {
	if (_sampleRate != uint32(-1) && _stereo != byte(-1)) {
		_audStream.reset(makeQueuingAudioStream(_sampleRate, _stereo));
	}
}

bool APCStreamImpl::init(Common::SeekableReadStream &header) {
	// Header size
	if (header.size() < 32) {
		return false;
	}

	// Read magic and version at once
	byte magicVersion[12];
	if (header.read(magicVersion, sizeof(magicVersion)) != sizeof(magicVersion)) {
		return false;
	}
	if (memcmp(magicVersion, "CRYO_APC1.20", sizeof(magicVersion))) {
		return false;
	}

	//uint32 num_samples = header.readUint32LE();
	header.skip(4);

	uint32 samplerate = header.readUint32LE();
	if (_sampleRate != uint32(-1) && _sampleRate != samplerate) {
		error("Samplerate mismatch");
		return false;
	}
	_sampleRate = samplerate;

	uint32 leftSample = header.readSint32LE();
	uint32 rightSample = header.readSint32LE();

	uint32 audioFlags = header.readUint32LE();
	byte stereo = (audioFlags & 1);
	if (_stereo != byte(-1) && _stereo != stereo) {
		error("Channels mismatch");
		return false;

	}
	_stereo = stereo;

	_status[0].last = leftSample;
	_status[1].last = rightSample;
	_status[0].stepIndex = _status[1].stepIndex = 0;
	_status[0].step = _status[1].step = Ima_ADPCMStream::_imaTable[0];

	if (!_audStream) {
		_audStream.reset(makeQueuingAudioStream(_sampleRate, _stereo));
	}

	return true;
}

void APCStreamImpl::queuePacket(Common::SeekableReadStream *data) {
	Common::ScopedPtr<Common::SeekableReadStream> packet(data);

	uint32 size = packet->size() - packet->pos();
	if (size == 0) {
		return;
	}

	// From 4-bits samples to 16-bits
	int16 *outputBuffer = (int16 *)malloc(size * 4);
	int16 *outputPtr = outputBuffer;

	int channelOffset = (_stereo ? 1 : 0);

	for (uint32 counter = size; counter > 0; counter--) {
		byte word = packet->readByte();
		*(outputPtr++) = decodeIMA((word >> 4) & 0x0f, _status);
		*(outputPtr++) = decodeIMA((word >> 0) & 0x0f, _status + channelOffset);
	}

	byte flags = FLAG_16BITS;
	if (_stereo) {
		flags |= FLAG_STEREO;
	}
#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
	_audStream->queueBuffer((byte *)outputBuffer, size * 4, DisposeAfterUse::YES, flags);
}

int16 APCStreamImpl::decodeIMA(byte code, Status *status) {
	int32 E = (2 * (code & 0x7) + 1) * status->step / 8;
	int32 diff = (code & 0x08) ? -E : E;
	// In Cryo APC data is only truncated and not CLIPed as expected
	int16 samp = (status->last + diff);

	int32 index = status->stepIndex + Ima_ADPCMStream::_stepAdjustTable[code];
	index = CLIP<int32>(index, 0, ARRAYSIZE(Ima_ADPCMStream::_imaTable) - 1);

	status->last = samp;
	status->stepIndex = index;
	status->step = Ima_ADPCMStream::_imaTable[index];

	return samp;
}

PacketizedAudioStream *makeAPCStream(Common::SeekableReadStream &header) {
	Common::ScopedPtr<APCStream> stream(new APCStreamImpl());
	if (!stream->init(header)) {
		return nullptr;
	}

	return stream.release();
}

APCStream *makeAPCStream(uint sampleRate, bool stereo) {
	assert((sampleRate % 11025) == 0);
	return new APCStreamImpl(sampleRate, stereo ? 1 : 0);
}

} // End of namespace Audio

