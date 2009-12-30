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

#include "mohawk/sound.h"

#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/mp3.h"
#include "sound/wave.h"


namespace Mohawk {

Sound::Sound(MohawkEngine* vm) : _vm(vm) {
	_rivenSoundFile = NULL;
	_midiDriver = NULL;
	_midiParser = NULL;
	
	for (uint32 i = 0; i < _handles.size(); i++) {
		_handles[i].handle = Audio::SoundHandle();
		_handles[i].type = kFreeHandle;
	}
	
	initMidi();
}

Sound::~Sound() {
	stopSound();
	stopAllSLST();
	delete _rivenSoundFile;
	
	if (_midiDriver) {
		_midiDriver->close();
		delete _midiDriver;
	}
	
	if (_midiParser) {
		_midiParser->unloadMusic();
		delete _midiParser;
	}
}

void Sound::loadRivenSounds(uint16 stack) {
	static const char prefixes[] = { 'a', 'b', 'g', 'j', 'o', 'p', 'r', 't' };

	if (!_rivenSoundFile)
		_rivenSoundFile = new MohawkFile();
	
	_rivenSoundFile->open(Common::String(prefixes[stack]) + "_Sounds.mhk");
}

void Sound::initMidi() {
	if (!(_vm->getFeatures() & GF_HASMIDI))
		return;
		
	// Let's get our MIDI parser/driver
	_midiParser = MidiParser::createParser_SMF();
	_midiDriver = MidiDriver::createMidi(MidiDriver::detectMusicDriver(MDT_ADLIB|MDT_MIDI));
	
	// Set up everything!
	_midiDriver->open();
	_midiParser->setMidiDriver(_midiDriver);
	_midiParser->setTimerRate(_midiDriver->getBaseTempo());
}
        
Audio::SoundHandle *Sound::playSound(uint16 id, bool mainSoundFile, byte volume) {
	debug (0, "Playing sound %d", id);
	
	SndHandle *handle = getHandle();
	handle->type = kUsedHandle;
	
	Audio::AudioStream* audStream = NULL;

	switch (_vm->getGameType()) {
	case GType_MYST:
		if (_vm->getFeatures() & GF_ME) {
			// Myst ME is a bit more efficient with sound storage than Myst
			// Myst has lots of sounds repeated. To overcome this, Myst ME
			// has MJMP resources which provide a link to the actual MSND
			// resource we're looking for. This saves a lot of space from
			// repeated data.
			if (_vm->hasResource(ID_MJMP, id)) {
				Common::SeekableReadStream *mjmpStream = _vm->getRawData(ID_MJMP, id);
				id = mjmpStream->readUint16LE();
				delete mjmpStream;
			}
			
			audStream = Audio::makeWAVStream(_vm->getRawData(ID_MSND, id), true);
		} else
			audStream = makeMohawkWaveStream(_vm->getRawData(ID_MSND, id));
		break;
	case GType_RIVEN:
		if (mainSoundFile)
			audStream = makeMohawkWaveStream(_rivenSoundFile->getRawData(ID_TWAV, id));
		else
			audStream = makeMohawkWaveStream(_vm->getRawData(ID_TWAV, id));
		break;
	case GType_ZOOMBINI:
		audStream = makeMohawkWaveStream(_vm->getRawData(ID_SND, id));
		break;
	case GType_CSAMTRAK:
		if (mainSoundFile)
			audStream = makeMohawkWaveStream(_vm->getRawData(ID_TWAV, id));
		else
			audStream = getCSAmtrakMusic(id);
		break;
	case GType_OLDLIVINGBOOKS:
		audStream = makeOldMohawkWaveStream(_vm->getRawData(ID_WAV, id));
		break;
	default:
		audStream = makeMohawkWaveStream(_vm->getRawData(ID_TWAV, id));
	}
	
	if (audStream) {
		_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &handle->handle, audStream, -1, volume);
		return &handle->handle;
	}
	
