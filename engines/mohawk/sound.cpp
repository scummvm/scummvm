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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"

#include "mohawk/mohawk.h"
#include "mohawk/sound.h"
#include "mohawk/resource.h"

namespace Mohawk {

Audio::RewindableAudioStream *makeMohawkWaveStream(Common::SeekableReadStream *stream, CueList *cueList) {
	uint32 tag = 0;
	ADPCMStatus adpcmStatus;
	DataChunk dataChunk;
	uint32 dataSize = 0;

	memset(&dataChunk, 0, sizeof(DataChunk));

	if (stream->readUint32BE() != ID_MHWK) // MHWK tag again
		error ("Could not find tag 'MHWK'");

	stream->readUint32BE(); // Skip size

	if (stream->readUint32BE() != ID_WAVE)
		error ("Could not find tag 'WAVE'");

	while (!dataChunk.audioData) {
		tag = stream->readUint32BE();

		switch (tag) {
			case ID_ADPC:
				debug(2, "Found Tag ADPC");
				// ADPCM Sound Only
				//
				// This is useful for seeking in the stream, and is actually quite brilliant
				// considering some of the other things Broderbund did with the engine.
				// Only Riven and CSTime are known to use ADPCM audio and only CSTime
				// actually requires this for seeking. On the other hand, it may be interesting
				// to look at that one Riven sample that uses the cue points.
				//
				// Basically, the sample frame from the cue list is looked up here and then
				// sets the starting sample and step index at the point specified. Quite
				// an elegant/efficient system, really.

				adpcmStatus.size = stream->readUint32BE();
				adpcmStatus.itemCount = stream->readUint16BE();
				adpcmStatus.channels = stream->readUint16BE();
				adpcmStatus.statusItems = new ADPCMStatus::StatusItem[adpcmStatus.itemCount];

				assert(adpcmStatus.channels <= 2);

				for (uint16 i = 0; i < adpcmStatus.itemCount; i++) {
					adpcmStatus.statusItems[i].sampleFrame = stream->readUint32BE();

					for (uint16 j = 0; j < adpcmStatus.channels; j++) {
						adpcmStatus.statusItems[i].channelStatus[j].last = stream->readSint16BE();
						adpcmStatus.statusItems[i].channelStatus[j].stepIndex = stream->readUint16BE();
					}
				}

				// TODO: Actually use this chunk. For now, just delete the status items...
				delete[] adpcmStatus.statusItems;
				break;
			case ID_CUE:
				debug(2, "Found Tag Cue#");
				// Cues are used for animation sync. There are a couple in Myst and
				// Riven but are not used there at all.

				if (!cueList) {
					uint32 size = stream->readUint32BE();
					stream->skip(size);
					break;
				}

				cueList->size = stream->readUint32BE();
				cueList->pointCount = stream->readUint16BE();

				if (cueList->pointCount == 0)
					debug(2, "Cue# chunk found with no points!");
				else
					debug(2, "Cue# chunk found with %d point(s)!", cueList->pointCount);

				cueList->points.resize(cueList->pointCount);
				for (uint16 i = 0; i < cueList->pointCount; i++) {
					cueList->points[i].sampleFrame = stream->readUint32BE();

					byte nameLength = stream->readByte();
					cueList->points[i].name.clear();
					for (byte j = 0; j < nameLength; j++)
						cueList->points[i].name += stream->readByte();

					// Realign to an even boundary
					if (!(nameLength & 1))
						stream->readByte();

					debug (3, "Cue# chunk point %d (frame %d): %s", i, cueList->points[i].sampleFrame, cueList->points[i].name.c_str());
				}
				break;
			case ID_DATA:
				debug(2, "Found Tag DATA");
				// We subtract 20 from the actual chunk size, which is the total size
				// of the chunk's header
				dataSize = stream->readUint32BE() - 20;
				dataChunk.sampleRate = stream->readUint16BE();
				dataChunk.sampleCount = stream->readUint32BE();
				dataChunk.bitsPerSample = stream->readByte();
				dataChunk.channels = stream->readByte();
				dataChunk.encoding = stream->readUint16BE();
				dataChunk.loopCount = stream->readUint16BE();
				dataChunk.loopStart = stream->readUint32BE();
				dataChunk.loopEnd = stream->readUint32BE();

				// NOTE: We currently ignore all of the loop parameters here. Myst uses the
				// loopCount variable but the loopStart and loopEnd are always 0 and the size of
				// the sample. Myst ME doesn't use the Mohawk Sound format and just standard WAVE
				// files and therefore does not contain any of this metadata and we have to specify
				// whether or not to loop elsewhere.

				dataChunk.audioData = stream->readStream(dataSize);
				break;
			default:
				error ("Unknown tag found in 'tWAV' chunk -- '%s'", tag2str(tag));
		}
	}

	// makeMohawkWaveStream always takes control of the original stream
	delete stream;

	// The sound in Myst uses raw unsigned 8-bit data
	// The sound in the CD version of Riven is encoded in Intel DVI ADPCM
	// The sound in the DVD version of Riven is encoded in MPEG-2 Layer II or Intel DVI ADPCM
	if (dataChunk.encoding == kCodecRaw) {
		byte flags = 0;

		if (dataChunk.channels == 2)
			flags |= Audio::FLAG_STEREO;

		if (dataChunk.bitsPerSample == 16)
			flags |= Audio::FLAG_16BITS;
		else
			flags |= Audio::FLAG_UNSIGNED;

		return Audio::makeRawStream(dataChunk.audioData, dataChunk.sampleRate, flags);
	} else if (dataChunk.encoding == kCodecADPCM) {
		uint32 blockAlign = dataChunk.channels * dataChunk.bitsPerSample / 8;
		return Audio::makeADPCMStream(dataChunk.audioData, DisposeAfterUse::YES, dataSize, Audio::kADPCMDVI, dataChunk.sampleRate, dataChunk.channels, blockAlign);
	} else if (dataChunk.encoding == kCodecMPEG2) {
#ifdef USE_MAD
		return Audio::makeMP3Stream(dataChunk.audioData, DisposeAfterUse::YES);
#else
		warning ("MAD library not included - unable to play MP2 audio");
#endif
	} else {
		error ("Unknown Mohawk WAVE encoding %d", dataChunk.encoding);
	}

	return nullptr;
}

Sound::Sound(MohawkEngine* vm) :
		_vm(vm) {
}

Sound::~Sound() {
	stopSound();
}

Audio::RewindableAudioStream *Sound::makeAudioStream(uint16 id, CueList *cueList) {
	Audio::RewindableAudioStream *audStream = nullptr;

	switch (_vm->getGameType()) {
	case GType_LIVINGBOOKSV1:
		audStream = makeLivingBooksWaveStream_v1(_vm->getResource(ID_WAV, id));
		break;
	case GType_LIVINGBOOKSV2:
		if (_vm->getPlatform() == Common::kPlatformMacintosh) {
			audStream = makeLivingBooksWaveStream_v1(_vm->getResource(ID_WAV, id));
			break;
		}
		// fall through
	default:
		audStream = makeMohawkWaveStream(_vm->getResource(ID_TWAV, id), cueList);
	}

	return audStream;
}

Audio::SoundHandle *Sound::playSound(uint16 id, byte volume, bool loop, CueList *cueList) {
	debug (0, "Playing sound %d", id);

	Audio::RewindableAudioStream *rewindStream = makeAudioStream(id, cueList);

	if (rewindStream) {
		SndHandle *handle = getHandle();
		handle->type = kUsedHandle;
		handle->id = id;
		handle->samplesPerSecond = rewindStream->getRate();

		// Set the stream to loop here if it's requested
		Audio::AudioStream *audStream = rewindStream;
		if (loop)
			audStream = Audio::makeLoopingAudioStream(rewindStream, 0);

		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle->handle, audStream, -1, volume);
		return &handle->handle;
	}

