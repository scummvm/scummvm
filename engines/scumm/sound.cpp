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

#include "common/config-manager.h"
#include "common/util.h"
#include "common/ptr.h"
#include "common/substream.h"

#include "scumm/actor.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/players/player_towns.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"

#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/mididrv.h"
#include "audio/mixer.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"

namespace Scumm {

struct MP3OffsetTable {					/* Compressed Sound (.SO3) */
	int org_offset;
	int new_offset;
	int num_tags;
	int compressed_size;
};


Sound::Sound(ScummEngine *parent, Audio::Mixer *mixer, bool useReplacementAudioTracks)
	:
	_vm(parent),
	_mixer(mixer),
	_speechTimerMod(0),
	_midiQueuePos(0),
	_soundQueuePos(0),
	_sfxFileEncByte(0),
	_offsetTable(nullptr),
	_numSoundEffects(0),
	_soundMode(kVOCMode),
	_queuedSfxOffset(0),
	_queuedTalkieOffset(0),
	_queuedSfxLen(0),
	_queuedTalkieLen(0),
	_queuedSoundMode(DIGI_SND_MODE_EMPTY),
	_queuedSfxChannel(0),
	_mouthSyncMode(false),
	_endOfMouthSync(false),
	_curSoundPos(0),
	_currentMusic(0),
	_lastSound(0),
	_soundsPaused(false),
	_digiSndMode(DIGI_SND_MODE_EMPTY) {

	memset(_midiQueue, 0, sizeof(_midiQueue));
	memset(_soundQueue, 0, sizeof(_soundQueue));
	memset(_mouthSyncTimes, 0, sizeof(_mouthSyncTimes));

	_musicType = MDT_NONE;

	_talkChannelHandle = new Audio::SoundHandle();

	if (_vm->_game.features & GF_DOUBLEFINE_PAK)
		_soundSE = new SoundSE(_vm, _mixer);
	_soundCD = new SoundCD(_vm, _mixer, _soundSE, useReplacementAudioTracks);

	_useRemasteredAudio = ConfMan.getBool("use_remastered_audio");

	// This timer targets every talkie game, except for LOOM CD
	// which is handled differently, and except for COMI which
	// handles lipsync within Digital iMUSE.
	if (_vm->_game.version >= 5 && _vm->_game.version <= 7 && _vm->_game.heversion == 0) {
		startSpeechTimer();
	}
}

Sound::~Sound() {
	free(_offsetTable);
	delete _talkChannelHandle;
	delete _soundSE;
	delete _soundCD;
	if (_vm->_game.version >= 5 && _vm->_game.version <= 7 && _vm->_game.heversion == 0) {
		stopSpeechTimer();
	}
}

void Sound::startSound(int sound, int offset, int channel, int flags, int freq, int pan, int volume) {
	if (_vm->VAR_LAST_SOUND != 0xFF)
		_vm->VAR(_vm->VAR_LAST_SOUND) = sound;
	_lastSound = sound;

	addSoundToQueue(sound, offset, channel, flags, freq, pan, volume);
}

void Sound::addSoundToQueue(int sound, int offset, int channel, int flags, int freq, int pan, int volume) {
	assert(_soundQueuePos < ARRAYSIZE(_soundQueue));
	_soundQueue[_soundQueuePos].sound = sound;
	_soundQueue[_soundQueuePos].offset = offset;
	_soundQueue[_soundQueuePos].channel = channel;
	_soundQueue[_soundQueuePos].flags = flags;
	_soundQueue[_soundQueuePos].freq = freq;
	_soundQueue[_soundQueuePos].pan = pan;
	_soundQueue[_soundQueuePos].vol = volume;
	_soundQueuePos++;
}

void Sound::processSound() {
	if (_vm->_game.version >= 7) {
		processSfxQueues();
	} else if (_vm->_game.heversion >= 80) {
		processSoundQueues();
	} else {
		processSfxQueues();
		processSoundQueues();
	}
}

void Sound::processSoundQueues() {
	int i = 0, num;
	int snd;
	int data[16];

	while (_soundQueuePos) {
		_soundQueuePos--;
		snd = _soundQueue[_soundQueuePos].sound;
		if (snd)
			triggerSound(snd);
	}

	while (i < _midiQueuePos) {
		num = _midiQueue[i++];
		if (i + num > _midiQueuePos) {
			error("processSoundQues: invalid num value");
			break;
		}
		memset(data, 0, sizeof(data));
		if (num > 0) {
			for (int j = 0; j < num; j++)
				data[j] = _midiQueue[i + j];
			i += num;

			debugC(DEBUG_IMUSE, "processSoundQues(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
						data[0] >> 8, data[0] & 0xFF,
						data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

			if (_vm->_townsPlayer)
				_vm->VAR(_vm->VAR_SOUNDRESULT) = (short)_vm->_townsPlayer->doCommand(num, data);
			else if (_vm->_imuse)
				_vm->VAR(_vm->VAR_SOUNDRESULT) = (short)_vm->_imuse->doCommand(num, data);
		}
	}
	_midiQueuePos = 0;
}

void Sound::triggerSound(int soundID) {
	byte *ptr;
	byte *sound;
	Audio::AudioStream *stream;
	int size = -1;
	int rate;

	if (_soundCD->triggerCDSound(soundID))
		return;

	debugC(DEBUG_SOUND, "triggerSound #%d", soundID);

	ptr = _vm->getResourceAddress(rtSound, soundID);

	if (!ptr) {
		return;
	}

	// WORKAROUND bug #2221
	else if (READ_BE_UINT32(ptr) == 0x460e200d) {
		// This sound resource occurs in the Macintosh version of Monkey Island.
		// I do now know whether it is used in any place other than the one
		// mentioned in the bug report above; in case it is, I put a check here.
		assert(soundID == 39);

		// The samplerate is copied from the sound resource 39 of the PC CD/VGA
		// version of Monkey Island.

		// Read info from the header
		size = READ_BE_UINT32(ptr+4);
		rate = 6849;

		// Skip over the header (fixed size)
		ptr += 0x26;

		// Allocate a sound buffer, copy the data into it, and play
		sound = (byte *)malloc(size);
		memcpy(sound, ptr, size);
		stream = Audio::makeRawStream(sound, size, rate, Audio::FLAG_UNSIGNED);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, nullptr, stream, soundID);
	}
	// Support for sampled sound effects in Monkey Island 1 and 2
	else if (_vm->_game.platform != Common::kPlatformFMTowns
	         && _vm->_game.platform != Common::kPlatformMacintosh
	         && READ_BE_UINT32(ptr) == MKTAG('S','B','L',' ')) {
		debugC(DEBUG_SOUND, "Using SBL sound effect");

		// SBL resources essentially contain VOC sound data.
		// There are at least two main variants: in one,
		// there are two subchunks AUhd and AUdt, in the other
		// the chunks are called WVhd and WVdt. Besides that,
		// the two variants seem pretty similar.

		// The first subchunk (AUhd resp. WVhd) seems to always
		// contain three bytes (00 00 80) of unknown meaning.
		// After that, a second subchunk contains VOC data.
		// Two real examples:
		//
		// 53 42 4c 20 00 00 11 ae  |SBL ....|
		// 41 55 68 64 00 00 00 03  |AUhd....|
		// 00 00 80 41 55 64 74 00  |...AUdt.|
		// 00 11 9b 01 96 11 00 a6  |........|
		// 00 7f 7f 7e 7e 7e 7e 7e  |...~~~~~|
		// 7e 7f 7f 80 80 7f 7f 7f  |~.......|
		// 7f 80 80 7f 7e 7d 7d 7e  |....~}}~|
		// 7e 7e 7e 7e 7e 7e 7e 7f  |~~~~~~~.|
		//
		// And from the non-interactive Sam & Max demo:
		//
		// 53 42 4c 20 00 01 15 6e  |SBL ...n|
		// 57 56 68 64 00 00 00 03  |WVhd....|
		// 00 00 80 57 56 64 74 00  |...WVdt.|
		// 01 15 5b 01 56 15 01 a6  |..[.V...|
		// 00 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|

		size = READ_BE_UINT32(ptr + 4) - 27;
		ptr += 27;

		// Fingolfin says: after eyeballing a single SEGA
		// SBL resource, it would seem as if the content of the
		// data subchunk (AUdt) is XORed with 0x16. At least
		// then a semi-sane VOC header is revealed, with
		// a sampling rate of ~25000 Hz (does that make sense?).
		// I'll add some code to test that theory for now.

		// Check if the resource has already been demangled
		if ((_vm->_game.platform == Common::kPlatformSegaCD) && (ptr[0] != 1))	{
			for (int i = 0; i < size; i++)   {
				ptr[i] ^= 0x16;
				if (ptr[i] >= 0x7F) {
					ptr[i] = 0xFE - ptr[i];
					ptr[i] ^= 0x80;
				}
			}
		}

		// TODO: It would be nice if we could use readVOCFromMemory() here.
		// We'd have to add the 'Creative Voice File' header for this, though,
		// or make readVOCFromMemory() less strict.

		Audio::VocBlockHeader &voc_block_hdr = *(Audio::VocBlockHeader *)ptr;
		assert(voc_block_hdr.blocktype == 1);
		size = voc_block_hdr.size[0] + (voc_block_hdr.size[1] << 8) + (voc_block_hdr.size[2] << 16) - 2;
		rate = Audio::getSampleRateFromVOCRate(voc_block_hdr.sr);
		assert(voc_block_hdr.pack == 0);

		// Allocate a sound buffer, copy the data into it, and play
		sound = (byte *)malloc(size);
		memcpy(sound, ptr + 6, size);
		stream = Audio::makeRawStream(sound, size, rate, Audio::FLAG_UNSIGNED);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, nullptr, stream, soundID);
	}
	else if (_vm->_game.platform != Common::kPlatformFMTowns && READ_BE_UINT32(ptr) == MKTAG('S','O','U','N')) {
		if (_vm->_game.version != 3)
			ptr += 2;

		int type = *(ptr + 0x0D);

		if (type == 2) {
			// CD track resource
			ptr += 0x16;
			if (soundID == _soundCD->_currentCDSound && _soundCD->pollCD() == 1)
				return;

			int track = ptr[0];
			int loops = ptr[1];
			int start = (ptr[2] * 60 + ptr[3]) * 75 + ptr[4];
			int end = (ptr[5] * 60 + ptr[6]) * 75 + ptr[7];

			// Add the user-specified adjustments.
			if (_vm->_game.id == GID_MONKEY && track == 17) {
				int adjustment = ConfMan.getInt(start == 0 ? "mi1_intro_adjustment" : "mi1_outlook_adjustment");

				start += ((75 * adjustment) / 100);
			}

			_soundCD->playCDTrack(track, loops == 0xff ? -1 : loops, start, end <= start ? 0 : end - start);
			_soundCD->_currentCDSound = soundID;
		} else {
			// All other sound types are ignored
			warning("Scumm::Sound::triggerSound: encountered audio resource with chunk type 'SOUN' and sound type %d", type);
		}
	}
	else {
		if (_vm->_game.id == GID_MONKEY_VGA || _vm->_game.id == GID_MONKEY_EGA) {
			// Works around the fact that in some places in MonkeyEGA/VGA,
			// the music is never explicitly stopped.
			// Rather it seems that starting a new music is supposed to
			// automatically stop the old song.
			if (_vm->_imuse) {
				if (READ_BE_UINT32(ptr) != MKTAG('A','S','F','X'))
					_vm->_imuse->stopAllSounds();
			}
		}

		// TODO: If called from MI2SE, this will play the music
		// multiple times
		//if (_soundSE) {
		//	_soundSE->startSound(soundID);
		//	return;
		//}

		if (_vm->_musicEngine)
			_vm->_musicEngine->startSound(soundID);

		if (_vm->_townsPlayer)
			_soundCD->_currentCDSound = _vm->_townsPlayer->getCurrentCdaSound();
	}
}

