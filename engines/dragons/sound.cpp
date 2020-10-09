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
#include <audio/soundfont/rawfile.h>
#include <audio/soundfont/vab/vab.h>
#include <audio/soundfont/vgmcoll.h>
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/xa.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "dragons/dragons.h"
#include "dragons/sound.h"
#include "dragons/bigfile.h"
#include "dragons/dragonrms.h"
#include "dragons/vabsound.h"

#define RAW_CD_SECTOR_SIZE 2352

#define CDXA_TYPE_MASK     0x0E
#define CDXA_TYPE_DATA     0x08
#define CDXA_TYPE_AUDIO    0x04

namespace Dragons {

struct SpeechLocation {
	uint32 talkId;
	uint16 sectorStart;
	int8 startOffset;
	uint16 sectorEnd;
} SpeechLocation;

void CdIntToPos_0(uint32 param_1) { //, byte *param_2)
	int iVar1;
	int iVar2;
	int iVar3;
	uint8 minute;
	uint8 second;
	uint8 sector;

	iVar3 = (param_1 + 0x96) / 0x4b;
	iVar2 = (param_1 + 0x96) % 0x4b;
	iVar1 = iVar3 / 0x3c;
	iVar3 = iVar3 % 0x3c;
	second = (char)iVar3 + (char)(iVar3 / 10) * 6;
	sector = (char)iVar2 + (char)(iVar2 / 10) * 6;
	minute = (char)iVar1 + (char)(iVar1 / 10) * 6;


	uint32 out =          (((uint)(minute >> 4) * 10 + ((uint)minute & 0xf)) * 0x3c +
						   (uint)(second >> 4) * 10 + ((uint)second & 0xf)) * 0x4b +
						  (uint)(sector >> 4) * 10 + ((uint)sector & 0xf) + -0x96;

	debug(3, "Seek Audio %2X:%2X:%2X  in: %d out %d", minute, second, sector, param_1, out);

	return;
}

void SoundManager::playSpeech(uint32 textIndex) {
	if (isSpeechPlaying()) {
		_vm->_mixer->stopHandle(_speechHandle);
	}

	// Reduce music volume while playing dialog.
	_midiPlayer->setVolume(_musicVolume / 2);

	struct SpeechLocation speechLocation;
	if (!getSpeechLocation(textIndex, &speechLocation)) {
		return;
	}

	Common::File *fd = new Common::File();
	if (!fd->open("dtspeech.xa")) {
		error("Failed to open dtspeech.xa");
	}
	CdIntToPos_0(speechLocation.sectorStart * 32);
	PSXAudioTrack *_audioTrack = new PSXAudioTrack();

	_vm->setFlags(ENGINE_FLAG_8000);
	_vm->_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, _audioTrack->createNewAudioStream(fd, speechLocation.sectorStart, speechLocation.startOffset, speechLocation.sectorEnd), -1, _speechVolume);
	fd->close();
	delete fd;
	delete _audioTrack;
}

bool SoundManager::isSpeechPlaying() {
	return _vm->_mixer->isSoundHandleActive(_speechHandle);
}

bool SoundManager::getSpeechLocation(uint32 talkId, struct SpeechLocation *location) {
	Common::File *fd = new Common::File();
	if (!fd->open("dragon.exe")) {
		error("Failed to open dragon.exe");
	}
	fd->seek(_vm->getSpeechTblOffsetFromDragonEXE());
	bool foundId = false;
	for (int i = 0; i < 2272; i++) { //TODO check that the number of speech audio tracks is the same across game variants
		uint32 id = (fd->readUint32LE() & 0xffffff);
		fd->seek(-1, SEEK_CUR);
		int8 startOffset = fd->readSByte();
		uint16 start = fd->readUint16LE();
		uint16 end = fd->readUint16LE();
		if (id == talkId) {
			location->talkId = id;
			location->sectorStart = start;
			location->startOffset = startOffset;
			location->sectorEnd = end;
			foundId = true;
			debug(3, "sectors [%d-%d] unk byte = %d", start * 32, end * 32, startOffset);
			break;
		}
	}

	fd->close();
	delete fd;

	return foundId;
}