	return NULL;
}

void Sound::playMidi(uint16 id) {
	uint32 idTag;
	if (!(_vm->getFeatures() & GF_HASMIDI)) {
		warning ("Attempting to play MIDI in a game without MIDI");
		return;
	}

	assert(_midiDriver && _midiParser);
	
	_midiParser->unloadMusic();
	Common::SeekableReadStream *midi = _vm->getRawData(ID_TMID, id);
	
	idTag = midi->readUint32BE();
	assert(idTag == ID_MHWK);
	midi->readUint32BE(); // Skip size
	idTag = midi->readUint32BE();
	assert(idTag == ID_MIDI);
	
	byte *midiData = (byte *)malloc(midi->size() - 12); // Enough to cover MThd/Prg#/MTrk
	
	// Read the MThd Data
	midi->read(midiData, 14);
	
	// Skip the unknown Prg# section
	idTag = midi->readUint32BE();
	assert(idTag == ID_PRG);
	midi->skip(midi->readUint32BE());
	
	// Read the MTrk Data
	uint32 mtrkSize = midi->size() - midi->pos();
	midi->read(midiData + 14, mtrkSize);
	
	delete midi;

	// Now, play it :)
	if (!_midiParser->loadMusic(midiData, 14 + mtrkSize))
		error ("Could not play MIDI music from tMID %04x\n", id);
		
	_midiDriver->setTimerCallback(_midiParser, MidiParser::timerCallback);
}

void Sound::playSLST(uint16 index, uint16 card) {
	Common::SeekableReadStream *slstStream = _vm->getRawData(ID_SLST, card);
	SLSTRecord slstRecord;
	uint16 recordCount = slstStream->readUint16BE();
	
	for (uint16 i = 0; i < recordCount; i++) {
		slstRecord.index = slstStream->readUint16BE();
		slstRecord.sound_count = slstStream->readUint16BE();
		slstRecord.sound_ids = new uint16[slstRecord.sound_count];

		for (uint16 j = 0; j < slstRecord.sound_count; j++)
			slstRecord.sound_ids[j] = slstStream->readUint16BE();

		slstRecord.fade_flags = slstStream->readUint16BE();
		slstRecord.loop = slstStream->readUint16BE();
		slstRecord.global_volume = slstStream->readUint16BE();
		slstRecord.u0 = slstStream->readUint16BE();			// Unknown

		if (slstRecord.u0 > 1)
			warning("slstRecord.u0: %d non-boolean", slstRecord.u0);

		slstRecord.u1 = slstStream->readUint16BE();			// Unknown

		if (slstRecord.u1 != 0)
			warning("slstRecord.u1: %d non-zero", slstRecord.u1);

		slstRecord.volumes = new uint16[slstRecord.sound_count];
		slstRecord.balances = new int16[slstRecord.sound_count];
		slstRecord.u2 = new uint16[slstRecord.sound_count];

		for (uint16 j = 0; j < slstRecord.sound_count; j++)
			slstRecord.volumes[j] = slstStream->readUint16BE();

		for (uint16 j = 0; j < slstRecord.sound_count; j++)
			slstRecord.balances[j] = slstStream->readSint16BE();	// negative = left, 0 = center, positive = right

		for (uint16 j = 0; j < slstRecord.sound_count; j++) {
			slstRecord.u2[j] = slstStream->readUint16BE();		// Unknown

			if (slstRecord.u2[j] != 255 && slstRecord.u2[j] != 256)
				warning("slstRecord.u2[%d]: %d not 255 or 256", j, slstRecord.u2[j]);
		}

		if (slstRecord.index == index) {
			playSLST(slstRecord);
			delete slstStream;
			return;
		}
		
		delete[] slstRecord.sound_ids;
		delete[] slstRecord.volumes;
		delete[] slstRecord.balances;
		delete[] slstRecord.u2;
	}
	
	delete slstStream;
	// No matching records, assume we need to stop all SLST's
	stopAllSLST();
}

