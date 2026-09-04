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

#include "common/debug.h"
#include "common/config-manager.h"
#include "common/endian.h"

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

/**
 * Extract the Standard MIDI File data from a Mohawk tMID resource.
 *
 * Broderbund stores SMF chunks inside an MHWK/MIDI container. The container
 * may also hold driver patch-cache chunks such as Prg# and Key#, before or
 * after MTrk chunks. The original player enumerates those chunks separately;
 * ScummVM's SMF parser must receive only the standard MThd and MTrk chunks.
 *
 * The original Zoombini Windows player accepts both format 0 and format 1
 * SMF resources and schedules every MTrk independently. Do not flatten a
 * type-1 resource as a compatibility workaround: MIDIMPC and MIDIMAC are
 * separately authored device profiles, not a parser-format limitation.
 */
bool MidiPlayer::extractMohawkMidi(Common::SeekableReadStream *stream, Common::Array<byte> &standardMidi) {
	const uint32 resourceSize = stream->size();
	if (resourceSize < 26) {
		warning("Mohawk MIDI resource is too small (%u bytes)", resourceSize);
		return false;
	}

	Common::Array<byte> resourceData;
	resourceData.resize(resourceSize);
	if (stream->read(resourceData.data(), resourceSize) != resourceSize) {
		warning("Could not read complete Mohawk MIDI resource");
		return false;
	}

	const byte *data = resourceData.data();
	if (READ_BE_UINT32(data) != ID_MHWK || READ_BE_UINT32(data + 8) != ID_MIDI) {
		warning("Invalid Mohawk MIDI resource header");
		return false;
	}

	const uint32 containerSize = READ_BE_UINT32(data + 4);
	if (containerSize > resourceSize - 8) {
		warning("Mohawk MIDI container length %u exceeds resource length %u", containerSize, resourceSize);
		return false;
	}
	const uint32 containerEnd = (containerSize + 9) & ~1U;

	const uint32 smfHeaderOffset = 12;
	if (READ_BE_UINT32(data + smfHeaderOffset) != MKTAG('M', 'T', 'h', 'd')) {
		warning("Could not find MThd in Mohawk MIDI resource");
		return false;
	}

	const uint32 smfHeaderSize = READ_BE_UINT32(data + smfHeaderOffset + 4);
	if (smfHeaderSize != 6 || smfHeaderSize > containerEnd - smfHeaderOffset - 8) {
		warning("Invalid MThd length %u in Mohawk MIDI resource", smfHeaderSize);
		return false;
	}

	const uint16 midiType = READ_BE_UINT16(data + smfHeaderOffset + 8);
	const uint16 trackCount = READ_BE_UINT16(data + smfHeaderOffset + 10);
	if (midiType > 1 || trackCount == 0) {
		warning("Unsupported Mohawk MIDI type %u with %u tracks", midiType, trackCount);
		return false;
	}

	uint32 chunkOffset = smfHeaderOffset + 8 + smfHeaderSize;
	standardMidi.resize(8 + smfHeaderSize);
	memcpy(standardMidi.data(), data + smfHeaderOffset, standardMidi.size());

	uint16 tracksRead = 0;
	while (tracksRead < trackCount) {
		if (chunkOffset > containerEnd || containerEnd - chunkOffset < 8) {
			warning("Mohawk MIDI resource ended before MTrk chunk %u", tracksRead);
			return false;
		}

		const uint32 chunkTag = READ_BE_UINT32(data + chunkOffset);
		const uint32 chunkSize = READ_BE_UINT32(data + chunkOffset + 4);
		if (chunkSize > containerEnd - chunkOffset - 8) {
			warning("Invalid Mohawk MIDI chunk length %u", chunkSize);
			return false;
		}

		const uint32 nextChunkOffset = chunkOffset + 8 + chunkSize;
		const uint32 alignedChunkOffset = (nextChunkOffset + 1) & ~1U;
		if (alignedChunkOffset < nextChunkOffset || alignedChunkOffset > containerEnd) {
			warning("Invalid Mohawk MIDI chunk alignment");
			return false;
		}

		if (chunkTag == MKTAG('M', 'T', 'r', 'k')) {
			const uint32 outputOffset = standardMidi.size();
			standardMidi.resize(outputOffset + 8 + chunkSize);
			memcpy(standardMidi.data() + outputOffset, data + chunkOffset, 8 + chunkSize);
			++tracksRead;
		}

		chunkOffset = alignedChunkOffset;
	}

	return true;
}

