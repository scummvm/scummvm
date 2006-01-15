/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "scumm/actor.h"
#include "scumm/imuse.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"

#include "common/config-manager.h"
#include "common/timer.h"
#include "common/util.h"

#include "sound/adpcm.h"
#include "sound/audiocd.h"
#include "sound/flac.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/voc.h"
#include "sound/vorbis.h"
#include "sound/wave.h"

namespace Scumm {

int Sound::isSoundCodeUsed(int sound) {
	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1) {
		return _heChannel[chan].sbngBlock;
	} else {
		return 0;
	}
}

int Sound::getSoundPos(int sound) {
	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1) {
		int time =  _vm->getHETimer(chan + 4) * 11025 / 1000;
		return time;
	} else {
		return 0;
	}
}

int Sound::getSoundVar(int sound, int var) {
	if (_vm->_heversion >= 90 && var == 26) {
		return isSoundCodeUsed(sound);
	}

	checkRange(25, 0, var, "Illegal sound variable %d");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1) {
		debug(0, "getSoundVar: sound %d var %d result %d", sound, var, _heChannel[chan].soundVars[var]);
		return _heChannel[chan].soundVars[var];
	} else {
		return 0;
	}
}

void Sound::setSoundVar(int sound, int var, int val) {
	checkRange(25, 0, var, "Illegal sound variable %d");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1) {
		debug(0, "setSoundVar: sound %d var %d val %d", sound, var, val);
		_heChannel[chan].soundVars[var] = val;
	}
}

void Sound::setOverrideFreq(int freq) {
	_overrideFreq = freq;
}

void Sound::setupHEMusicFile() {
	int i, total_size;
	char buf[32], buf1[128];
	Common::File musicFile;

	sprintf(buf, "%s.he4", _vm->getGameName());

	if (_vm->_substResFileNameIndex > 0) {
		_vm->generateSubstResFileName(buf, buf1, sizeof(buf1));
		strcpy(buf, buf1);
	}
	if (musicFile.open(buf) == true) {
		musicFile.seek(4, SEEK_SET);
		total_size = musicFile.readUint32BE();
		musicFile.seek(16, SEEK_SET);
		_heMusicTracks = musicFile.readUint32LE();
		debug(0, "Total music tracks %d", _heMusicTracks);

		int musicStart = (_vm->_heversion >= 80) ? 56 : 20;
		musicFile.seek(musicStart, SEEK_SET);

		_heMusic = (HEMusic *)malloc((_heMusicTracks + 1) * sizeof(HEMusic));
		for (i = 0; i < _heMusicTracks; i++) {
			_heMusic[i].id = musicFile.readUint32LE();
			_heMusic[i].offset = musicFile.readUint32LE();
			_heMusic[i].size = musicFile.readUint32LE();

			if (_vm->_heversion >= 80) {
				musicFile.seek(+9, SEEK_CUR);
			} else {
				musicFile.seek(+13, SEEK_CUR);
			}
		}

		musicFile.close();
	}
}

bool Sound::getHEMusicDetails(int id, int &musicOffs, int &musicSize) {
	int i;

	for (i = 0; i < _heMusicTracks; i++) {
		if (_heMusic[i].id == id) {
			musicOffs = _heMusic[i].offset;
			musicSize = _heMusic[i].size;
			return 1;
		}
	}

	return 0;
}

void Sound::processSoundCode() {
	byte *codePtr;
	int chan, tmr, size, time;

	for (chan = 0; chan < ARRAYSIZE(_heChannel); chan++) {
		if (_heChannel[chan].sound == 0) {
			continue;
		}

		if (_heChannel[chan].codeOffs == -1) {
			continue;
		}

		tmr = _vm->getHETimer(chan + 4) * 11025 / 1000;
		tmr += _vm->VAR(_vm->VAR_SOUNDCODE_TMR);
		if (tmr < 0)
			tmr = 0;

		if (_heChannel[chan].sound > _vm->_numSounds) {
			codePtr = _vm->getResourceAddress(rtSpoolBuffer, chan);
		} else {
			codePtr = _vm->getResourceAddress(rtSound, _heChannel[chan].sound);
		}
		assert(codePtr);
		codePtr += _heChannel[chan].codeOffs;

		while(1) {
			size = READ_LE_UINT16(codePtr);
			time = READ_LE_UINT32(codePtr + 2);

			if (size == 0) {
				_heChannel[chan].codeOffs = -1;
				break;
			}

			debug(1, "Channel %d Timer %d Time %d", chan,tmr, time);
			if (time >= tmr)
				break;

			processSoundOpcodes(_heChannel[chan].sound, codePtr + 6, _heChannel[chan].soundVars);

			codePtr += size;
			_heChannel[chan].codeOffs += size;
		}
	}
}