void Sound::processSfxQueues() {

	if (_queuedSoundMode != DIGI_SND_MODE_EMPTY) {
		if (_queuedSoundMode & DIGI_SND_MODE_SFX)
			startTalkSound(_queuedSfxOffset, _queuedSfxLen, 1);
		if (_queuedSoundMode & DIGI_SND_MODE_TALKIE)
			startTalkSound(_queuedTalkieOffset, _queuedTalkieLen, 2, _talkChannelHandle);
		_queuedSoundMode = DIGI_SND_MODE_EMPTY;
	}

	const int act = _vm->getTalkingActor();
	if ((_digiSndMode & DIGI_SND_MODE_TALKIE) && act != 0) {
		Actor *a;
		bool finished;

		if (_vm->_imuseDigital) {
			finished = !isSoundRunning(kTalkSoundID);
			if (_vm->_game.id == GID_CMI) {
#if defined(ENABLE_SCUMM_7_8)
				_curSoundPos = _vm->_imuseDigital->getSoundElapsedTimeInMs(kTalkSoundID) * 60 / 1000;
#endif
			}
		} else if (_vm->_game.heversion >= 60) {
			finished = !isSoundInUse(1);
		} else {
			finished = !_mixer->isSoundHandleActive(*_talkChannelHandle);
		}

		if (_vm->_game.heversion == 0 &&
			((uint)act < 0x80 && ((_vm->_game.version == 8) || (_vm->_game.version <= 7 && !_vm->_string[0].no_talk_anim)))) {
			a = _vm->derefActor(act, "processSfxQueues");
			if (a->isInCurrentRoom()) {
				if (finished || (isMouthSyncOff(_curSoundPos) && _mouthSyncMode)) {
					a->runActorTalkScript(a->_talkStopFrame);
					_mouthSyncMode = 0;
				} else if (isMouthSyncOff(_curSoundPos) == 0 && !_mouthSyncMode) {
					a->runActorTalkScript(a->_talkStartFrame);
					_mouthSyncMode = 1;
				}
			}
#if defined(ENABLE_SCUMM_7_8)
			if (_vm->_imuseDigital && !_vm->_imuseDigital->isFTSoundEngine()) {
				int volume = a->_talkVolume;
				int frequency = a->_talkFrequency;
				int pan = a->_talkPan;
				if (_vm->_imuseDigital->isSoundRunning(kTalkSoundID)) {
					if (_vm->VAR(_vm->VAR_VOICE_MODE) == 2)
						volume = 0;
					if (_vm->_imuseDigital->getCurSpeechVolume() != volume) {
						_vm->_imuseDigital->setVolume(kTalkSoundID, volume);
					}
					if (_vm->_imuseDigital->getCurSpeechFrequency() != frequency) {
						_vm->_imuseDigital->setFrequency(kTalkSoundID, frequency);
					}
					if (_vm->_imuseDigital->getCurSpeechPan() != pan) {
						_vm->_imuseDigital->setPan(kTalkSoundID, pan);
					}
				}
			}
#endif
		}

		if ((!ConfMan.getBool("subtitles") && finished) || (finished && _vm->_talkDelay == 0)) {
			if (!(_vm->_game.version == 8 && _vm->VAR(_vm->VAR_HAVE_MSG) == 0))
				_vm->stopTalk();
		}
	}

	if (_digiSndMode & DIGI_SND_MODE_SFX) {
		if (isSfxFinished()) {
			_digiSndMode &= ~DIGI_SND_MODE_SFX;
		}
	}
}

static int compareMP3OffsetTable(const void *a, const void *b) {
	return ((const MP3OffsetTable *)a)->org_offset - ((const MP3OffsetTable *)b)->org_offset;
}

static Audio::AudioStream *checkForBrokenIndy4Sample(Common::SeekableReadStream *file, uint32 offset) {
	byte vocHeader[32];

	file->read(vocHeader, 32);
	// If the bogus VOC header isn't found, don't apply the workaround
	if (memcmp(vocHeader, "Creative Voice File\x1a\x1a\x00\x0a\x01\x29\x11\x01\x02\x50\x01\xa6\x00", 32) != 0) {
		file->seek(-32, SEEK_CUR);
		return nullptr;
	}

	const int size = 86016; // size of speech sample
	offset += 32; // size of VOC header
	return Audio::makeRawStream(
		new Common::SeekableSubReadStream(
			file,
			offset,
			offset + size,
			DisposeAfterUse::YES
		),
		44100,
		Audio::FLAG_16BITS,
		DisposeAfterUse::YES
	);
}