void Sound::playSLST(SLSTRecord slstRecord) {
	// End old sounds
	for (uint16 i = 0; i < _currentSLSTSounds.size(); i++) {
		bool noLongerPlay = true;
		for (uint16 j = 0; j < slstRecord.sound_count; j++)
			if (_currentSLSTSounds[i].id == slstRecord.sound_ids[j])
				noLongerPlay = false;
		if (noLongerPlay)
			stopSLSTSound(i, (slstRecord.fade_flags & 1) != 0);
	}
	
	// Start new sounds
	for (uint16 i = 0; i < slstRecord.sound_count; i++) {
		bool alreadyPlaying = false;
		for (uint16 j = 0; j < _currentSLSTSounds.size(); j++) {
			if (_currentSLSTSounds[j].id == slstRecord.sound_ids[i])
				alreadyPlaying = true;
		}
		if (!alreadyPlaying) {
			playSLSTSound(slstRecord.sound_ids[i],
						 (slstRecord.fade_flags & (1 << 1)) != 0,
						 slstRecord.loop != 0,
						 slstRecord.volumes[i],
						 slstRecord.balances[i]);
		}
	}
}

void Sound::stopAllSLST() {
	for (uint16 i = 0; i < _currentSLSTSounds.size(); i++)
		_vm->_mixer->stopHandle(*_currentSLSTSounds[i].handle);
	_currentSLSTSounds.clear();
}

static int8 convertBalance(int16 balance) {
	return (int8)(balance >> 8);
}

void Sound::playSLSTSound(uint16 id, bool fade, bool loop, uint16 volume, int16 balance) {
	// WORKAROUND: Some Riven SLST entries have a volume of 0, so we just ignore them.
	if (volume == 0)
		return;

	SLSTSndHandle sndHandle;
	sndHandle.handle = new Audio::SoundHandle();
	sndHandle.id = id;
	_currentSLSTSounds.push_back(sndHandle);
	
	Audio::AudioStream *audStream = makeMohawkWaveStream(_rivenSoundFile->getRawData(ID_TWAV, id), loop);
	
	// The max mixer volume is 255 and the max Riven volume is 256. Just change it to 255.
	if (volume == 256)
		volume = 255;
	
	// TODO: Handle fading, possibly just raise the volume of the channel in increments?
	
	_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, sndHandle.handle, audStream, -1, volume, convertBalance(balance));
}

void Sound::stopSLSTSound(uint16 index, bool fade) {
	// TODO: Fade out, mixer needs to be extended to get volume on a handle	
	_vm->_mixer->stopHandle(*_currentSLSTSounds[index].handle);
	_currentSLSTSounds.remove_at(index);
}

void Sound::pauseSLST() {
	for (uint16 i = 0; i < _currentSLSTSounds.size(); i++)
		_vm->_mixer->pauseHandle(*_currentSLSTSounds[i].handle, true);
}

void Sound::resumeSLST() {
	for (uint16 i = 0; i < _currentSLSTSounds.size(); i++)
		_vm->_mixer->pauseHandle(*_currentSLSTSounds[i].handle, false);
}

Audio::AudioStream *Sound::getCSAmtrakMusic(uint16 id) {
	char filename[18];
	sprintf(filename, "MUSIC/MUSIC%02d.MHK", id);
	MohawkFile *file = new MohawkFile();
	file->open(filename);
	Audio::AudioStream *audStream = makeMohawkWaveStream(file->getRawData(ID_TWAV, 2000 + id));
	delete file;
	return audStream;
}