void Sound::processSoundOpcodes(int sound, byte *codePtr, int *soundVars) {
	int edi, opcode, var, val;

	while(READ_LE_UINT16(codePtr) != 0) {
		codePtr += 2;
		opcode = READ_LE_UINT16(codePtr); codePtr += 2;
		opcode &= ~0xF000;
		opcode /= 16;
		edi = opcode;
		opcode &= ~3;
		edi &= 3;

		debug(1, "processSoundOpcodes: sound %d opcode %d", sound, opcode);
		switch (opcode) {
		case 0: // Continue
			break;
		case 16: // Set talk state
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			setSoundVar(sound, 19, val);
			break;
		case 32: // Set var
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (edi == 2) {
				val = getSoundVar(sound, val);
			}
			setSoundVar(sound, var, val);
			break;
		case 48: // Add
			var = READ_LE_UINT16(codePtr); codePtr += 2;;
			val = READ_LE_UINT16(codePtr); codePtr += 2;;

			val = getSoundVar(sound, var) + val;
			setSoundVar(sound, var, val);
			break;
		case 56: // Subtract
			var = READ_LE_UINT16(codePtr); codePtr += 2;;
			val = READ_LE_UINT16(codePtr); codePtr += 2;;

			val = getSoundVar(sound, var) - val;
			setSoundVar(sound, var, val);
			break;
		case 64: // Multiple
			var = READ_LE_UINT16(codePtr); codePtr += 2;;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (edi == 2) {
				val = getSoundVar(sound, val);
			}

			val = getSoundVar(sound, var) * val;
			setSoundVar(sound, var, val);
			break;
		case 80: // Divide
			var = READ_LE_UINT16(codePtr); codePtr += 2;;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (edi == 2) {
				val = getSoundVar(sound, val);
			}

			val = getSoundVar(sound, var) / val;
			setSoundVar(sound, var, val);
			break;
		case 96: // Increment
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) + 1;
			setSoundVar(sound, var, val);
			break;
		case 104: // Decrement
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) - 1;
			setSoundVar(sound, var, val);
			break;
		default:
			error("Illegal sound %d opcode %d", sound, opcode);
		}
	}
}