void Sound::startTalkSound(uint32 offset, uint32 length, int mode, Audio::SoundHandle *handle) {
	int num = 0, i;
	int id = -1;
	int size = 0;
	Common::ScopedPtr<ScummFile> file;
	uint32 origOffset = offset;

	if (_vm->_game.id == GID_CMI || (_vm->_game.id == GID_DIG && !(_vm->_game.features & GF_DEMO))) {
		// COMI (full & demo), DIG (full)
		_digiSndMode |= mode;

		if (_vm->_game.id == GID_DIG)
			resetSpeechTimer();

		return;
	} else if (_vm->_game.id == GID_DIG && (_vm->_game.features & GF_DEMO) &&
			   _vm->_voiceMode != 2) {
		_digiSndMode |= mode;

		char filename[30];
		char roomname[10];
		int roomNumber = offset;
		int fileNumber = length;

		if (roomNumber == 1)
			Common::strlcpy(roomname, "logo", sizeof(roomname));
		else if (roomNumber == 15)
			Common::strlcpy(roomname, "canyon", sizeof(roomname));
		else if (roomNumber == 17)
			Common::strlcpy(roomname, "pig", sizeof(roomname));
		else if (roomNumber == 18)
			Common::strlcpy(roomname, "derelict", sizeof(roomname));
		else if (roomNumber == 19)
			Common::strlcpy(roomname, "wreck", sizeof(roomname));
		else if (roomNumber == 20)
			Common::strlcpy(roomname, "grave", sizeof(roomname));
		else if (roomNumber == 23)
			Common::strlcpy(roomname, "nexus", sizeof(roomname));
		else if (roomNumber == 79)
			Common::strlcpy(roomname, "newton", sizeof(roomname));
		else {
			warning("startTalkSound: dig demo: unknown room number: %d", roomNumber);
			return;
		}

		file.reset(new ScummFile(_vm));
		if (!file)
			error("startTalkSound: Out of memory");

		Common::sprintf_s(filename, "audio/%s.%u/%u.voc", roomname, roomNumber, fileNumber);
		if (!_vm->openFile(*file, filename)) {
			Common::sprintf_s(filename, "audio/%s_%u/%u.voc", roomname, roomNumber, fileNumber);
			_vm->openFile(*file, filename);
		}

		if (!file->isOpen()) {
			Common::sprintf_s(filename, "%u.%u.voc", roomNumber, fileNumber);
			_vm->openFile(*file, filename);
		}

		if (!file->isOpen()) {
			warning("startTalkSound: dig demo: voc file not found");
			return;
		}

		file->seek(0, SEEK_END);
#if defined(ENABLE_SCUMM_7_8)
		int fileSize = file->pos();
		_vm->_imuseDigital->startVoice(filename, file.release(), 0, fileSize);
#endif
		return;
	} else if (_vm->_game.id == GID_FT) {
		int totalOffset, soundSize, fileSize, headerTag, vctlBlockSize;

		if (_vm->_voiceMode != 2) {
			// Don't read the index of the PAK file, as we cached the
			// location and size of monster.sou, which is the only file
			// we need at this point. We do this to avoid reading the
			// whole PAK file index for each speech sound.
			ScummFile *scummFile = _vm->instantiateScummFile(false);
			if (_vm->_game.features & GF_DOUBLEFINE_PAK) {
				PAKFile tmpPak;
				tmpPak.start = _cachedSfxLocationInPak;
				tmpPak.len = _cachedSfxLengthInPak;
				dynamic_cast<ScummPAKFile *>(scummFile)->setPAKFileIndex(_sfxFilename, tmpPak);
			}
			file.reset(scummFile);

			if (!file)
				error("startTalkSound: Out of memory");

			if (!_vm->openFile(*file, Common::Path(_sfxFilename))) {
				warning("startTalkSound: could not open sfx file %s", _sfxFilename.c_str());
				return;
			}

			// File format for each speech file:
			// - VCTL block; containing:
			//   - "VCTL" string (4 bytes);
			//   - The size of said block (4 bytes);
			//   - A variable number of mouth sync timestamps (2 bytes each);
			//     subtracting 8 from the size of the block, and dividing by 2,
			//     yields the number of mouth syncs available for the current file.
			//     Curiously, the number of syncs is already given as an argument
			//     to this function, coming from the control codes of the current
			//     dialog string.
			//
			// - VTLK block; containing:
			//   - "VTLK" string (4 bytes);
			//   - The size of said block (4 bytes);
			//   - A full VOC file (complete with each header).
			//
			// The engine also allows for a VOC file without a VTLK header.

			file->setEnc(_sfxFileEncByte);
			file->seek(offset + 4 + 4, SEEK_SET); // Skip "VCTL" and the block size
			vctlBlockSize = length;

			if (vctlBlockSize > 8) {
				num = (vctlBlockSize - 8) >> 1;
			}

			if (num >= 50)
				num = 48;

			assert(num + 1 < (int)ARRAYSIZE(_mouthSyncTimes));
			for (i = 0; i < num; i++)
				_mouthSyncTimes[i] = file->readUint16BE();

			_mouthSyncTimes[i] = 0xFFFF;
			_digiSndMode |= mode;
			resetSpeechTimer();
			_mouthSyncMode = true;

			totalOffset = offset + vctlBlockSize;
			file->seek(totalOffset, SEEK_SET);
			headerTag = file->readUint32BE();
			soundSize = file->readUint32BE() - 8;
			fileSize = soundSize;
			if (headerTag == MKTAG('C','r','e','a')) {
				file->seek(totalOffset + 27, SEEK_SET);
				fileSize = 31;
				fileSize += file->readUint32LE() >> 8;
#if defined(ENABLE_SCUMM_7_8)
				_vm->_imuseDigital->startVoice(_sfxFilename.c_str(), file.release(), totalOffset, fileSize);
#else
				(void)fileSize;
#endif
			} else if (headerTag == MKTAG('V','T','L','K')) {
#if defined(ENABLE_SCUMM_7_8)
				_vm->_imuseDigital->startVoice(_sfxFilename.c_str(), file.release(), totalOffset + 8, soundSize);
#endif
			} else {
				file.release()->close();
			}
		}
		return;
	} else if ((_vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2) && (_vm->_game.features & GF_DOUBLEFINE_PAK) && _useRemasteredAudio) {
		// MI1 and MI2 SE
		if (_soundSE && !_soundsPaused && _mixer->isReady()) {
			Audio::AudioStream *input = _soundSE->getAudioStream(
				_currentMISpeechIndex,
				mode == DIGI_SND_MODE_SFX ? kSoundSETypeSFX : kSoundSETypeSpeech);

			_digiSndMode |= mode;

			if (input) {
				if (mode == DIGI_SND_MODE_SFX)
					_mixer->playStream(Audio::Mixer::kSFXSoundType, handle, input, id);
				else
					_mixer->playStream(Audio::Mixer::kSpeechSoundType, handle, input, id);
			}

			_vm->_currentSpeechIndexMI++;
		}

		return;
	} else {
		// This has been verified for INDY4, DOTT and SAM
		if (_vm->_voiceMode == 2 && _vm->_game.version <= 6)
			return;

		if (_sfxFilename.empty()) {
			warning("startTalkSound: SFX file not found");
			return;
		}

		// Some games frequently assume that starting one sound effect will
		// automatically stop any other that may be playing at that time. So
		// that is what we do here, but we make an exception for speech.

		if (mode == DIGI_SND_MODE_SFX && (_vm->_game.id == GID_TENTACLE || _vm->_game.id == GID_SAMNMAX)) {
			id = 777777 + _queuedSfxChannel;
			_mixer->stopID(id);
		}

		if (length > 8) {
			num = (length - 8) >> 1;
		}

		if (_offsetTable != nullptr) {
			MP3OffsetTable *result = nullptr, key;

			key.org_offset = offset;
			result = (MP3OffsetTable *)bsearch(&key, _offsetTable, _numSoundEffects,
													sizeof(MP3OffsetTable), compareMP3OffsetTable);

			if (result == nullptr) {
				warning("startTalkSound: did not find sound at offset %d", offset);
				return;
			}
			if (2 * num != result->num_tags) {
				warning("startTalkSound: number of tags do not match (%d - %d)", length,
								result->num_tags);
				num = result->num_tags;
			}
			offset = result->new_offset;
#if defined(USE_FLAC) || defined(USE_VORBIS) || defined(USE_MAD)
			size = result->compressed_size;
#endif
		} else {
			offset += 8;
		}

		// Don't read the index of the PAK file, as we cached the
		// location and size of monster.sou, which is the only file
		// we need at this point. We do this to avoid reading the
		// whole PAK file index for each speech sound.
		ScummFile *scummFile = _vm->instantiateScummFile(false);
		if (_vm->_game.features & GF_DOUBLEFINE_PAK) {
			PAKFile tmpPak;
			tmpPak.start = _cachedSfxLocationInPak;
			tmpPak.len = _cachedSfxLengthInPak;
			dynamic_cast<ScummPAKFile *>(scummFile)->setPAKFileIndex(_sfxFilename, tmpPak);
		}
		file.reset(scummFile);
		
		if (!file)
			error("startTalkSound: Out of memory");

		if (!_vm->openFile(*file, Common::Path(_sfxFilename))) {
			warning("startTalkSound: could not open sfx file %s", _sfxFilename.c_str());
			return;
		}

		file->setEnc(_sfxFileEncByte);
		file->seek(offset, SEEK_SET);

		assert(num + 1 < (int)ARRAYSIZE(_mouthSyncTimes));
		for (i = 0; i < num; i++)
			_mouthSyncTimes[i] = file->readUint16BE();

		// Adjust offset to account for the mouth sync times. It is noteworthy
		// that we do not adjust the size here for compressed streams, since
		// they only set size to the size of the compressed sound data.
		offset += num * 2;
		// TODO: In case we ever set up the size for VOC streams, we should
		// really check whether the size contains the _mouthSyncTimes.
		//if (_soundMode == kVOCMode)
		//	size -= num * 2;

		_mouthSyncTimes[i] = 0xFFFF;
		_digiSndMode |= mode;
		resetSpeechTimer();
		_mouthSyncMode = true;
	}

	if (!_soundsPaused && _mixer->isReady()) {
		Audio::AudioStream *input = nullptr;

		switch (_soundMode) {
		case kMP3Mode:
#ifdef USE_MAD
			{
			assert(size > 0);
			input = Audio::makeMP3Stream(new Common::SeekableSubReadStream(file.release(), offset, offset + size, DisposeAfterUse::YES), DisposeAfterUse::YES);
			}
#endif
			break;
		case kVorbisMode:
#ifdef USE_VORBIS
			{
			assert(size > 0);
			input = Audio::makeVorbisStream(new Common::SeekableSubReadStream(file.release(), offset, offset + size, DisposeAfterUse::YES), DisposeAfterUse::YES);
			}
#endif
			break;
		case kFLACMode:
#ifdef USE_FLAC
			{
			assert(size > 0);
			input = Audio::makeFLACStream(new Common::SeekableSubReadStream(file.release(), offset, offset + size, DisposeAfterUse::YES), DisposeAfterUse::YES);
			}
#endif
			break;
		default:
			// WORKAROUND: Check for original Indy4 MONSTER.SOU bug
			// The speech sample at VCTL offset 0x76ccbca ("Hey you!") which is used
			// when Indy gets caught on the German submarine seems to not be a VOC
			// but raw PCM s16be at (this is a guess) 44.1 kHz with a bogus VOC header.
			// To work around this we skip the VOC header and decode the raw PCM data.
			// Fixes Trac#10559
			//
			// We use `kEnhGameBreakingBugFixes` because the original bug causes some terrible
			// harsh white noise which could damage one's ears when wearing headphones.
			if (mode == 2 && _vm->_game.id == GID_INDY4 && offset == 0x76ccbd4 && _vm->enhancementEnabled(kEnhGameBreakingBugFixes))
				input = checkForBrokenIndy4Sample(file.release(), offset);

			if (!input && _soundSE && _useRemasteredAudio) {
				input = _soundSE->getAudioStream(
					origOffset,
					mode == DIGI_SND_MODE_SFX ? kSoundSETypeSFX : kSoundSETypeSpeech
				);
			}

			if (!input) {
				input = Audio::makeVOCStream(
					file.release(),
					Audio::FLAG_UNSIGNED,
					DisposeAfterUse::YES
				);
			}

			break;
		}

		if (!input) {
			warning("startSfxSound failed to load sound");
			return;
		}

		if (!_vm->_imuseDigital) {
			if (mode == DIGI_SND_MODE_SFX) {
				_mixer->playStream(Audio::Mixer::kSFXSoundType, handle, input, id);
			} else {
				_mixer->playStream(Audio::Mixer::kSpeechSoundType, handle, input, id);
			}
		}
	}
}

void Sound::stopTalkSound() {
	if (_digiSndMode & DIGI_SND_MODE_TALKIE) {
		if (_vm->_imuseDigital) {
#ifdef ENABLE_SCUMM_7_8
			_vm->_imuseDigital->stopSound(kTalkSoundID);
#endif
		} else if (_vm->_game.heversion >= 60) {
			stopSound(1);
		} else {
			_mixer->stopHandle(*_talkChannelHandle);
		}
		_digiSndMode &= ~DIGI_SND_MODE_TALKIE;
	}
}

bool Sound::isMouthSyncOff(uint pos) {
	uint j;
	bool val = true;
	uint16 *ms = _mouthSyncTimes;
	uint delay = (_vm->_game.version == 6) ? 10 : 0;

	if (_vm->_game.id == GID_DIG && !(_vm->_game.features & GF_DEMO)) {
		pos = 1000 * pos / 60;
		val = false;
	}

	_endOfMouthSync = false;
	do {
		val = !val;
		j = *ms++;
		if (j == 0xFFFF) {
			_endOfMouthSync = true;
			break;
		}
	} while (pos + delay > j);

	if (_vm->_game.version < 7) {
		return val;
	} else {
		return (j != 0xFFFF) ? val : false;
	}
}