void SoundManager::resumeMusic() {
	if (isSpeechPlaying()) {
		_vm->_mixer->stopHandle(_speechHandle);
		_vm->clearFlags(ENGINE_FLAG_8000);
	}
	if (_currentSong != -1) {
		_midiPlayer->resume();
	}
}

SoundManager::PSXAudioTrack::PSXAudioTrack() {
	memset(&_adpcmStatus, 0, sizeof(_adpcmStatus));
}

// Ha! It's palindromic!
#define AUDIO_DATA_CHUNK_SIZE   2304
#define AUDIO_DATA_SAMPLE_COUNT 4032

static const int s_xaTable[5][2] = {
	{   0,   0 },
	{  60,   0 },
	{ 115, -52 },
	{  98, -55 },
	{ 122, -60 }
};

void SoundManager::PSXAudioTrack::queueAudioFromSector(Audio::QueuingAudioStream *audStream, Common::SeekableReadStream *sector) {
	sector->skip(24);

	// This XA audio is different (yet similar) from normal XA audio! Watch out!
	// TODO: It's probably similar enough to normal XA that we can merge it somehow...
	// TODO: RTZ PSX needs the same audio code in a regular AudioStream class. Probably
	// will do something similar to QuickTime and creating a base class 'ISOMode2Parser'
	// or something similar.
	byte *buf = new byte[AUDIO_DATA_CHUNK_SIZE];
	sector->read(buf, AUDIO_DATA_CHUNK_SIZE);

	int channels = audStream->isStereo() ? 2 : 1;
	int16 *dst = new int16[AUDIO_DATA_SAMPLE_COUNT];
	int16 *leftChannel = dst;
	int16 *rightChannel = dst + 1;

	for (byte *src = buf; src < buf + AUDIO_DATA_CHUNK_SIZE; src += 128) {
		for (int i = 0; i < 4; i++) {
			int shift = 12 - (src[4 + i * 2] & 0xf);
			int filter = src[4 + i * 2] >> 4;
			int f0 = s_xaTable[filter][0];
			int f1 = s_xaTable[filter][1];
			int16 s_1 = _adpcmStatus[0].sample[0];
			int16 s_2 = _adpcmStatus[0].sample[1];

			for (int j = 0; j < 28; j++) {
				byte d = src[16 + i + j * 4];
				int t = (int8)(d << 4) >> 4;
				int s = (t << shift) + ((s_1 * f0 + s_2 * f1 + 32) >> 6);
				s_2 = s_1;
				s_1 = CLIP<int>(s, -32768, 32767);
				*leftChannel = s_1;
				leftChannel += channels;
			}

			if (channels == 2) {
				_adpcmStatus[0].sample[0] = s_1;
				_adpcmStatus[0].sample[1] = s_2;
				s_1 = _adpcmStatus[1].sample[0];
				s_2 = _adpcmStatus[1].sample[1];
			}

			shift = 12 - (src[5 + i * 2] & 0xf);
			filter = src[5 + i * 2] >> 4;
			f0 = s_xaTable[filter][0];
			f1 = s_xaTable[filter][1];

			for (int j = 0; j < 28; j++) {
				byte d = src[16 + i + j * 4];
				int t = (int8)d >> 4;
				int s = (t << shift) + ((s_1 * f0 + s_2 * f1 + 32) >> 6);
				s_2 = s_1;
				s_1 = CLIP<int>(s, -32768, 32767);

				if (channels == 2) {
					*rightChannel = s_1;
					rightChannel += 2;
				} else {
					*leftChannel++ = s_1;
				}
			}

			if (channels == 2) {
				_adpcmStatus[1].sample[0] = s_1;
				_adpcmStatus[1].sample[1] = s_2;
			} else {
				_adpcmStatus[0].sample[0] = s_1;
				_adpcmStatus[0].sample[1] = s_2;
			}
		}
	}

	int flags = Audio::FLAG_16BITS;

	if (audStream->isStereo())
		flags |= Audio::FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif

	audStream->queueBuffer((byte *)dst, AUDIO_DATA_SAMPLE_COUNT * 2, DisposeAfterUse::YES, flags);
	delete[] buf;
}