Audio::AudioStream *Sound::makeMohawkWaveStream(Common::SeekableReadStream *stream, bool loop) {
	bool foundData = false;
	uint32 tag = 0;
	ADPC_Chunk adpc;
	Cue_Chunk cue;
	Data_Chunk data_chunk;

	if (stream->readUint32BE() == ID_MHWK) // MHWK tag again
		debug(2, "Found Tag MHWK");
	else
		error ("Could not find tag \'MHWK\'");

	stream->readUint32BE(); // Skip size

	if (stream->readUint32BE() == ID_WAVE)
		debug(2, "Found Tag WAVE");
	else
		error ("Could not find tag \'WAVE\'");

	while (!foundData) {
		tag = stream->readUint32BE();

		switch (tag) {
		case ID_ADPC:
			debug(2, "Found Tag ADPC");
			// Riven ADPCM Sound Only
			// NOTE: This is completely useless for us. All of this
			// is already in the ADPCM decoder in /sound.
			adpc.size = stream->readUint32BE();
			adpc.u0 = stream->readUint16BE();
			adpc.channels = stream->readUint16BE();
			adpc.u1 = stream->readUint32BE();

			for (uint16 i = 0; i < adpc.channels; i++)
				adpc.u2[i] = stream->readUint32BE();
			if (adpc.u0 == 2) {
				adpc.u3 = stream->readUint32BE();
				for (uint16 i = 0; i < adpc.channels; i++)
					adpc.u4[i] = stream->readUint32BE();
			}
			break;
		case ID_CUE:
			debug(2, "Found Tag Cue#");
			// I have not tested this with Myst, but the one Riven test-case,
			// pspit tWAV 3, has two cue points: "Beg Loop" and "End Loop".
			// So, my guess is that a cue chunk just holds where to loop the
			// sound. Some cue chunks even have no point count (such as
			// Myst's intro.dat MSND 2. So, my theory is that a cue chunk
			// always represents a loop, and if there is a point count, that
			// represents the points from which to loop.
			//
			// This theory is probably not entirely true anymore. I've found
			// that the values (which were previously unknown) in the DATA
			// chunk are for looping. Since it was only used in Myst, it was
			// always 10 0's, Tito just thought it was useless. I'm still not
			// sure what purpose this has.
		
			cue.size = stream->readUint32BE();
			cue.point_count = stream->readUint16BE();

			if (cue.point_count == 0)
				debug (2, "Cue# chunk found with no points!");
			else
				debug (2, "Cue# chunk found with %d point(s)!", cue.point_count);

			for (uint16 i = 0; i < cue.point_count; i++) {
				cue.cueList[i].position = stream->readUint32BE();
				cue.cueList[i].length = stream->readByte();
				for (byte j = 0; j < cue.cueList[i].length; j++)
					cue.cueList[i].name += stream->readByte();
				// Realign to uint16 boundaries...
				if (!(cue.cueList[i].length & 1))
					stream->readByte();
				debug (3, "Cue# chunk point %d: %s", i, cue.cueList[i].name.c_str());
			}
			break;
		case ID_DATA:
			debug(2, "Found Tag DATA");
			// We subtract 20 from the actual chunk size, which is the total size
			// of the chunk's header
			data_chunk.size = stream->readUint32BE() - 20;
			data_chunk.sample_rate = stream->readUint16BE();
			data_chunk.sample_count = stream->readUint32BE();
			data_chunk.bitsPerSample = stream->readByte();
			data_chunk.channels = stream->readByte();
			data_chunk.encoding = stream->readUint16BE();
			data_chunk.loop = stream->readUint16BE();
			data_chunk.loopStart = stream->readUint32BE();
			data_chunk.loopEnd = stream->readUint32BE();

			data_chunk.audio_data = (byte *)malloc(data_chunk.size);
			stream->read(data_chunk.audio_data, data_chunk.size);
			foundData = true;
			break;
		default:
			error ("Unknown tag found in 'tWAV' chunk -- \'%s\'", tag2str(tag));
		}
	}
	
	// makeMohawkWaveStream always takes control of the original stream
	delete stream;

	// The sound in Myst uses raw unsigned 8-bit data
	// The sound in the CD version of Riven is encoded in Intel DVI ADPCM
	// The sound in the DVD version of Riven is encoded in MPEG-2 Layer II or Intel DVI ADPCM
	if (data_chunk.encoding == kCodecRaw) {
		byte flags = Audio::Mixer::FLAG_UNSIGNED|Audio::Mixer::FLAG_AUTOFREE;
		if (data_chunk.channels == 2)
			flags |= Audio::Mixer::FLAG_STEREO;
		if (data_chunk.loop == 0xFFFF || loop)
			flags |= Audio::Mixer::FLAG_LOOP;
		return Audio::makeLinearInputStream(data_chunk.audio_data, data_chunk.size, data_chunk.sample_rate, flags, data_chunk.loopStart, data_chunk.loopEnd);
	} else if (data_chunk.encoding == kCodecADPCM) {
		Common::MemoryReadStream *dataStream = new Common::MemoryReadStream(data_chunk.audio_data, data_chunk.size, Common::DisposeAfterUse::YES);
		uint32 blockAlign = data_chunk.channels * data_chunk.bitsPerSample / 8;
		return Audio::makeADPCMStream(dataStream, true, data_chunk.size, Audio::kADPCMIma, data_chunk.sample_rate, data_chunk.channels, blockAlign, !loop);
	} else if (data_chunk.encoding == kCodecMPEG2) {
#ifdef USE_MAD
		Common::MemoryReadStream *dataStream = new Common::MemoryReadStream(data_chunk.audio_data, data_chunk.size, Common::DisposeAfterUse::YES);
		return Audio::makeMP3Stream(dataStream, true, 0, 0, !loop);
#else
		warning ("MAD library not included - unable to play MP2 audio");
#endif
	} else {
		error ("Unknown Mohawk WAVE encoding %d", data_chunk.encoding);
	}

	return NULL;
}