int Sound::isSoundRunning(int sound) const {
#ifdef ENABLE_SCUMM_7_8
	if (_vm->_imuseDigital) {
		return (_vm->_imuseDigital->isSoundRunning(sound) != 0);
	}
#endif

	if (sound == _soundCD->_currentCDSound)
		return _soundCD->pollCD();

	if (_mixer->isSoundIDActive(sound))
		return 1;

	if (isSoundInQueue(sound))
		return 1;

	if (sound > _vm->_numSounds || !_vm->_res->isResourceLoaded(rtSound, sound))
		return 0;

	if (_vm->_musicEngine)
		return _vm->_musicEngine->getSoundStatus(sound);

	return 0;
}

/**
 * Check whether the sound resource with the specified ID is still
 * used. This is invoked by ScummEngine::isResourceInUse(), to determine
 * which resources can be expired from memory.
 * Technically, this works very similar to isSoundRunning(), however it
 * calls IMuse::get_sound_active() instead of IMuse::getSoundStatus().
 * The difference between those two is in how they treat sounds which
 * are being faded out: get_sound_active() returns true even when the
 * sound is being faded out, while getSoundStatus() returns false in
 * that case.
 *
 * Another difference is that isSoundRunning() checks if sound is greater
 * than _numSounds before checking if the resource is loaded. That check is
 * only for non-HE games. In HE games, a number higher than _numSounds
 * represents a (streamed) music track. HE games have their own implementation
 * of isSoundRunning(), while isSoundInUse() is used by all.
 */
bool Sound::isSoundInUse(int sound) const {

#ifdef ENABLE_SCUMM_7_8
	if (_vm->_imuseDigital)
		return (_vm->_imuseDigital->isSoundRunning(sound) != 0);
#endif

	if (sound == _soundCD->_currentCDSound)
		return _soundCD->pollCD() != 0;

	if (_mixer->isSoundIDActive(sound))
		return true;

	if (isSoundInQueue(sound))
		return true;

	if (!_vm->_res->isResourceLoaded(rtSound, sound))
		return false;

	if (_vm->_imuse)
		return _vm->_imuse->get_sound_active(sound);
	else if (_vm->_musicEngine)
		return _vm->_musicEngine->getSoundStatus(sound);

	return false;
}

bool Sound::isSoundInQueue(int sound) const {
	int i, num;

	i = _soundQueuePos;
	while (i--) {
		if (_soundQueue[i].sound == sound)
			return true;
	}

	i = 0;
	while (i < _midiQueuePos) {
		num = _midiQueue[i++];

		if (num > 0) {
			if (_midiQueue[i + 0] == 0x10F && _midiQueue[i + 1] == 8 && _midiQueue[i + 2] == sound)
				return true;
			i += num;
		}
	}
	return false;
}

void Sound::stopSound(int sound) {
	int i;

	_soundCD->stopCDSound(sound);

	if (_vm->_game.version < 7)
		_mixer->stopID(sound);

	if (_vm->_musicEngine)
		_vm->_musicEngine->stopSound(sound);

	for (i = 0; i < ARRAYSIZE(_soundQueue); i++) {
		if (_soundQueue[i].sound == sound) {
			_soundQueue[i].sound = 0;
			_soundQueue[i].offset = 0;
			_soundQueue[i].channel = 0;
			_soundQueue[i].flags = 0;
			_soundQueue[i].freq = 0;
			_soundQueue[i].pan = 0;
			_soundQueue[i].vol = 0;
		}
	}
}

void Sound::stopAllSounds() {
	_soundCD->stopAllCDSounds();

	// Clear the (secondary) sound queue
	_lastSound = 0;
	_soundQueuePos = 0;
	memset(_soundQueue, 0, sizeof(_soundQueue));

	if (_vm->_musicEngine) {
		_vm->_musicEngine->stopAllSounds();
	}

	// Stop all SFX
	if (!_vm->_imuseDigital) {
		_mixer->stopAll();
	}
}

void Sound::soundKludge(int *list, int num) {
	int i;

#ifdef ENABLE_SCUMM_7_8
	if (_vm->_imuseDigital) {
		_vm->_imuseDigital->parseScriptCmds(list[0],  list[1],  list[2],  list[3],  list[4],
									  list[5],  list[6],  list[7],  list[8],  list[9],
									  list[10], list[11], list[12], list[13], list[14], list[15]);
		return;
	}
#endif

	if (list[0] == -1) {
		processSound();
	} else {
		_midiQueue[_midiQueuePos++] = num;

		for (i = 0; i < num; i++) {
			_midiQueue[_midiQueuePos++] = list[i];
		}
	}
}

void Sound::talkSound(uint32 offset, uint32 length, int mode, int channel) {
	if (mode == DIGI_SND_MODE_SFX) {
		_queuedSfxOffset = offset;
		_queuedSfxLen = length;
		_queuedSfxChannel = channel;
	} else {
		_queuedTalkieOffset = offset;
		_queuedTalkieLen = length;
	}

	_queuedSoundMode |= mode;
}

void Sound::setupSound() {
	setupSfxFile();

	if (_vm->_game.id == GID_FT) {
		_vm->VAR(_vm->VAR_VOICE_BUNDLE_LOADED) = _sfxFilename.empty() ? 0 : 1;
	}
}

void Sound::pauseSounds(bool pause) {
	if (_vm->_imuse)
		_vm->_imuse->pause(pause);

	_soundsPaused = _soundCD->_soundsPaused = pause;

#ifdef ENABLE_SCUMM_7_8
	if (_vm->_imuseDigital) {
		_vm->_imuseDigital->pause(pause);
	}
#endif

	_mixer->pauseAll(pause);

	_soundCD->pauseCDSounds(pause);
}

bool Sound::isSfxFileCompressed() {
	return !(_soundMode == kVOCMode);
}

bool Sound::hasSfxFile() const
{
	return !_sfxFilename.empty();
}

ScummFile *Sound::restoreDiMUSESpeechFile(const char *fileName) {
	Common::ScopedPtr<ScummFile> file;
	file.reset(_vm->instantiateScummFile());
	if (!_vm->openFile(*file, fileName)) {
		return NULL;
	}

	return file.release();
}

/* The approach used by the full version of The Dig for obtaining mouth syncs is a bit weird:
 * they are stored in a text marker found inside the DiMUSE map for each speech file, and when
 * said engine reaches said marker, the function below is triggered.
 *
 * A good reason why this is the way it's done, is that in The Dig the whole speech file,
 * including its map (and consequently, the text marker), is compressed with the same codec as
 * sound data; this prevents us from getting the mouth syncs before the file has started playing.
 * Also, although I can't confirm this, there might be more than one sync marker in a single
 * speech file, so let's just be safe and follow what the original does.
 */
void Sound::extractSyncsFromDiMUSEMarker(const char *marker) {
	int syncIdx = 0;

	while (marker[syncIdx * 8]) {
		_mouthSyncTimes[syncIdx] = (uint16)atoi(&marker[syncIdx * 8]);
		syncIdx++;
	}

	_mouthSyncTimes[syncIdx] = 0xFFFF;
}

void Sound::setupSfxFile() {
	struct SoundFileExtensions {
		const char *ext;
		SoundMode mode;
	};

	static const SoundFileExtensions extensions[] = {
		{ "sou", kVOCMode },
#ifdef USE_FLAC
		{ "sof", kFLACMode },
#endif
#ifdef USE_VORBIS
		{ "sog", kVorbisMode },
#endif
#ifdef USE_MAD
		{ "so3", kMP3Mode },
#endif
		{ nullptr, kVOCMode }
	};

	ScummFile *file = _vm->instantiateScummFile();
	_offsetTable = nullptr;
	_sfxFileEncByte = 0;
	_sfxFilename.clear();

	/* Try opening the file <baseName>.sou first, e.g. tentacle.sou.
	 * That way, you can keep .sou files for multiple games in the
	 * same directory */

	Common::String basename[2];
	Common::Path tmp;

	const char *ptr = strchr(_vm->_filenamePattern.pattern, '.');
	if (ptr) {
		basename[0] = Common::String(_vm->_filenamePattern.pattern, ptr - _vm->_filenamePattern.pattern + 1);
	} else {
		basename[0] = _vm->_filenamePattern.pattern;
		basename[0] += '.';
	}
	basename[1] = "monster.";

	if (_vm->_game.heversion >= 60) {
		if ((_vm->_game.heversion <= 62 && _vm->_game.platform == Common::kPlatformMacintosh) || (_vm->_game.heversion >= 70)) {
			tmp = _vm->generateFilename(-2);
		} else {
			tmp = basename[0];
			tmp.appendInPlace("tlk");
		}

		if (file->open(Common::Path(tmp)))
			_sfxFilename = tmp.toString('/');

		if (_vm->_game.heversion <= 74)
			_sfxFileEncByte = 0x69;

		_soundMode = kVOCMode;
	} else {
		for (uint j = 0; j < 2 && !file->isOpen(); ++j) {
			for (int i = 0; extensions[i].ext; ++i) {
				tmp = basename[j];
				tmp.appendInPlace(extensions[i].ext);
				if (_vm->openFile(*file, tmp)) {
					_soundMode = extensions[i].mode;
					_sfxFilename = tmp.toString('/');

					// Cache SFX file location for classic game versions
					// packed within remastered ones
					if (_vm->_game.features & GF_DOUBLEFINE_PAK) {
						PAKFile *tmpPak = dynamic_cast<ScummPAKFile *>(file)->getPAKFileIndex(_sfxFilename);
						_cachedSfxLocationInPak = tmpPak->start;
						_cachedSfxLengthInPak = tmpPak->len;
					}
					break;
				}
			}
		}
	}

	if (_soundMode != kVOCMode) {
		/* Now load the 'offset' index in memory to be able to find the MP3 data

		   The format of the .SO3 file is easy :
		   - number of bytes of the 'index' part
		   - N times the following fields (4 bytes each) :
		   + offset in the original sound file
		   + offset of the MP3 data in the .SO3 file WITHOUT taking into account
		   the index field and the 'size' field
		   + the number of 'tags'
		   + the size of the MP3 data
		   - and then N times :
		   + the tags
		   + the MP3 data
		 */
		int size, compressed_offset;
		MP3OffsetTable *cur;
		compressed_offset = file->readUint32BE();
		_offsetTable = (MP3OffsetTable *) malloc(compressed_offset);
		_numSoundEffects = compressed_offset / 16;

		size = compressed_offset;
		cur = _offsetTable;
		while (size > 0) {
			cur->org_offset = file->readUint32BE();
			cur->new_offset = file->readUint32BE() + compressed_offset + 4; /* The + 4 is to take into accound the 'size' field */
			cur->num_tags = file->readUint32BE();
			cur->compressed_size = file->readUint32BE();
			size -= 4 * 4;
			cur++;
		}
	}

	file->close();
	delete file;
}