Audio::QueuingAudioStream *SoundManager::PSXAudioTrack::createNewAudioStream(Common::File *fd, uint16 sectorStart, int8 startOffset, uint16 sectorEnd) {
	fd->seek(((sectorStart * 32) + startOffset) * RAW_CD_SECTOR_SIZE);
	fd->skip(19);
	byte format = fd->readByte();
	bool stereo = (format & (1 << 0)) != 0;
	uint rate = (format & (1 << 2)) ? 18900 : 37800;

	Audio::QueuingAudioStream *audStream = Audio::makeQueuingAudioStream(rate, stereo);
	for (int i = 0x0; i < sectorEnd - sectorStart; i++) {
		fd->seek(((sectorStart * 32) + startOffset + i * 32) * RAW_CD_SECTOR_SIZE);
		queueAudioFromSector(audStream, fd);
	}
	audStream->finish();
	return audStream;
}

SoundManager::SoundManager(DragonsEngine *vm, BigfileArchive *bigFileArchive, DragonRMS *dragonRMS)
		: _vm(vm),
		  _sfxVolume(0),
		  _musicVolume(0),
		  _speechVolume(0),
		  _bigFileArchive(bigFileArchive),
		  _dragonRMS(dragonRMS) {
	_dat_8006bb60_sound_related = 0;
	_currentSong = -1;

	bool allSoundIsMuted = false;
	if (ConfMan.hasKey("mute")) {
		allSoundIsMuted = ConfMan.getBool("mute");
	}

	if (ConfMan.hasKey("speech_mute") && !allSoundIsMuted) {
		_vm->_mixer->muteSoundType(_vm->_mixer->kSpeechSoundType, ConfMan.getBool("speech_mute"));
	}

	if (ConfMan.hasKey("sfx_mute") && !allSoundIsMuted) {
		_vm->_mixer->muteSoundType(_vm->_mixer->kSFXSoundType, ConfMan.getBool("sfx_mute"));
	}

	if (ConfMan.hasKey("music_mute") && !allSoundIsMuted) {
		_vm->_mixer->muteSoundType(_vm->_mixer->kMusicSoundType, ConfMan.getBool("music_mute"));
	}

	SomeInitSound_FUN_8003f64c();
	initVabData();
	_midiPlayer = new MidiMusicPlayer(_bigFileArchive, _vabMusx);

	syncSoundSettings();
}

SoundManager::~SoundManager() {
	if (isSpeechPlaying()) {
		_vm->_mixer->stopHandle(_speechHandle);
	}

	stopAllVoices();

	_midiPlayer->stop();

	delete _midiPlayer;
	delete _vabMusx;
	delete _vabMsf;
	delete _vabGlob;
}