	return nullptr;
}

Audio::RewindableAudioStream *Sound::makeLivingBooksWaveStream_v1(Common::SeekableReadStream *stream) {
	uint16 header = stream->readUint16BE();
	uint16 rate = 0;
	uint32 size = 0;

	if (header == 'Wv') { // Big Endian
		rate = stream->readUint16BE();
		stream->skip(10); // Unknown
		size = stream->readUint32BE();
	} else if (header == 'vW') { // Little Endian
		stream->readUint16LE(); // Unknown
		rate = stream->readUint16LE();
		stream->skip(8); // Unknown
		size = stream->readUint32LE();
	} else
		error("Could not find Old Mohawk Sound header");

	Common::SeekableReadStream *dataStream = stream->readStream(size);
	delete stream;

	return Audio::makeRawStream(dataStream, rate, Audio::FLAG_UNSIGNED);
}

SndHandle *Sound::getHandle() {
	for (uint32 i = 0; i < _handles.size(); i++) {
		if (_handles[i].type == kFreeHandle)
			return &_handles[i];

		if (!_vm->_mixer->isSoundHandleActive(_handles[i].handle)) {
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
			return &_handles[i];
		}
	}

	// Let's add a new sound handle!
	SndHandle handle;
	handle.handle = Audio::SoundHandle();
	handle.type = kFreeHandle;
	handle.id = 0;
	_handles.push_back(handle);

	return &_handles[_handles.size() - 1];
}

void Sound::stopSound() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle) {
			_vm->_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
		}
}

void Sound::stopSound(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id) {
			_vm->_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
		}
}

bool Sound::isPlaying(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id)
			return _vm->_mixer->isSoundHandleActive(_handles[i].handle);

	return false;
}

bool Sound::isPlaying() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle)
			if (_vm->_mixer->isSoundHandleActive(_handles[i].handle))
				return true;

	return false;
}

uint Sound::getNumSamplesPlayed(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id) {
			return (_vm->_mixer->getSoundElapsedTime(_handles[i].handle) * _handles[i].samplesPerSecond) / 1000;
		}

	return 0;
}

} // End of namespace Mohawk