bool Sound::isSfxFinished() const {
	return !_mixer->hasActiveChannelOfType(Audio::Mixer::kSFXSoundType);
}

void Sound::incrementSpeechTimer() {
	if (!_soundsPaused)
		_curSoundPos++;
}

void Sound::resetSpeechTimer() {
	_curSoundPos = 0;
}

static void speechTimerHandler(void *refCon) {
	Sound *snd = (Sound *)refCon;
	if ((snd->_speechTimerMod++ & 3) == 0) {
		snd->incrementSpeechTimer();
	}
}

void Sound::startSpeechTimer() {
	_vm->getTimerManager()->installTimerProc(&speechTimerHandler, 1000000 / _vm->getTimerFrequency(), this, "scummSpeechTimer");
}

void Sound::stopSpeechTimer() {
	_vm->getTimerManager()->removeTimerProc(&speechTimerHandler);
}

bool Sound::speechIsPlaying() {
	return _mixer->isSoundHandleActive(*_talkChannelHandle);
}

void Sound::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint16LE(_soundCD->_currentCDSound, VER(35));
	s.syncAsSint16LE(_currentMusic, VER(35));
}

void Sound::restoreAfterLoad() {
	_soundCD->restoreAfterLoad();
}

bool Sound::isAudioDisabled() {
#ifdef ENABLE_SCUMM_7_8
	if (_vm->_game.version > 6) {
		return _vm->_imuseDigital->isEngineDisabled();
	}
#endif

	return false;
}

#pragma mark -
#pragma mark --- Sound resource handling ---
#pragma mark -

/*
 * TODO: The way we handle sound/music resources really is one huge hack.
 * We probably should reconsider how we do this, and maybe come up with a
 * better/cleaner solution. Even if we keep the existing code, it really
 * could stand a thorough cleanup!
 */

int ScummEngine::readSoundResource(ResId idx) {
	uint32 pos, total_size, size, tag, basetag, max_total_size;
	int pri, best_pri;
	uint32 best_size = 0, best_offs = 0;
	byte *ptr;

	debugC(DEBUG_RESOURCE, "readSoundResource(%d)", idx);

	pos = 0;

	_fileHandle->readUint32LE();
	max_total_size = _fileHandle->readUint32BE() - 8;
	basetag = _fileHandle->readUint32BE();
	total_size = _fileHandle->readUint32BE();

	debugC(DEBUG_RESOURCE, "  basetag: %s, total_size=%d", tag2str(basetag), total_size);

	switch (basetag) {
	case MKTAG('M','I','D','I'):
	case MKTAG('i','M','U','S'):
		if (_sound->_musicType != MDT_PCSPK && _sound->_musicType != MDT_PCJR) {
			_fileHandle->seek(-8, SEEK_CUR);
			_fileHandle->read(_res->createResource(rtSound, idx, total_size + 8), total_size + 8);
			return 1;
		}
		break;
	case MKTAG('S','O','U',' '):
		best_pri = -1;
		while (pos < total_size) {
			tag = _fileHandle->readUint32BE();
			size = _fileHandle->readUint32BE() + 8;
			pos += size;

			pri = -1;

			switch (tag) {
			case MKTAG('T','O','W','S'):
				pri = 16;
				break;
			case MKTAG('S','B','L',' '):
				pri = 15;
				break;
			case MKTAG('A','D','L',' '):
				if (_sound->_musicType == MDT_ADLIB || _sound->_musicType == MDT_TOWNS)
					pri = 10;
				else if (_sound->_musicType != MDT_MACINTOSH && _sound->_musicType != MDT_AMIGA)
					pri = 1;
				break;
			case MKTAG('A','M','I',' '):
				pri = 3;
				break;
			case MKTAG('R','O','L',' '):
				// Some of the Mac MI2 music only exists as Roland tracks. The
				// original interpreter doesn't play them. I don't think there
				// is any similarly missing FoA music.
				if (_game.id == GID_MONKEY2 && _game.platform == Common::kPlatformMacintosh && !enhancementEnabled(kEnhAudioChanges))
					break;

				pri = 3;
				if (_native_mt32)
					pri = 5;
				break;
			case MKTAG('G','M','D',' '):
				pri = 4;
				break;
			case MKTAG('M','A','C',' '):	// Occurs in Mac MI2, FOA
				pri = 2;
				break;
			case MKTAG('S','P','K',' '):
				if (_sound->_musicType == MDT_PCSPK || _sound->_musicType == MDT_PCJR)
					pri = 11;
				break;
			default:
				break;
			}

			// We only allow SPK resources for PC Speaker and PCJr here
			// since other resource would sound horribly with their output
			// drivers.
			if ((_sound->_musicType == MDT_PCSPK || _sound->_musicType == MDT_PCJR) && pri != 11)
				pri = -1;

			// We only allow ADL, SBL and TOWS resources when AdLib
			// or FM-Towns is used as primary audio output. This fixes some
			// odd sounds when Indy and Sophia leave Atlantis with the
			// submarine in Indy4. (Easy to check with bootparam 4061 in
			// the CD version). It seems the game only contains a ROL resource
			// for sound id 60. Formerly we tried to play that via the AdLib
			// or FM-Towns audio driver resulting in strange noises. Now we
			// behave like the original did.
			// We make an exception for Macintosh, which uses priority 2 for
			// its sound resources, and Amiga games, which feature only ROL
			// resources, since we are a doing Midi -> AdLib conversion for
			// these.
			if ((_sound->_musicType == MDT_ADLIB || _sound->_musicType == MDT_TOWNS) && pri != 16
				&& pri != 15 && pri != 10 && pri != 2 && _game.platform != Common::kPlatformAmiga)
				pri = -1;

			debugC(DEBUG_RESOURCE, "    tag: %s, total_size=%d, pri=%d", tag2str(tag), size, pri);

			if (pri > best_pri) {
				best_pri = pri;
				best_size = size;
				best_offs = _fileHandle->pos();
			}

			_fileHandle->seek(size - 8, SEEK_CUR);
		}

		if (best_pri != -1) {
			_fileHandle->seek(best_offs - 8, SEEK_SET);
			ptr = _res->createResource(rtSound, idx, best_size);
			_fileHandle->read(ptr, best_size);
			//dumpResource("sound-", idx, ptr);
			return 1;
		} else if (_game.platform == Common::kPlatformAmiga || _game.platform == Common::kPlatformMacintosh) {
			// This isn't a bug. These versions simply don't have all the sounds. Very noticeable for Amiga (which
			// doesn't even have the Woodtick song), but also for Macintosh in a few cases.
			debugC(DEBUG_RESOURCE, "No appropriate sound data for target platform found in resource %d", idx);
			return 0;
		}
		break;
	case MKTAG('M','a','c','0'):
		_fileHandle->seek(-12, SEEK_CUR);
		total_size = _fileHandle->readUint32BE() - 8;
		ptr = _res->createResource(rtSound, idx, total_size);
		_fileHandle->read(ptr, total_size);
		//dumpResource("sound-", idx, ptr);
		return 1;

	case MKTAG('M','a','c','1'):
	case MKTAG('R','I','F','F'):
	case MKTAG('T','A','L','K'):
	case MKTAG('D','I','G','I'):
	case MKTAG('C','r','e','a'):
	case 0x460e200d:	// WORKAROUND bug #2221
		// Some HE games take WAV files and put a WSOU header around them
		if (_game.heversion > 0 && basetag == MKTAG('R','I','F','F')) {
			_fileHandle->seek(-4, SEEK_CUR);

			// The chunksize field in a RIFF header is a LE field
			total_size = _fileHandle->readUint32LE() + 8;

			// Make space for the WSOU tag and for the size field
			ptr = _res->createResource(rtSound, idx, total_size + 8);

			((uint32 *)ptr)[0] = TO_BE_32(MKTAG('W', 'S', 'O', 'U'));
			((uint32 *)ptr)[1] = TO_BE_32(total_size + 8);

			// Move the ptr forward for the actual data allocation,
			// so that our new header doesn't get rewritten
			ptr += 8;
			_fileHandle->seek(-8, SEEK_CUR);
			_fileHandle->read(ptr, total_size);
		} else {
			_fileHandle->seek(-12, SEEK_CUR);
			total_size = _fileHandle->readUint32BE();
			ptr = _res->createResource(rtSound, idx, total_size);
			_fileHandle->read(ptr, total_size - 8);
			//dumpResource("sound-", idx, ptr);
		}

		return 1;

	case MKTAG('H','S','H','D'):
		// HE sound type without SOUN header
		_fileHandle->seek(-16, SEEK_CUR);
		total_size = max_total_size + 8;
		ptr = _res->createResource(rtSound, idx, total_size);
		_fileHandle->read(ptr, total_size);
		//dumpResource("sound-", idx, ptr);
		return 1;

	case MKTAG('F','M','U','S'): {
		// Used in 3DO version of puttputt joins the parade and probably others
		// Specifies a separate file to be used for music from what I gather.
		int tmpsize;
		Common::File dmuFile;
		char buffer[128];
		debugC(DEBUG_SOUND, "Found base tag FMUS in sound %d, size %d", idx, total_size);
		debugC(DEBUG_SOUND, "It was at position %d", (int)_fileHandle->pos());

		_fileHandle->seek(4, SEEK_CUR);
		// HSHD size
		tmpsize = _fileHandle->readUint32BE();
		// skip to size part of the SDAT block
		_fileHandle->seek(tmpsize - 4, SEEK_CUR);
		// SDAT size
		tmpsize = _fileHandle->readUint32BE();

		// SDAT contains name of file we want
		_fileHandle->read(buffer, MIN(128, tmpsize - 8));
		// files seem to be 11 chars (8.3)
		char *p = (char *)memchr(buffer, '.', 12);
		if (!p) p = &buffer[8];
		Common::strlcpy(p, ".dmu", sizeof(buffer) - (p - buffer));
		debugC(DEBUG_SOUND, "FMUS file %s", buffer);

		if (!dmuFile.open(buffer)) {
			error("Can't open music file %s", buffer);
			_res->_types[rtSound][idx]._roomoffs = RES_INVALID_OFFSET;
			return 0;
		}
		dmuFile.seek(4, SEEK_SET);
		total_size = dmuFile.readUint32BE();
		debugC(DEBUG_SOUND, "dmu file size %d", total_size);
		dmuFile.seek(-8, SEEK_CUR);
		dmuFile.read(_res->createResource(rtSound, idx, total_size), total_size);
		dmuFile.close();
		}
		return 1;

	default:
		if (SWAP_BYTES_32(basetag) == max_total_size) {
			_fileHandle->seek(-12, SEEK_CUR);
			total_size = _fileHandle->readUint32BE();
			_fileHandle->seek(-8, SEEK_CUR);
			ptr = _res->createResource(rtSound, idx, total_size);
			_fileHandle->read(ptr, total_size);
			//dumpResource("sound-", idx, ptr);
			return 1;
		}
	}

	if (total_size)
		warning("Unrecognized base tag '%c%c%c%c' in sound %d", (basetag >> 24) & 0xFF, (basetag >> 16) & 0xFF, (basetag >> 8) & 0xFF, basetag & 0xFF, idx);
	_res->_types[rtSound][idx]._roomoffs = RES_INVALID_OFFSET;
	return 0;
}