void SoundManager::SomeInitSound_FUN_8003f64c() {
	// TODO: Check if this changes on different game versions?
	memset(_sfxVolumeTbl, 0x10, sizeof(_sfxVolumeTbl));

	_sfxVolumeTbl[192] = 0x0b;
	_sfxVolumeTbl[193] = 0x0b;
	_sfxVolumeTbl[226] = _sfxVolumeTbl[226] | 0x80u;
	_sfxVolumeTbl[229] = 0x0b;
	_sfxVolumeTbl[230] = 0x0b;
	_sfxVolumeTbl[450] = 0x0b;
	_sfxVolumeTbl[451] = 0x0b;
	_sfxVolumeTbl[514] = 0x8b;
	_sfxVolumeTbl[515] = 0x0b;
	_sfxVolumeTbl[516] = 0x0b;
	_sfxVolumeTbl[578] = 0x0b;
	_sfxVolumeTbl[579] = 0x0b;
	_sfxVolumeTbl[580] = 0x0b;
	_sfxVolumeTbl[611] = 0x0b;
	_sfxVolumeTbl[674] = 0x8b;
	_sfxVolumeTbl[675] = 0x88;
	_sfxVolumeTbl[711] = 0x08;
	_sfxVolumeTbl[866] = 0x0b;
	_sfxVolumeTbl[896] = 0x0b;
	_sfxVolumeTbl[897] = _sfxVolumeTbl[897] | 0x80u;
	_sfxVolumeTbl[930] = _sfxVolumeTbl[930] | 0x80u;
	_sfxVolumeTbl[934] = 0x8b;
	_sfxVolumeTbl[935] = 0x8b;
	_sfxVolumeTbl[936] = 0x0b;
	_sfxVolumeTbl[937] = 0x88;
	_sfxVolumeTbl[941] = 0x0b;
	_sfxVolumeTbl[964] = 0x0b;
	_sfxVolumeTbl[995] = _sfxVolumeTbl[995] | 0x80u;
	_sfxVolumeTbl[1027] = 0x08;
	_sfxVolumeTbl[1056] = 0x8b;
	_sfxVolumeTbl[1059] = _sfxVolumeTbl[1059] | 0x80u;
	_sfxVolumeTbl[1122] = 0x0b;
	_sfxVolumeTbl[1250] = 0x08;
	_sfxVolumeTbl[1252] = 0x0b;
	_sfxVolumeTbl[1256] = 0x0b;
	_sfxVolumeTbl[1257] = 0x08;
	_sfxVolumeTbl[1258] = 0x0b;
	_sfxVolumeTbl[1284] = 0x0b;
	_sfxVolumeTbl[1378] = 0x0b;
	_sfxVolumeTbl[1379] = _sfxVolumeTbl[1379] | 0x80u;
	_sfxVolumeTbl[1380] = 0x0b;
	_sfxVolumeTbl[1385] = 0x0b;
	_sfxVolumeTbl[1443] = 0x8b;
	_sfxVolumeTbl[1444] = _sfxVolumeTbl[1444] | 0x80u;
	_sfxVolumeTbl[1445] = _sfxVolumeTbl[1445] | 0x80u;
	_sfxVolumeTbl[1446] = 0x8b;
	_sfxVolumeTbl[1472] = 0x8b;
	_sfxVolumeTbl[1508] = _sfxVolumeTbl[1508] | 0x80u;
	_sfxVolumeTbl[1575] = 0x08;
	_sfxVolumeTbl[1576] = 0x08;
	_sfxVolumeTbl[1577] = 0x08;
	_sfxVolumeTbl[1604] = 0x08;
	_sfxVolumeTbl[1605] = 0x08;
	_sfxVolumeTbl[1610] = 0x0b;
	_sfxVolumeTbl[1611] = 0x0b;
	_sfxVolumeTbl[1612] = 0x0b;
}

void SoundManager::initVabData() {
	_vabMusx = loadVab("musx.vh", "musx.vb");
	_vabMsf = loadVab("musx.vh", "musx.vb");
	_vabGlob = loadVab("glob.vh", "glob.vb");
}

VabSound * SoundManager::loadVab(const char *headerFilename, const char *bodyFilename) {
	uint32 headSize, bodySize;

	byte *headData = _bigFileArchive->load(headerFilename, headSize);
	byte *bodyData = _bigFileArchive->load(bodyFilename, bodySize);

	Common::SeekableReadStream *headStream = new Common::MemoryReadStream(headData, headSize, DisposeAfterUse::YES);
	Common::SeekableReadStream *bodyStream = new Common::MemoryReadStream(bodyData, bodySize, DisposeAfterUse::YES);

	return new VabSound(headStream, bodyStream);
}

/**
 *
 * @param soundId Bit 0x4000 set indicates STOP SOUND, bit 0x8000 set indicates SOUND IS GLOBAL (comes from glob.v[hb])
 */
void SoundManager::playOrStopSound(uint16 soundId) {
	uint16 volumeId;
	if ((soundId & 0x8000u) == 0) {
		volumeId = (soundId & ~0x4000u) + _vm->getCurrentSceneId() * 0x20;
	} else {
		volumeId = soundId & ~(0x4000u | 0x8000u);
	}

	if ((soundId & 0x4000u) == 0) {
		playSound(soundId, volumeId);
	} else {
		stopSound(soundId, volumeId);
	}
}

void SoundManager::playSound(uint16 soundId, uint16 volumeId) {
	byte volume = 0;

	volume = _sfxVolumeTbl[volumeId] & 0x1fu;
	_sfxVolumeTbl[volumeId] = _sfxVolumeTbl[volumeId] | 0x40u;      // Set bit 0x40

	VabSound *vabSound = ((soundId & 0x8000u) != 0) ? _vabGlob : _vabMsf;

	uint16 realId = soundId & 0x7fffu;

	uint16 program = realId >> 4u;
	uint16 key = ((realId & 0xfu) << 1u | 0x40u);

	if (isVoicePlaying(soundId)) {
		stopVoicePlaying(soundId);
	}

	if (vabSound->hasSound(program, key)) {
		Audio::SoundHandle *handle = getVoiceHandle(soundId);
		if (handle) {
			uint8 adjustedVolume = (uint8)((float)_sfxVolume * ((float)volume / 31));
			debug(3, "Playing SFX: Master Volume %d  Adjusted Volume %d diff %f%%", _sfxVolume, adjustedVolume, 100 * ((float)volume / 31));
			Audio::AudioStream *audioStream = vabSound->getAudioStream(program, key);
			if (audioStream) {
				_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, handle, audioStream, -1, adjustedVolume);
			}
		}
	} else {
		warning("Sound not found Program: %d, key %d", program, key);
	}
}

