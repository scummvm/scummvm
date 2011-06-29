/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based on the Xentax Wiki documentation:
// http://wiki.xentax.com/index.php/The_Last_Express_SND

#include "lastexpress/data/snd.h"

#include "lastexpress/debug.h"

#include "audio/decoders/adpcm_intern.h"
#include "audio/audiostream.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace LastExpress {

// Last Express ADPCM is similar to MS IMA mono, but inverts its nibbles
// and does not have the 4 byte per channel requirement

class LastExpress_ADPCMStream : public Audio::Ima_ADPCMStream {
public:
	LastExpress_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, uint32 blockSize) :
			Audio::Ima_ADPCMStream(stream, disposeAfterUse, size, 44100, 1, blockSize) {}

	int readBuffer(int16 *buffer, const int numSamples) {
		int samples = 0;

		assert(numSamples % 2 == 0);

		while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
			if (_blockPos[0] == _blockAlign) {
				// read block header
				_status.ima_ch[0].last = _stream->readSint16LE();
				_status.ima_ch[0].stepIndex = _stream->readSint16LE();
				_blockPos[0] = 4;
			}

			for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
				byte data = _stream->readByte();
				_blockPos[0]++;
				buffer[samples] = decodeIMA((data >> 4) & 0x0f);
				buffer[samples + 1] = decodeIMA(data & 0x0f);
			}
		}

		return samples;
	}
};

//////////////////////////////////////////////////////////////////////////
// Sound
//////////////////////////////////////////////////////////////////////////
SimpleSound::SimpleSound() : _size(0), _blocks(0), _blockSize(0) {}

SimpleSound::~SimpleSound() {
	stop();
}

// Stop the sound
void SimpleSound::stop() const {
	g_system->getMixer()->stopHandle(_handle);
}

void SimpleSound::loadHeader(Common::SeekableReadStream *in) {
	_size = in->readUint32LE();
	_blocks = in->readUint16LE();
	debugC(5, kLastExpressDebugSound, "    sound header data: size=\"%d\", %d blocks", _size, _blocks);

	assert (_size % _blocks == 0);
	_blockSize = _size / _blocks;
}

Audio::AudioStream *SimpleSound::makeDecoder(Common::SeekableReadStream *in, uint32 size) const {
	return new LastExpress_ADPCMStream(in, DisposeAfterUse::YES, size, _blockSize);
}

void SimpleSound::play(Audio::AudioStream *as) {
	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, as);
}

//////////////////////////////////////////////////////////////////////////
// StreamedSound
//////////////////////////////////////////////////////////////////////////
StreamedSound::StreamedSound() : _loaded(false) {}

StreamedSound::~StreamedSound() {}

bool StreamedSound::load(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	g_system->getMixer()->stopHandle(_handle);

	loadHeader(stream);

	// Start decoding the input stream
	Audio::AudioStream *as = makeDecoder(stream, _size);

	// Start playing the decoded audio stream
	play(as);

	_loaded = true;

	return true;
}

bool StreamedSound::isFinished() {
	if (!_loaded)
		return false;

	return !g_system->getMixer()->isSoundHandleActive(_handle);
}

//////////////////////////////////////////////////////////////////////////
// StreamedSound
//////////////////////////////////////////////////////////////////////////
AppendableSound::AppendableSound() : SimpleSound() {
	// Create an audio stream where the decoded chunks will be appended
	_as = Audio::makeQueuingAudioStream(44100, false);
	_finished = false;

	// Start playing the decoded audio stream
	play(_as);

	// Initialize the block size
	// TODO: get it as an argument?
	_blockSize = 739;
}

AppendableSound::~AppendableSound() {
	finish();

	_as = NULL;
}

void AppendableSound::queueBuffer(const byte *data, uint32 size) {
	Common::MemoryReadStream *buffer = new Common::MemoryReadStream(data, size);
	queueBuffer(buffer);
}

void AppendableSound::queueBuffer(Common::SeekableReadStream *bufferIn) {
	if (!_as)
		error("[AppendableSound::queueBuffer] Audio stream is invalid");

	// Setup the ADPCM decoder
	uint32 sizeIn = (uint32)bufferIn->size();
	Audio::AudioStream *adpcm = makeDecoder(bufferIn, sizeIn);

	// Queue the stream
	_as->queueAudioStream(adpcm);
}

void AppendableSound::finish() {
	if (!_as)
		error("[AppendableSound::finish] Audio stream is invalid");

	if (!_finished)
		_as->finish();

	_finished = true;
}

bool AppendableSound::isFinished() {
	return _as->endOfStream();
}

} // End of namespace LastExpress