// AdLib MIDI-SYSEX to set MIDI instruments for small header games.
static const byte ADLIB_INSTR_MIDI_HACK[95] = {
	0x00, 0xf0, 0x14, 0x7d, 0x00,  // sysex 00: part on/off
	0x00, 0x00, 0x03,              // part/channel  (offset  5)
	0x00, 0x00, 0x07, 0x0f, 0x00, 0x00, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0xf7,
	0x00, 0xf0, 0x41, 0x7d, 0x10,  // sysex 16: set instrument
	0x00, 0x01,                    // part/channel  (offset 28)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xf7,
	0x00, 0xb0, 0x07, 0x64        // Controller 7 = 100 (offset 92)
};

static const byte map_param[7] = {
	0, 2, 3, 4, 8, 9, 0,
};

static const byte freq2note[128] = {
	/*128*/	6, 6, 6, 6,
	/*132*/ 7, 7, 7, 7, 7, 7, 7,
	/*139*/ 8, 8, 8, 8, 8, 8, 8, 8, 8,
	/*148*/ 9, 9, 9, 9, 9, 9, 9, 9, 9,
	/*157*/ 10, 10, 10, 10, 10, 10, 10, 10, 10,
	/*166*/ 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	/*176*/ 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	/*186*/ 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	/*197*/ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	/*209*/ 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	/*222*/ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	/*235*/ 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
	/*249*/ 18, 18, 18, 18, 18, 18, 18
};

static const uint16 num_steps_table[] = {
	1, 2, 4, 5,
	6, 7, 8, 9,
	10, 12, 14, 16,
	18, 21, 24, 30,
	36, 50, 64, 82,
	100, 136, 160, 192,
	240, 276, 340, 460,
	600, 860, 1200, 1600
};

static int convert_extraflags(byte * ptr, byte * src_ptr) {
	int flags = src_ptr[0];

	int t1, t2, t3, t4, time;
	int v1, v2, v3;

	if (!(flags & 0x80))
		return -1;

	t1 = (src_ptr[1] & 0xf0) >> 3;
	t2 = (src_ptr[2] & 0xf0) >> 3;
	t3 = (src_ptr[3] & 0xf0) >> 3 | (flags & 0x40 ? 0x80 : 0);
	t4 = (src_ptr[3] & 0x0f) << 1;
	v1 = (src_ptr[1] & 0x0f);
	v2 = (src_ptr[2] & 0x0f);
	v3 = 31;
	if ((flags & 0x7) == 0) {
	  v1 = v1 + 31 + 8;
	  v2 = v2 + 31 + 8;
	} else {
	  v1 = v1 * 2 + 31;
	  v2 = v2 * 2 + 31;
	}

	/* flags a */
	if ((flags & 0x7) == 6)
		ptr[0] = 0;
	else {
		ptr[0] = (flags >> 4) & 0xb;
		ptr[1] = map_param[flags & 0x7];
	}

	/* extra a */
	ptr[2] = 0;
	ptr[3] = 0;
	ptr[4] = t1 >> 4;
	ptr[5] = t1 & 0xf;
	ptr[6] = v1 >> 4;
	ptr[7] = v1 & 0xf;
	ptr[8] = t2 >> 4;
	ptr[9] = t2 & 0xf;
	ptr[10] = v2 >> 4;
	ptr[11] = v2 & 0xf;
	ptr[12] = t3 >> 4;
	ptr[13] = t3 & 0xf;
	ptr[14] = t4 >> 4;
	ptr[15] = t4 & 0xf;
	ptr[16] = v3 >> 4;
	ptr[17] = v3 & 0xf;

	time = num_steps_table[t1] + num_steps_table[t2]
		+ num_steps_table[t3 & 0x7f] + num_steps_table[t4];
	if (flags & 0x20) {
		int playtime = ((src_ptr[4] >> 4) & 0xf) * 118 +
			(src_ptr[4] & 0xf) * 8;
		if (playtime > time)
			time = playtime;
	}
	/*
	time = ((src_ptr[4] >> 4) & 0xf) * 118 +
		(src_ptr[4] & 0xf) * 8;
	*/
	return time;
}

#define kMIDIHeaderSize		46
static byte *writeMIDIHeader(byte *ptr, const char *type, int ppqn, int total_size) {
	uint32 dw = TO_BE_32(total_size);

	memcpy(ptr, type, 4); ptr += 4;
	memcpy(ptr, &dw, 4); ptr += 4;
	memcpy(ptr, "MDhd", 4); ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 8;
	ptr += 4;
	memset(ptr, 0, 8); ptr += 8;
	memcpy(ptr, "MThd", 4); ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 6;
	ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 1; // MIDI format 0 with 1 track
	ptr += 4;

	*ptr++ = ppqn >> 8;
	*ptr++ = ppqn & 0xFF;

	memcpy(ptr, "MTrk", 4); ptr += 4;
	memcpy(ptr, &dw, 4); ptr += 4;

	return ptr;
}

static byte *writeVLQ(byte *ptr, int value) {
	if (value > 0x7f) {
		if (value > 0x3fff) {
			*ptr++ = (value >> 14) | 0x80;
			value &= 0x3fff;
		}
		*ptr++ = (value >> 7) | 0x80;
		value &= 0x7f;
	}
	*ptr++ = value;
	return ptr;
}