/**
 * Applies a heuristic to detect and address discontinuity
 * at the end of 8-bit unsigned PCM samples, which were present in the
 * original game assets. The function modifies the DataChunk and dataSize
 * directly if a fix is applied.
 *
 * @param dataChunk The DataChunk containing sample metadata.
 * @param dataSize The total size of the data.
 * @param stream The stream to look for samples.
 */
void scanAndFixAudioPops(DataChunk &dataChunk, uint32 &dataSize, Common::SeekableReadStream *stream) {

	// Some assets declare more DATA payload bytes than remain in the current stream.
	// Clamp dataSize to the bytes physically available before probing the final samples.
	if (stream->size() != -1) {
		int64 availableBytes = stream->size() - stream->pos();
		if (availableBytes >= 0 && dataSize > (uint32)availableBytes) {
			debug(1, "MOHAWK: Clamping declared dataSize %u to actual available bytes %u", dataSize, (uint32)availableBytes);
			dataSize = (uint32)availableBytes;
		}
	}

	if (dataSize < 4)
		return;

	const int PCM8_U_SILENCE = 0x80;
	const int SQUELCH = 8; // Threshold of discontinuity before removing. Lower values = increased sensitivity.
	bool is_safe = false;

	// Peek at the last 4 samples without permanently moving the stream pointer.
	uint32 current_pos = stream->pos();
	stream->seek(current_pos + dataSize - 4, SEEK_SET);
	byte s[4];
	stream->read(s, 4);
	stream->seek(current_pos, SEEK_SET); // Return to original position

	// Path 1: Check for sustained quietness. If all samples are very close to silence,
	// any minor fluctuation is inaudible and the sound is considered safe.
	bool is_stable_and_quiet = true;
	for (int i = 0; i < 4; i++) {
		if (abs(s[i] - PCM8_U_SILENCE) >= SQUELCH) {
			is_stable_and_quiet = false;
			break;
		}
	}
	if (is_stable_and_quiet) {
		is_safe = true;
	}

	// Path 2: If not stable/quiet, check for a consistent fade-out trend.
	if (!is_safe) {
		int dist_last = abs(s[3] - PCM8_U_SILENCE);
		int dist_prev = abs(s[2] - PCM8_U_SILENCE);
		int dist_ante = abs(s[1] - PCM8_U_SILENCE);

		if (dist_last < dist_prev && dist_prev < dist_ante) {
			is_safe = true;
		}
	}

	// If the ending is neither stable nor fading, apply the fix.
	if (!is_safe) {
		debug(0, "MOHAWK: Pop/click detected at sample %u. Final samples: %02x %02x %02x %02x. Truncating one sample.",
			dataChunk.sampleCount, s[0], s[1], s[2], s[3]);

		dataChunk.sampleCount--;
		dataSize--; // Also decrement the total data size to be read.
		if (dataChunk.loopCount == 0xFFFF && dataChunk.loopEnd > dataChunk.sampleCount) {
			dataChunk.loopEnd = dataChunk.sampleCount;
		}
	}
}

/**
 * Decode an MHWK/WAVE resource and optionally export a supported loop range.
 *
 * Loop metadata stays separate from the returned decoder stream so existing
 * callers retain the public rewindable-stream contract.
 * Zoombini playback always requests @p loopInfo and applies the exported
 * infinite loop.
 *
 * @param stream The resource stream, which is consumed and deleted.
 * @param cueList Optional destination for parsed Cue# entries.
 * @param loopInfo Optional destination for a validated embedded-loop range.
 * @return The decoded seekable stream, or nullptr when decoding fails.
 */
