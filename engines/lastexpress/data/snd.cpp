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
 * $URL$
 * $Id$
 *
 */

// Based on the Xentax Wiki documentation:
// http://wiki.xentax.com/index.php/The_Last_Express_SND

#include "lastexpress/data/snd.h"

#include "lastexpress/debug.h"

#include "sound/decoders/adpcm.h"
#include "sound/audiostream.h"
#include "common/memstream.h"

namespace LastExpress {

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
	return Audio::makeADPCMStream(in, DisposeAfterUse::YES, size, Audio::kADPCMMSImaLastExpress, 44100, 1, _blockSize);
}

void SimpleSound::play(Audio::AudioStream *as) {
	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, as);
}

//////////////////////////////////////////////////////////////////////////
// StreamedSound
//////////////////////////////////////////////////////////////////////////
StreamedSound::StreamedSound() {}
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

	return true;
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
		error("AppendableSound::queueBuffer - internal error: the audio stream is invalid!");

	// Setup the ADPCM decoder
	uint32 sizeIn = (uint32)bufferIn->size();
	Audio::AudioStream *adpcm = makeDecoder(bufferIn, sizeIn);

	// Queue the stream
	_as->queueAudioStream(adpcm);
}

void AppendableSound::finish() {
	if (!_as)
		error("AppendableSound::queueBuffer - internal error: the audio stream is invalid!");

	if (!_finished)
		_as->finish();

	_finished = true;
}

} // End of namespace LastExpress