static void convertADResource(ResourceManager *res, const GameSettings& game, ResId idx, byte *src_ptr, int size) {
	// We will ignore the PPQN in the original resource, because
	// it's invalid anyway. We use a constant PPQN of 480.
	const int ppqn = 480;
	uint32 dw;
	int i, ch;
	byte *ptr;
	int total_size = kMIDIHeaderSize + 7 + 8 * sizeof(ADLIB_INSTR_MIDI_HACK) + size;
	total_size += 24;	// Up to 24 additional bytes are needed for the jump sysex

	ptr = res->createResource(rtSound, idx, total_size);

	src_ptr += 2;
	size -= 2;

	// 0x80 marks a music resource. Otherwise it's a SFX
	if (*src_ptr == 0x80) {
		byte ticks, play_once;
		byte num_instr;
		byte *channel, *instr, *track;

		ptr = writeMIDIHeader(ptr, "ADL ", ppqn, total_size);

		// The "speed" of the song
		ticks = *(src_ptr + 1);

		// Flag that tells us whether we should loop the song (0) or play it only once (1)
		play_once = *(src_ptr + 2);

		// Number of instruments used
		num_instr = *(src_ptr + 8);	// Normally 8

		// copy the pointer to instrument data
		channel = src_ptr + 9;
		instr   = src_ptr + 0x11;

		// skip over the rest of the header and copy the MIDI data into a buffer
		src_ptr  += 0x11 + 8 * 16;
		size -= 0x11 + 8 * 16;

		track = src_ptr;

		// Convert the ticks into a MIDI tempo.
		// Unfortunate LOOM and INDY3 have different interpretation
		// of the ticks value.
		if (game.id == GID_INDY3) {
			// Note: since we fix ppqn at 480, ppqn/473 is almost 1
			dw = 500000 * 256 / 473 * ppqn / ticks;
		} else if (game.id == GID_LOOM && game.version == 3) {
			dw = 500000 * ppqn / 4 / ticks;
		} else {
			dw = 500000 * 256 / ticks;
		}
		debugC(DEBUG_SOUND, "  ticks = %d, speed = %d", ticks, dw);

		// Write a tempo change Meta event
		memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
		*ptr++ = (byte)((dw >> 16) & 0xFF);
		*ptr++ = (byte)((dw >> 8) & 0xFF);
		*ptr++ = (byte)(dw & 0xFF);

		// Copy our hardcoded instrument table into it
		// Then, convert the instrument table as given in this song resource
		// And write it *over* the hardcoded table.
		// Note: we deliberately.

		/* now fill in the instruments */
		for (i = 0; i < num_instr; i++) {
			ch = channel[i] - 1;
			if (ch < 0 || ch > 15)
				continue;

			if (instr[i*16 + 13])
				debugC(DEBUG_SOUND, "Sound %d instrument %d uses percussion", idx, i);

			debugC(DEBUG_SOUND, "Sound %d: instrument %d on channel %d.", idx, i, ch);

			memcpy(ptr, ADLIB_INSTR_MIDI_HACK, sizeof(ADLIB_INSTR_MIDI_HACK));

			ptr[5]  += ch;
			ptr[28] += ch;
			ptr[92] += ch;

			/* mod_characteristics */
			ptr[30 + 0] = (instr[i * 16 + 3] >> 4) & 0xf;
			ptr[30 + 1] = instr[i * 16 + 3] & 0xf;

			/* mod_scalingOutputLevel */
			ptr[30 + 2] = (instr[i * 16 + 4] >> 4) & 0xf;
			ptr[30 + 3] = instr[i * 16 + 4] & 0xf;

			/* mod_attackDecay */
			ptr[30 + 4] = ((~instr[i * 16 + 5]) >> 4) & 0xf;
			ptr[30 + 5] = (~instr[i * 16 + 5]) & 0xf;

			/* mod_sustainRelease */
			ptr[30 + 6] = ((~instr[i * 16 + 6]) >> 4) & 0xf;
			ptr[30 + 7] = (~instr[i * 16 + 6]) & 0xf;

			/* mod_waveformSelect */
			ptr[30 + 8] = (instr[i * 16 + 7] >> 4) & 0xf;
			ptr[30 + 9] = instr[i * 16 + 7] & 0xf;

			/* car_characteristic */
			ptr[30 + 10] = (instr[i * 16 + 8] >> 4) & 0xf;
			ptr[30 + 11] = instr[i * 16 + 8] & 0xf;

			/* car_scalingOutputLevel */
			ptr[30 + 12] = (instr[i * 16 + 9] >> 4) & 0xf;
			ptr[30 + 13] = instr[i * 16 + 9] & 0xf;

			/* car_attackDecay */
			ptr[30 + 14] = ((~instr[i * 16 + 10]) >> 4) & 0xf;
			ptr[30 + 15] = (~instr[i * 16 + 10]) & 0xf;

			/* car_sustainRelease */
			ptr[30 + 16] = ((~instr[i * 16 + 11]) >> 4) & 0xf;
			ptr[30 + 17] = (~instr[i * 16 + 11]) & 0xf;

			/* car_waveFormSelect */
			ptr[30 + 18] = (instr[i * 16 + 12] >> 4) & 0xf;
			ptr[30 + 19] = instr[i * 16 + 12] & 0xf;

			/* feedback */
			ptr[30 + 20] = (instr[i * 16 + 2] >> 4) & 0xf;
			ptr[30 + 21] = instr[i * 16 + 2] & 0xf;
			ptr += sizeof(ADLIB_INSTR_MIDI_HACK);
		}

		// There is a constant delay of ppqn/3 before the music starts.
		if (ppqn / 3 >= 128)
			*ptr++ = ((ppqn / 3) >> 7) | 0x80;
		*ptr++ = ppqn / 3 & 0x7f;

		// Now copy the actual music data
		memcpy(ptr, track, size);
		ptr += size;

		if (!play_once) {
			// The song is meant to be looped. We achieve this by inserting just
			// before the song end a jump to the song start. More precisely we abuse
			// a S&M sysex, "maybe_jump" to achieve this effect. We could also
			// use a set_loop sysex, but it's a bit longer, a little more complicated,
			// and has no advantage either.

			// First, find the track end
			byte *end = ptr;
			ptr -= size;
			for (; ptr < end; ptr++) {
				if (*ptr == 0xff && *(ptr + 1) == 0x2f)
					break;
			}
			assert(ptr < end);

			// Now insert the jump. The jump offset is measured in ticks.
			// We have ppqn/3 ticks before the first note.

			const int jump_offset = ppqn / 3;
			memcpy(ptr, "\xf0\x13\x7d\x30\00", 5); ptr += 5;	// maybe_jump
			memcpy(ptr, "\x00\x00", 2); ptr += 2;			// cmd -> 0 means always jump
			memcpy(ptr, "\x00\x00\x00\x00", 4); ptr += 4;	// track -> there is only one track, 0
			memcpy(ptr, "\x00\x00\x00\x01", 4); ptr += 4;	// beat -> for now, 1 (first beat)
			// Ticks
			*ptr++ = (byte)((jump_offset >> 12) & 0x0F);
			*ptr++ = (byte)((jump_offset >> 8) & 0x0F);
			*ptr++ = (byte)((jump_offset >> 4) & 0x0F);
			*ptr++ = (byte)(jump_offset & 0x0F);
			memcpy(ptr, "\x00\xf7", 2); ptr += 2;	// sysex end marker
		}
	} else {

		/* This is a sfx resource.  First parse it quickly to find the parallel
		 * tracks.
		 */
		ptr = writeMIDIHeader(ptr, "ASFX", ppqn, total_size);

		byte current_instr[3][14];
		int  current_note[3];
		int track_time[3];
		byte *track_data[3];
		memset(current_instr, 0, sizeof(current_instr));

		int track_ctr = 0;
		byte chunk_type = 0;
		int delay, delay2, olddelay;

		// Write a tempo change Meta event
		// 473 / 4 Hz, convert to micro seconds.
		dw = 1000000 * ppqn * 4 / 473;
		memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
		*ptr++ = (byte)((dw >> 16) & 0xFF);
		*ptr++ = (byte)((dw >> 8) & 0xFF);
		*ptr++ = (byte)(dw & 0xFF);

		for (i = 0; i < 3; i++) {
			track_time[i] = -1;
			current_note[i] = -1;
		}
		while (size > 0) {
			assert(track_ctr < 3);
			track_data[track_ctr] = src_ptr;
			track_time[track_ctr] = 0;
			track_ctr++;
			while (size > 0) {
				chunk_type = *(src_ptr);
				if (chunk_type == 1) {
					src_ptr += 15;
					size -= 15;
				} else if (chunk_type == 2) {
					src_ptr += 11;
					size -= 11;
				} else if (chunk_type == 0x80) {
					src_ptr ++;
					size --;
				} else {
					break;
				}
			}
			if (chunk_type == 0xff)
				break;
			src_ptr++;
		}

		int curtime = 0;
		for (;;) {
			int mintime = -1;
			ch = -1;
			for (i = 0; i < 3; i++) {
				if (track_time[i] >= 0 &&
					(mintime == -1 || mintime > track_time[i])) {
					mintime = track_time[i];
					ch = i;
				}
			}
			if (mintime < 0)
				break;

			src_ptr = track_data[ch];
			chunk_type = *src_ptr;

			if (current_note[ch] >= 0) {
				delay = mintime - curtime;
				curtime = mintime;
				ptr = writeVLQ(ptr, delay);
				*ptr++ = 0x80 + ch; // key off channel;
				*ptr++ = current_note[ch];
				*ptr++ = 0;
				current_note[ch] = -1;
			}

			switch (chunk_type) {
			case 1:
				/* Instrument definition */
				memcpy(current_instr[ch], src_ptr + 1, 14);
				src_ptr += 15;
				break;

			case 2:
				/* tone/parammodulation */
				memcpy(ptr, ADLIB_INSTR_MIDI_HACK,
					   sizeof(ADLIB_INSTR_MIDI_HACK));

				ptr[5]  += ch;
				ptr[28] += ch;
				ptr[92] += ch;

				/* mod_characteristic */
				ptr[30 + 0] = (current_instr[ch][3] >> 4) & 0xf;
				ptr[30 + 1] = current_instr[ch][3] & 0xf;

				/* mod_scalingOutputLevel */
				ptr[30 + 2] = (current_instr[ch][4] >> 4) & 0xf;
				ptr[30 + 3] = current_instr[ch][4] & 0xf;

				/* mod_attackDecay */
				ptr[30 + 4] = ((~current_instr[ch][5]) >> 4) & 0xf;
				ptr[30 + 5] = (~current_instr[ch][5]) & 0xf;

				/* mod_sustainRelease */
				ptr[30 + 6] = ((~current_instr[ch][6]) >> 4) & 0xf;
				ptr[30 + 7] = (~current_instr[ch][6]) & 0xf;

				/* mod_waveformSelect */
				ptr[30 + 8] = (current_instr[ch][7] >> 4) & 0xf;
				ptr[30 + 9] = current_instr[ch][7] & 0xf;

				/* car_characteristic */
				ptr[30 + 10] = (current_instr[ch][8] >> 4) & 0xf;
				ptr[30 + 11] = current_instr[ch][8] & 0xf;

				/* car_scalingOutputLevel */
				ptr[30 + 12] = ((current_instr[ch][9]) >> 4) & 0xf;
				ptr[30 + 13] = (current_instr[ch][9]) & 0xf;

				/* car_attackDecay */
				ptr[30 + 14] = ((~current_instr[ch][10]) >> 4) & 0xf;
				ptr[30 + 15] = (~current_instr[ch][10]) & 0xf;

				/* car_sustainRelease */
				ptr[30 + 16] = ((~current_instr[ch][11]) >> 4) & 0xf;
				ptr[30 + 17] = (~current_instr[ch][11]) & 0xf;

				/* car_waveFormSelect */
				ptr[30 + 18] = (current_instr[ch][12] >> 4) & 0xf;
				ptr[30 + 19] = current_instr[ch][12] & 0xf;

				/* feedback */
				ptr[30 + 20] = (current_instr[ch][2] >> 4) & 0xf;
				ptr[30 + 21] = current_instr[ch][2] & 0xf;

				delay = mintime - curtime;
				curtime = mintime;

				{
					delay = convert_extraflags(ptr + 30 + 22, src_ptr + 1);
					delay2 = convert_extraflags(ptr + 30 + 40, src_ptr + 6);
					debugC(DEBUG_SOUND, "delays: %d / %d", delay, delay2);
					if (delay2 >= 0 && delay2 < delay)
						delay = delay2;
					if (delay == -1)
						delay = 0;
				}

				/* duration */
				ptr[30 + 58] = 0; // ((delay * 17 / 63) >> 4) & 0xf;
				ptr[30 + 59] = 0; // (delay * 17 / 63) & 0xf;

				ptr += sizeof(ADLIB_INSTR_MIDI_HACK);

				olddelay = mintime - curtime;
				curtime = mintime;
				ptr = writeVLQ(ptr, olddelay);

				{
					int freq = ((current_instr[ch][1] & 3) << 8)
						| current_instr[ch][0];
					if (!freq)
						freq = 0x80;
					freq <<= (((current_instr[ch][1] >> 2) + 1) & 7);
					int note = -11;
					while (freq >= 0x100) {
						note += 12;
						freq >>= 1;
					}
					debugC(DEBUG_SOUND, "Freq: %d (%x) Note: %d", freq, freq, note);
					if (freq < 0x80)
						note = 0;
					else
						note += freq2note[freq - 0x80];

					debugC(DEBUG_SOUND, "Note: %d", note);
					if (note <= 0)
						note = 1;
					else if (note > 127)
						note = 127;

					// Insert a note on event
					*ptr++ = 0x90 + ch; // key on channel
					*ptr++ = note;
					*ptr++ = 63;
					current_note[ch] = note;
					track_time[ch] = curtime + delay;
				}
				src_ptr += 11;
				break;

			case 0x80:
				// FIXME: This is incorrect. The original uses 0x80 for
				// looping a single channel. We currently interpret it as stop
				// thus we won't get looping for sound effects. It should
				// always jump to the start of the channel.
				//
				// Since we convert the data to MIDI and we cannot only loop a
				// single channel via MIDI fixing this will require some more
				// thought.
				track_time[ch] = -1;
				src_ptr ++;
				break;

			default:
				track_time[ch] = -1;
				break;
			}
			track_data[ch] = src_ptr;
		}
	}

	// Insert end of song sysex
	memcpy(ptr, "\x00\xff\x2f\x00\x00", 5); ptr += 5;
}