Audio::SeekableAudioStream *makeMohawkWaveStream(
		Common::SeekableReadStream *stream, CueList *cueList,
		MohawkWaveLoopInfo *loopInfo) {
	uint32 tag = 0;
	ADPCMStatus adpcmStatus;
	DataChunk dataChunk;
	uint32 dataSize = 0;

	memset(&dataChunk, 0, sizeof(DataChunk));
	if (loopInfo)
		*loopInfo = MohawkWaveLoopInfo();

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
				// The Data payload begins with a 20-byte big-endian format header.
				// The declared chunk size includes this header.
				// loopStart is inclusive and loopEnd is exclusive.
				// Both loop positions use source sample frames rather than byte offsets.
				dataSize = stream->readUint32BE() - 20;
				dataChunk.sampleRate = stream->readUint16BE();
				dataChunk.sampleCount = stream->readUint32BE();
				dataChunk.bitsPerSample = stream->readByte();
				dataChunk.channels = stream->readByte();
				dataChunk.encoding = stream->readUint16BE();
				dataChunk.loopCount = stream->readUint16BE();
				dataChunk.loopStart = stream->readUint32BE();
				dataChunk.loopEnd = stream->readUint32BE();

				// For unsigned 8-bit PCM, check for and fix a potential pop/click at the end of the sample.
				if (dataChunk.encoding == kCodecRaw && dataChunk.bitsPerSample == 8 && dataChunk.sampleCount >= 4) {
					MohawkEngine *mohawkEngine = static_cast<MohawkEngine *>(g_engine);
					const char *gameId = mohawkEngine->getGameId();
					// Myst does not have pops and Riven does not have unsigned 8-bit PCM and so is ignored.
					if (strcmp(gameId, "myst") != 0 && strcmp(gameId, "riven") != 0 && ConfMan.getBool("fix_audio_pops"))
						scanAndFixAudioPops(dataChunk, dataSize, stream);
				}

				dataChunk.audioData = stream->readStream(dataSize);
				break;
			default:
				error ("Unknown tag found in 'tWAV' chunk -- '%s'", tag2str(tag));
		}
	}

	// makeMohawkWaveStream always takes control of the original stream
	delete stream;

	// The sound in Myst, Zoombini uses raw unsigned 8-bit data
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

		// Publish only the verified raw 8-bit infinite-loop form.
		// Available Zoombini looped SND resources use this form.
		// Finite loops are not published because
		// @ref Audio::SubLoopingAudioStream ends at the exclusive loop end
		// after its final iteration instead of continuing through the tail.
		// Validate after the optional pop fix so the exclusive loop end cannot
		// exceed a truncated sample payload.
		if (loopInfo && dataChunk.bitsPerSample == 8 &&
			dataChunk.loopCount == 0xFFFF &&
			dataChunk.loopStart < dataChunk.loopEnd &&
			dataChunk.loopEnd <= dataChunk.sampleCount) {
			loopInfo->start = dataChunk.loopStart;
			loopInfo->end = dataChunk.loopEnd;
		}

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

Sound::Sound(MohawkEngine *vm) :
		_vm(vm) {
}

Sound::~Sound() {
	stopSound();
}

Audio::SeekableAudioStream *Sound::makeAudioStream(uint16 id, CueList *cueList, MohawkWaveLoopInfo *loopInfo) {
	Audio::SeekableAudioStream *audStream = nullptr;

	switch (_vm->getGameType()) {
	case GType_ZOOMBINI:
		audStream = makeMohawkWaveStream(_vm->getResource(ID_SND, id), cueList, loopInfo);
		break;
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
		break;
	}

	return audStream;
}

Audio::SoundHandle *Sound::playSound(uint16 id, byte volume, bool loop, CueList *cueList) {
	return playSound(id, Audio::Mixer::kPlainSoundType, volume, loop, cueList);
}