void Sound::playHESound(int soundID, int heOffset, int heChannel, int heFlags) {
	debug(0,"playHESound: soundID %d heOffset %d heChannel %d heFlags %d", soundID, heOffset, heChannel, heFlags);
	byte *ptr, *spoolPtr;
	char *sound;
	int size = -1;
	int priority, rate;
	byte flags = Audio::Mixer::FLAG_UNSIGNED | Audio::Mixer::FLAG_AUTOFREE;

	if (heChannel == -1) {
		if (_vm->_heversion >= 95 && _vm->VAR(_vm->VAR_DEFAULT_SOUND_CHANNEL) != 0) {
			heChannel = _vm->VAR(_vm->VAR_DEFAULT_SOUND_CHANNEL);
		} else {
			heChannel = 1;
		}
	}

	if (soundID > _vm->_numSounds) {
		if (soundID >= 10000) {
			// Special codes, used in pjgames
			return;
		}

		int music_offs;
		char buf[32], buf1[128];
		Common::File musicFile;

		sprintf(buf, "%s.he4", _vm->getGameName());

		if (_vm->_substResFileNameIndex > 0) {
			_vm->generateSubstResFileName(buf, buf1, sizeof(buf1));
			strcpy(buf, buf1);
		}
		if (musicFile.open(buf) == false) {
			warning("playSound: Can't open music file %s", buf);
			return;
		}
		if (!getHEMusicDetails(soundID, music_offs, size)) {
			debug(0, "playSound: musicID %d not found", soundID);
			return;
		}

		musicFile.seek(music_offs, SEEK_SET);

		if (_vm->_heversion == 70) {
			spoolPtr = (byte *)malloc(size);
			musicFile.read(spoolPtr, size);
		} else {
			spoolPtr = _vm->res.createResource(rtSpoolBuffer, heChannel, size);
			assert(spoolPtr);
			musicFile.read(spoolPtr, size);
		}
		musicFile.close();

		_vm->_mixer->stopID(_currentMusic);
		_currentMusic = soundID;
		if (_vm->_heversion == 70) {
			_vm->_mixer->playRaw(&_heSoundChannels[heChannel], spoolPtr, size, 11025, flags, soundID);
			return;
		}
	}

	if (soundID > _vm->_numSounds) {
		ptr = _vm->getResourceAddress(rtSpoolBuffer, heChannel);
	} else {
		ptr = _vm->getResourceAddress(rtSound, soundID);
	}

	if (!ptr) {
		return;
	}

	// Support for sound in later Backyard sports games
	if (READ_UINT32(ptr) == MKID('RIFF')) {
		uint16 type;
		int blockAlign;
		size = READ_LE_UINT32(ptr + 4);
		Common::MemoryReadStream stream(ptr, size);

		if (!loadWAVFromStream(stream, size, rate, flags, &type, &blockAlign)) {
			error("playSound: Not a valid WAV file");
		}

		if (type == 17) {
			AudioStream *voxStream = new ADPCMInputStream(&stream, size, kADPCMIma, (flags & Audio::Mixer::FLAG_STEREO) ? 2 : 1, blockAlign);

			sound = (char *)malloc(size * 4);
			size = voxStream->readBuffer((int16*)sound, size * 2);
			size *= 2; // 16bits.
		} else {
			// Allocate a sound buffer, copy the data into it, and play
			sound = (char *)malloc(size);
			memcpy(sound, ptr + stream.pos(), size);
		}
		_vm->_mixer->playRaw(&_heSoundChannels[heChannel], sound, size, rate, flags, soundID);
	}
	// Support for sound in Humongous Entertainment games
	else if (READ_UINT32(ptr) == MKID('DIGI') || READ_UINT32(ptr) == MKID('TALK')) {
		byte *sndPtr = ptr;

		priority = *(ptr + 18);
		rate = READ_LE_UINT16(ptr + 22);
		ptr += 8 + READ_BE_UINT32(ptr + 12);

		// TODO
		/* if (_vm->_mixer->isSoundHandleActive(_heSoundChannels[heChannel])) {
			int curSnd = _heChannel[heChannel].sound;
			if (curSnd != 0 && curSnd != 1 && soundID != 1 && _heChannel[heChannel].priority > priority)
				return;
		} */

		int codeOffs = -1;
		if (READ_UINT32(ptr) == MKID('SBNG')) {
			codeOffs = ptr - sndPtr + 8;
			ptr += READ_BE_UINT32(ptr + 4);
		}

		assert(READ_UINT32(ptr) == MKID('SDAT'));
		size = READ_BE_UINT32(ptr+4) - 8;
		if (heOffset < 0 || heOffset > size) {
			// Occurs when making fireworks in puttmoon
			debug(0, "playSound: Invalid sound offset (offset %d, size %d) in sound %d", heOffset, size, soundID);
			heOffset = 0;
		}
		size -= heOffset;

		if (_overrideFreq) {
			// Used by the piano in Fatty Bear's Birthday Surprise
			rate = _overrideFreq;
			_overrideFreq = 0;
		}

		// TODO
		if (heFlags & 1) {
			//flags |= Audio::Mixer::FLAG_LOOP;
		}

		// Allocate a sound buffer, copy the data into it, and play
		sound = (char *)malloc(size);
		memcpy(sound, ptr + heOffset + 8, size);
		_vm->_mixer->playRaw(&_heSoundChannels[heChannel], sound, size, rate, flags, soundID);

		_vm->setHETimer(heChannel + 4);
		_heChannel[heChannel].sound = soundID;
		_heChannel[heChannel].priority = priority;
		_heChannel[heChannel].sbngBlock = (codeOffs != -1) ? 1 : 0;
		_heChannel[heChannel].codeOffs = codeOffs;
		memset(_heChannel[heChannel].soundVars, 0, sizeof(_heChannel[heChannel].soundVars));
	}
	// Support for PCM music in 3DO versions of Humongous Entertainment games
	else if (READ_UINT32(ptr) == MKID('MRAW')) {
		priority = *(ptr + 18);
		rate = READ_LE_UINT16(ptr + 22);
		ptr += 8 + READ_BE_UINT32(ptr+12);

		assert(READ_UINT32(ptr) == MKID('SDAT'));
		size = READ_BE_UINT32(ptr+4) - 8;

		flags = Audio::Mixer::FLAG_AUTOFREE;

		// Allocate a sound buffer, copy the data into it, and play
		sound = (char *)malloc(size);
		memcpy(sound, ptr + 8, size);
		_vm->_mixer->stopID(_currentMusic);
		_currentMusic = soundID;
		_vm->_mixer->playRaw(NULL, sound, size, rate, flags, soundID);
	}
	else {
		if (_vm->_musicEngine) {
			_vm->_musicEngine->startSound(soundID);
		}
	}
}

void Sound::startHETalkSound(uint32 offset) {
	byte *ptr;
	int32 size;

	if (ConfMan.getBool("speech_mute"))
		return;

	if (!_sfxFile->isOpen()) {
		error("startHETalkSound: Speech file is not open");
		return;
	}

	_sfxMode |= 2;
	_vm->res.nukeResource(rtSound, 1);

	_sfxFile->seek(offset + 4, SEEK_SET);
	 size = _sfxFile->readUint32BE();
	_sfxFile->seek(offset, SEEK_SET);

	_vm->res.createResource(rtSound, 1, size);
	ptr = _vm->getResourceAddress(rtSound, 1);
	_sfxFile->read(ptr, size);

	int channel = (_vm->VAR_SOUND_CHANNEL != 0xFF) ? _vm->VAR(_vm->VAR_SOUND_CHANNEL) : 0;
	addSoundToQueue2(1, 0, channel, 0);
}

} // End of namespace Scumm