int ScummEngine::readSoundResourceSmallHeader(ResId idx) {
	uint32 pos, total_size, size, tag;
	uint32 ad_size = 0, ad_offs = 0;
	uint32 ro_size = 0, ro_offs = 0;
	uint32 wa_size = 0, wa_offs = 0;

	debug(4, "readSoundResourceSmallHeader(%d)", idx);

	if (_sound->isRolandLoom()) {
		// Roland resources in Loom are tagless
		// So we add an RO tag to allow imuse to detect format
		byte *ptr, *src_ptr;
		ro_offs = _fileHandle->pos();
		ro_size = _fileHandle->readUint16LE();

		src_ptr = (byte *) calloc(ro_size - 4, 1);
		_fileHandle->seek(ro_offs + 4, SEEK_SET);
		_fileHandle->read(src_ptr, ro_size -4);

		ptr = _res->createResource(rtSound, idx, ro_size + 2);
		memcpy(ptr, "RO", 2); ptr += 2;
		memcpy(ptr, src_ptr, ro_size - 4); ptr += ro_size - 4;
		free(src_ptr);
		return 1;
	} else if (_game.features & GF_OLD_BUNDLE) {
		wa_offs = _fileHandle->pos();
		wa_size = _fileHandle->readUint16LE();
		_fileHandle->seek(wa_size - 2, SEEK_CUR);

		if (!(_game.platform == Common::kPlatformAtariST || _game.platform == Common::kPlatformMacintosh)) {
			ad_offs = _fileHandle->pos();
			ad_size = _fileHandle->readUint16LE();
		}
		_fileHandle->seek(4, SEEK_CUR);
		total_size = wa_size + ad_size;
	} else {
		total_size = size = _fileHandle->readUint32LE();
		tag = _fileHandle->readUint16LE();
		debug(4, "  tag='%c%c', size=%d", (char) (tag & 0xff),
				(char) ((tag >> 8) & 0xff), size);

		if (tag == 0x4F52) { // RO
			ro_offs = _fileHandle->pos();
			ro_size = size;
		} else {
			pos = 6;
			while (pos < total_size) {
				size = _fileHandle->readUint32LE();
				tag = _fileHandle->readUint16LE();
				debug(4, "  tag='%c%c', size=%d", (char) (tag & 0xff),
						(char) ((tag >> 8) & 0xff), size);
				pos += size;

				// MI1 and Indy3 uses one or more nested SO resources, which contains AD and WA
				// resources.
				if ((tag == 0x4441) && !(ad_offs)) { // AD
					ad_size = size;
					ad_offs = _fileHandle->pos();
				} else if ((tag == 0x4157) && !(wa_offs)) { // WA
					wa_size = size;
					wa_offs = _fileHandle->pos();
				} else { // other AD, WA and nested SO resources
					if (tag == 0x4F53) { // SO
						pos -= size;
						size = 6;
						pos += 6;
					}
				}
				_fileHandle->seek(size - 6, SEEK_CUR);
			}
		}
	}

	if ((_sound->_musicType == MDT_PCSPK || _sound->_musicType == MDT_PCJR || _sound->_musicType == MDT_MACINTOSH) && wa_offs != 0) {
		if (_game.features & GF_OLD_BUNDLE) {
			_fileHandle->seek(wa_offs, SEEK_SET);
			_fileHandle->read(_res->createResource(rtSound, idx, wa_size), wa_size);
		} else {
			_fileHandle->seek(wa_offs - 6, SEEK_SET);
			_fileHandle->read(_res->createResource(rtSound, idx, wa_size + 6), wa_size + 6);
		}
		return 1;
	} else if (_sound->_musicType == MDT_CMS) {
		if (_game.features & GF_OLD_BUNDLE) {
			bool hasAdLibMusicTrack = false;

			if (ad_offs) {
				_fileHandle->seek(ad_offs + 4 + 2, SEEK_SET);
				hasAdLibMusicTrack = (_fileHandle->readByte() == 0x80);
			}

			if (hasAdLibMusicTrack) {
				_fileHandle->seek(ad_offs, SEEK_SET);
				_fileHandle->read(_res->createResource(rtSound, idx, ad_size), ad_size);
			} else {
				_fileHandle->seek(wa_offs, SEEK_SET);
				_fileHandle->read(_res->createResource(rtSound, idx, wa_size), wa_size);
			}
		} else {
			bool hasAdLibMusicTrack = false;

			if (ad_offs) {
				_fileHandle->seek(ad_offs + 2, SEEK_SET);
				hasAdLibMusicTrack = (_fileHandle->readByte() == 0x80);
			}

			if (hasAdLibMusicTrack) {
				_fileHandle->seek(ad_offs - 4, SEEK_SET);
				_fileHandle->read(_res->createResource(rtSound, idx, ad_size + 4), ad_size + 4);
			} else {
				_fileHandle->seek(wa_offs - 6, SEEK_SET);
				_fileHandle->read(_res->createResource(rtSound, idx, wa_size + 6), wa_size + 6);
			}
		}
		return 1;
	} else if (ad_offs != 0) {
		// AD resources have a header, instrument definitions and one MIDI track.
		// We build an 'ADL ' resource from that:
		//   8 bytes resource header
		//  16 bytes MDhd header
		//  14 bytes MThd header
		//   8 bytes MTrk header
		//   7 bytes MIDI tempo sysex
		//     + some default instruments
		if (_game.features & GF_OLD_BUNDLE) {
			ad_size -= 4;
			_fileHandle->seek(ad_offs + 4, SEEK_SET);
		} else {
			ad_size -= 6;
			_fileHandle->seek(ad_offs, SEEK_SET);
		}
		// For games using AD except Indy3 and Loom we are using our iMuse
		// implementation. See output initialization in
		// ScummEngine::setupMusic for more information.
		if (_game.id != GID_INDY3 && _game.id != GID_LOOM) {
			byte *ptr = (byte *)calloc(ad_size, 1);
			_fileHandle->read(ptr, ad_size);
			convertADResource(_res, _game, idx, ptr, ad_size);
			free(ptr);
		} else {
			_fileHandle->read(_res->createResource(rtSound, idx, ad_size), ad_size);
		}
		return 1;
	} else if (ro_offs != 0) {
		_fileHandle->seek(ro_offs - 2, SEEK_SET);
		_fileHandle->read(_res->createResource(rtSound, idx, ro_size - 4), ro_size - 4);
		return 1;
	}
	_res->_types[rtSound][idx]._roomoffs = RES_INVALID_OFFSET;
	return 0;
}

void Sound::updateMusicTimer() {
	if (_vm->_game.features & GF_AUDIOTRACKS) {
		_vm->VAR(_vm->VAR_MUSIC_TIMER) = _soundCD->getCDMusicTimer();
	} else if (_vm->VAR_MUSIC_TIMER != 0xFF) {
		if (_soundCD->useReplacementAudio() && _soundCD->getCurrentCDSound()) {
			// The replacement music timer operates on real time, adjusted to
			// the expected length of the Loom Overture (since there are so
			// many different recordings of it). It's completely independent of
			// the SCUMM engine's timer frequency.
			_soundCD->updateMusicTimer();
			_vm->VAR(_vm->VAR_MUSIC_TIMER) = _soundCD->getMusicTimer();
		} else if (_vm->_musicEngine) {
			// The music engine generates the timer data for us.
			_vm->VAR(_vm->VAR_MUSIC_TIMER) = _vm->_musicEngine->getMusicTimer() * _vm->getTimerFrequency() / 240.0;
		}
	}
}

void Sound::startRemasteredSpeech(const char *msgString, uint16 roomNumber, uint16 actorTalking, uint16 currentScriptNum, uint16 currentScriptOffset, uint16 numWaits) {
	// Crudely adapted from the disasm of MI1SE...
	// TODO: Apply the various speech-line substitutions performed per-game

	int32 soundIndex = _soundSE->handleRemasteredSpeech(msgString, nullptr, roomNumber, actorTalking, currentScriptNum, currentScriptOffset, numWaits);

	if (soundIndex >= 0) {
		_currentMISpeechIndex = soundIndex;
		talkSound(0, 0, DIGI_SND_MODE_TALKIE);
	}
}

} // End of namespace Scumm