Audio::SoundHandle *Sound::playSound(uint16 id, Audio::Mixer::SoundType soundType, byte volume, bool loop, CueList *cueList) {
	debug (0, "Playing sound %d", id);

	MohawkWaveLoopInfo loopInfo;
	Audio::SeekableAudioStream *seekableStream = makeAudioStream(id, cueList, &loopInfo);

	if (seekableStream) {
		SndHandle *handle = getHandle();
		handle->type = kUsedHandle;
		handle->id = id;
		handle->samplesPerSecond = seekableStream->getRate();

		// Logical Journey of the Zoombinis make use of loop.
		// Playback starts with the resource prefix from frame zero.
		// Reaching the exported exclusive end then seeks to the inclusive start.
		// @ref Audio::SubLoopingAudioStream uses zero iterations to mean an infinite loop.
		// It takes ownership of the decoded seekable stream. The mixer owns the wrapper.
		// The ordinary whole-resource loop policy remains the fallback
		// only when the resource has no supported embedded loop.
		Audio::AudioStream *audStream = seekableStream;
		if (loopInfo.isValid()) {
			audStream = new Audio::SubLoopingAudioStream(seekableStream, 0,
				Audio::Timestamp(0, loopInfo.start, seekableStream->getRate()),
				Audio::Timestamp(0, loopInfo.end, seekableStream->getRate()));
		} else if (loop) { // Set the stream to loop here if it's requested
			audStream = Audio::makeLoopingAudioStream(seekableStream, 0);
		}

		_vm->_mixer->playStream(soundType, &handle->handle, audStream, -1, volume);
		return &handle->handle;
	}

	return nullptr;
}

Audio::SeekableAudioStream *Sound::makeLivingBooksWaveStream_v1(Common::SeekableReadStream *stream) {
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

	if (size >= 4 && ConfMan.getBool("fix_audio_pops")) {
		DataChunk chunk;
		memset(&chunk, 0, sizeof(DataChunk));
		chunk.sampleCount = size;

		scanAndFixAudioPops(chunk, size, stream);
	}

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


MidiPlayer::MidiPlayer(MohawkEngine *vm) : _vm(vm) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driver = MidiDriver::createMidi(dev);
	assert(_driver);
	_paused = false;
	_resetChannelsOnPlay = false;


	int ret = _driver->open();
	if (ret == 0) {
		_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

MidiPlayer::~MidiPlayer() {

}

void MidiPlayer::playMidi(uint16 id) {
	// debugC(3, kDebugMusic, "MidiPlayer::play");

	Common::StackLock lock(_mutex);
	playMidiStreamLocked(makeMidiStream(id), id);
}

void MidiPlayer::playMidiStream(Common::SeekableReadStream *stream, uint16 id) {
	Common::StackLock lock(_mutex);
	playMidiStreamLocked(stream, id);
}

void MidiPlayer::playMidiStreamLocked(Common::SeekableReadStream *stream, uint16 id) {
	stop();
	if (!stream)
		return;

	// The Zoombini Macintosh MIDI profile (MIDIMAC.MHK) relies on a clean GM
	// device state because, unlike the Windows profile (MIDIMPC.MHK), its songs
	// carry no inline GM/GS setup (bank selects, RPN pitch-bend range, resets).
	// Send a GM reset before each song so stale program/controller state from a
	// previous song cannot bleed in. Gated by the caller so the PC profile path
	// stays byte-for-byte unchanged.
	if (_resetChannelsOnPlay)
		_driver->sendGMReset();

	Common::Array<byte> standardMidi;
	const bool extracted = extractMohawkMidi(stream, standardMidi);
	delete stream;
	if (!extracted)
		return;

	_midiData = static_cast<byte *>(malloc(standardMidi.size()));
	if (!_midiData) {
		warning("Could not allocate %u bytes for Mohawk MIDI resource", standardMidi.size());
		return;
	}
	memcpy(_midiData, standardMidi.data(), standardMidi.size());

	_parser = MidiParser::createParser_SMF();
	if (!_parser->loadMusic(_midiData, standardMidi.size())) {
		warning("Could not parse Mohawk MIDI resource %u", id);
		delete _parser;
		_parser = nullptr;
		free(_midiData);
		_midiData = nullptr;
		return;
	}

	syncVolume();
	_parser->setTrack(0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_isLooping = true;
	_isPlaying = true;
}

void MidiPlayer::pause(bool p) {
	_paused = p;

	for (int i = 0; i < kNumChannels; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_paused ? 0 : _channelsVolume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::onTimer() {
	Common::StackLock lock(_mutex);

	if (!_paused && _isPlaying && _parser) {
		_parser->onTimer();
	}
}

void MidiPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

Common::SeekableReadStream *MidiPlayer::makeMidiStream(uint16 id) {
	return _vm->getResource(ID_TMID, id);
}

} // End of namespace Mohawk