Audio::AudioStream *Sound::makeOldMohawkWaveStream(Common::SeekableReadStream *stream, bool loop) {
	uint16 header = stream->readUint16BE();
	uint16 rate = 0;
	uint32 size = 0;
	
	if (header == 'Wv') { // Big Endian
		rate = stream->readUint16BE();
		stream->skip(10); // Loop chunk, like the newer format?
		size = stream->readUint32BE();
	} else if (header == 'vW') { // Little Endian
		rate = stream->readUint16LE();
		stream->skip(10); // Loop chunk, like the newer format?
		size = stream->readUint32LE();
	} else
		error("Could not find Old Mohawk Sound header");
		
	assert(size);
	byte *data = (byte *)malloc(size);
	stream->read(data, size);
	delete stream;
	
	byte flags = Audio::Mixer::FLAG_UNSIGNED|Audio::Mixer::FLAG_AUTOFREE;
	
	if (loop)
		flags |= Audio::Mixer::FLAG_LOOP;
	
	return Audio::makeLinearInputStream(data, size, rate, flags, 0, 0);
}

SndHandle *Sound::getHandle() {
	for (uint32 i = 0; i < _handles.size(); i++) {
		if (_handles[i].type == kFreeHandle)
			return &_handles[i];

		if (!_vm->_mixer->isSoundHandleActive(_handles[i].handle)) {
			_handles[i].type = kFreeHandle;
			return &_handles[i];
		}
	}

	// Let's add a new sound handle!
	SndHandle handle;
	handle.handle = Audio::SoundHandle();
	handle.type = kFreeHandle;
	_handles.push_back(handle);

	return &_handles[_handles.size() - 1];
}

void Sound::stopSound() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle) {
			_vm->_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
		}
}

void Sound::pauseSound() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle)
			_vm->_mixer->pauseHandle(_handles[i].handle, true);
}

void Sound::resumeSound() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle)
			_vm->_mixer->pauseHandle(_handles[i].handle, false);
}

} // End of namespace Mohawk