void SoundManager::stopSound(uint16 soundId, uint16 volumeId) {
	_sfxVolumeTbl[volumeId] = _sfxVolumeTbl[volumeId] & 0xbfu;      // Clear bit 0x40

//	uint16 vabId = getVabFromSoundId(soundId);

	stopVoicePlaying(soundId & ~0x4000u);
}

uint16 SoundManager::getVabFromSoundId(uint16 soundId) {
	// TODO
	return 0;
}

void SoundManager::loadMsf(uint32 sceneId) {
	char msfFileName[] = "XXXX.MSF";
	memcpy(msfFileName, _dragonRMS->getSceneName(sceneId), 4);

	debug(3, "Loading SFX file %s", msfFileName);

	if (_bigFileArchive->doesFileExist(msfFileName)) {
		uint32 msfSize;
		byte *msfData = _bigFileArchive->load(msfFileName, msfSize);

		Common::SeekableReadStream *msfStream = new Common::MemoryReadStream(msfData, msfSize, DisposeAfterUse::YES);

		stopAllVoices();

		delete _vabMsf;
		_vabMsf = new VabSound(msfStream, _vm);
	}
}

bool SoundManager::isVoicePlaying(uint16 soundID) {
	for (int i = 0; i < NUM_VOICES; i++) {
		if (_voice[i].soundID == soundID && _vm->_mixer->isSoundHandleActive(_voice[i].handle)) {
			return true;
		}
	}
	return false;
}

Audio::SoundHandle *SoundManager::getVoiceHandle(uint16 soundID) {
	for (int i = 0; i < NUM_VOICES; i++) {
		if (!_vm->_mixer->isSoundHandleActive(_voice[i].handle)) {
			_voice[i].soundID = soundID & ~0x4000u;
			return &_voice[i].handle;
		}
	}
	return nullptr;
}

void SoundManager::stopVoicePlaying(uint16 soundID) {
	for (int i = 0; i < NUM_VOICES; i++) {
		if (_voice[i].soundID == soundID) {
			_vm->_mixer->stopHandle(_voice[i].handle);
			return;
		}
	}
}

void SoundManager::stopAllVoices() {
	for (int i = 0; i < NUM_VOICES; i++) {
		_vm->_mixer->stopHandle(_voice[i].handle);
	}
}

void SoundManager::playMusic(int16 song) {
	char sceneName[5] = "nnnn";
	char filename[12] = "xxxxznn.msq";

	if (_currentSong == song) {
		return;
	}

	_currentSong = song;

	memcpy(sceneName, _vm->_dragonRMS->getSceneName(_vm->getCurrentSceneId()), 4);
	snprintf(filename, 12, "%sz%02d.msq", sceneName, song);
	debug(1, "Load music file %s", filename);

	if (!_bigFileArchive->doesFileExist(filename)) {
		warning("Could not find music file %s", filename);
		return;
	}

	uint32 dataSize;
	byte *seqData = _bigFileArchive->load(filename, dataSize);
	Common::MemoryReadStream *seq = new Common::MemoryReadStream(seqData, dataSize, DisposeAfterUse::YES);
	_midiPlayer->playSong(seq);
	delete seq;
}

void SoundManager::syncSoundSettings() {
	_musicVolume = CLIP<int>(ConfMan.getInt("music_volume"), 0, 255);
	_sfxVolume = CLIP<int>(ConfMan.getInt("sfx_volume"), 0, 255);
	_speechVolume = CLIP<int>(ConfMan.getInt("speech_volume"), 0, 255);

	_midiPlayer->setVolume(_musicVolume);
}

} // End of namespace Dragons
