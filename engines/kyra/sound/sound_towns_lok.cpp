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

#include "kyra/sound/sound_intern.h"
#include "kyra/resource/resource.h"

#include "audio/softsynth/fmtowns_pc98/towns_euphony.h"

#include "common/config-manager.h"

#include "backends/audiocd/audiocd.h"

namespace Kyra {

SoundTowns_LoK::SoundTowns_LoK(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: Sound(vm, mixer), _lastTrack(-1), _musicTrackData(0), _sfxFileData(0), _cdaPlaying(0),
	_sfxFileIndex((uint)-1), _musicFadeTable(0), _sfxWDTable(0), _sfxBTTable(0), _sfxChannel(0x46), _currentResourceSet(0) {
	memset(&_resInfo, 0, sizeof(_resInfo));
	_player = new EuphonyPlayer(_mixer);
}

SoundTowns_LoK::~SoundTowns_LoK() {
	g_system->getAudioCDManager()->stop();
	haltTrack();
	delete _player;
	delete[] _musicTrackData;
	delete[] _sfxFileData;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundTowns_LoK::init() {
	_vm->checkCD();
	int unused = 0;
	_musicFadeTable = _vm->staticres()->loadRawData(k1TownsMusicFadeTable, unused);
	_sfxWDTable = _vm->staticres()->loadRawData(k1TownsSFXwdTable, unused);
	_sfxBTTable = _vm->staticres()->loadRawData(k1TownsSFXbtTable, unused);
	_musicTrackData = new uint8[50570];

	if (!_player->init())
		return false;

	if (!loadInstruments())
		return false;

	/*_player->driver()->intf()->callback(68);
	_player->driver()->intf()->callback(70, 0x33);*/
	_player->driver()->setOutputVolume(1, 118, 118);

	// Initialize CD for audio
	g_system->getAudioCDManager()->open();

	return true;
}

void SoundTowns_LoK::process() {
	g_system->getAudioCDManager()->update();
}

void SoundTowns_LoK::playTrack(uint8 track) {
	if (track < 2)
		return;
	track -= 2;

	uint tTableIndex = 3 * track;

	assert(tTableIndex + 2 < res()->cdaTableSize);

	int trackNum = (int)READ_LE_UINT32(&res()->cdaTable[tTableIndex + 2]);
	int32 loop = (int32)READ_LE_UINT32(&res()->cdaTable[tTableIndex + 1]);

	if (track == _lastTrack && _musicEnabled)
		return;

	beginFadeOut();

	if (_musicEnabled == 2 && trackNum != -1) {
		_player->driver()->setOutputVolume(1, 118, 118);
		g_system->getAudioCDManager()->play(trackNum + 1, loop ? -1 : 1, 0, 0);
		g_system->getAudioCDManager()->update();
		_cdaPlaying = true;
	} else if (_musicEnabled) {
		playEuphonyTrack(READ_LE_UINT32(&res()->cdaTable[tTableIndex]), loop);
		_cdaPlaying = false;
	}

	_lastTrack = track;
}

void SoundTowns_LoK::haltTrack() {
	_lastTrack = -1;
	g_system->getAudioCDManager()->stop();
	g_system->getAudioCDManager()->update();
	_cdaPlaying = false;

	for (int i = 0; i < 6; i++)
		_player->driver()->channelVolume(i, 0);
	for (int i = 0x40; i < 0x46; i++)
		_player->driver()->channelVolume(i, 0);
	for (int i = 0; i < 32; i++)
		_player->configPart_enable(i, 0);
	_player->stop();
}

void SoundTowns_LoK::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new SoundResourceInfo_Towns(*(SoundResourceInfo_Towns*)info) : 0;
	}
}

void SoundTowns_LoK::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundTowns_LoK::hasSoundFile(uint file) const {
	if (file < res()->fileListSize)
		return (res()->fileList[file] != 0);
	return false;
}

void SoundTowns_LoK::loadSoundFile(uint file) {
	if (_sfxFileIndex == file || file >= res()->fileListSize)
		return;
	_sfxFileIndex = file;
	delete[] _sfxFileData;
	_sfxFileData = _vm->resource()->fileData(res()->fileList[file], 0);
}

void SoundTowns_LoK::playSoundEffect(uint16 track, uint8) {
	if (!_sfxEnabled || !_sfxFileData)
		return;

	if (track == 0 || track == 10) {
		stopAllSoundEffects();
		return;
	} else if (track == 1) {
		fadeOutSoundEffects();
		return;
	}

	uint8 note = 60;
	if (_sfxFileIndex == 5) {
		if (track == 16) {
			note = 62;
			track = 15;
		} else if (track == 17) {
			note = 64;
			track = 15;
		} else if (track == 18) {
			note = 65;
			track = 15;
		}
	}

	uint8 *fileBody = _sfxFileData + 0x01B8;
	int32 offset = (int32)READ_LE_UINT32(_sfxFileData + (track - 0x0B) * 4);
	if (offset == -1)
		return;

	if (!_player->driver()->soundEffectIsPlaying(_sfxChannel ^ 1)) {
		_sfxChannel ^= 1;
	} else if (_player->driver()->soundEffectIsPlaying(_sfxChannel)) {
		_sfxChannel ^= 1;
		_player->driver()->stopSoundEffect(_sfxChannel);
	}

	uint32 *sfxHeader = (uint32 *)(fileBody + offset);
	uint32 sfxHeaderID = READ_LE_UINT32(sfxHeader);
	uint32 playbackBufferSize = sfxHeaderID == 1 ? 30704 : READ_LE_UINT32(&sfxHeader[3]);

	uint8 *sfxPlaybackBuffer = new uint8[playbackBufferSize + 32];
	memcpy(sfxPlaybackBuffer, fileBody + offset, 32);

	uint8 *dst = sfxPlaybackBuffer + 32;
	memset(dst, 0x80, playbackBufferSize);

	uint8 *sfxBody = ((uint8 *)sfxHeader) + 0x20;

	if (!sfxHeaderID) {
		memcpy(dst, sfxBody, playbackBufferSize);
	} else if (sfxHeaderID == 1) {
		Screen::decodeFrame4(sfxBody, dst, playbackBufferSize);
	} else if (_sfxWDTable) {
		uint8 *tgt = dst;
		uint32 sfx_BtTable_Offset = 0;
		uint32 sfx_WdTable_Offset = 0;
		uint32 sfx_WdTable_Number = 5;
		uint32 inSize = READ_LE_UINT32(&sfxHeader[1]);

		for (uint32 i = 0; i < inSize; i++) {
			sfx_WdTable_Offset = (sfx_WdTable_Number * 3 << 9) + sfxBody[i] * 6;
			sfx_WdTable_Number = READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset);

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 2);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xFF)];

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 4);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xFF)];
		}
	}

	_player->driver()->channelVolume(_sfxChannel, 127);
	_player->driver()->channelPan(_sfxChannel, 0x40);
	_player->driver()->channelPitch(_sfxChannel, 0);
	_player->driver()->playSoundEffect(_sfxChannel, note, 127, sfxPlaybackBuffer);
	delete[] sfxPlaybackBuffer;
}

void SoundTowns_LoK::updateVolumeSettings() {
	if (!_player)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_player->driver()->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_player->driver()->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

void SoundTowns_LoK::stopAllSoundEffects() {
	_player->driver()->channelVolume(0x46, 0);
	_player->driver()->channelVolume(0x47, 0);
	_player->driver()->stopSoundEffect(0x46);
	_player->driver()->stopSoundEffect(0x47);
	_sfxChannel = 0x46;
}

void SoundTowns_LoK::beginFadeOut() {
	if (_cdaPlaying) {
		for (int i = 118; i > 103; i--) {
			_player->driver()->setOutputVolume(1, i, i);
			_vm->delay(2 * _vm->tickLength());
		}

		for (int i = 103; i > 83; i -= 2) {
			_player->driver()->setOutputVolume(1, i, i);
			_vm->delay(2 * _vm->tickLength());
		}

		for (int i = 83; i > 58; i -= 2) {
			_player->driver()->setOutputVolume(1, i, i);
			_vm->delay(_vm->tickLength());
		}

		for (int i = 58; i > 0; i--) {
			_player->driver()->setOutputVolume(1, i, i);
			_vm->delay(1);
		}

		_player->driver()->setOutputVolume(1, 0, 0);

	} else {
		if (_lastTrack == -1)
			return;

		uint32 ticks = 2;
		int tickAdv = 0;

		uint16 fadeVolCur[12];
		uint16 fadeVolStep[12];

		for (int i = 0; i < 6; i++) {
			fadeVolCur[i] = READ_LE_UINT16(&_musicFadeTable[(_lastTrack * 12 + i) * 2]);
			fadeVolStep[i] = fadeVolCur[i] / 50;
			fadeVolCur[i + 6] = READ_LE_UINT16(&_musicFadeTable[(_lastTrack * 12 + 6 + i) * 2]);
			fadeVolStep[i + 6] = fadeVolCur[i + 6] / 30;
		}

		for (int i = 0; i < 12; i++) {
			for (int ii = 0; ii < 6; ii++)
				_player->driver()->channelVolume(ii, fadeVolCur[ii]);
			for (int ii = 0x40; ii < 0x46; ii++)
				_player->driver()->channelVolume(ii, fadeVolCur[ii - 0x3A]);

			for (int ii = 0; ii < 6; ii++) {
				fadeVolCur[ii] -= fadeVolStep[ii];
				if (fadeVolCur[ii] < 10)
					fadeVolCur[ii] = 0;
				fadeVolCur[ii + 6] -= fadeVolStep[ii + 6];
				if (fadeVolCur[ii + 6] < 10)
					fadeVolCur[ii + 6] = 0;
			}

			if (++tickAdv == 3) {
				tickAdv = 0;
				ticks += 2;
			}
			_vm->delay(ticks * _vm->tickLength());
		}
	}

	haltTrack();
}

bool SoundTowns_LoK::loadInstruments() {
	uint8 *twm = _vm->resource()->fileData("twmusic.pak", 0);
	if (!twm)
		return false;

	Screen::decodeFrame4(twm, _musicTrackData, 50570);
	for (int i = 0; i < 128; i++)
		_player->driver()->loadInstrument(0, i, &_musicTrackData[i * 48 + 8]);

	Screen::decodeFrame4(twm + 3232, _musicTrackData, 50570);
	for (int i = 0; i < 32; i++)
		_player->driver()->loadInstrument(0x40, i, &_musicTrackData[i * 128 + 8]);

	_player->driver()->unloadWaveTable(-1);
	uint8 *src = &_musicTrackData[32 * 128 + 8];
	for (int i = 0; i < 10; i++) {
		_player->driver()->loadWaveTable(src);
		src = src + READ_LE_UINT16(&src[12]) + 32;
	}

	_player->driver()->reserveSoundEffectChannels(2);

	delete[] twm;

	return true;
}

void SoundTowns_LoK::playEuphonyTrack(uint32 offset, int loop) {
	uint8 *twm = _vm->resource()->fileData("twmusic.pak", 0);
	Screen::decodeFrame4(twm + 19312 + offset, _musicTrackData, 50570);
	delete[] twm;

	const uint8 *src = _musicTrackData + 852;
	for (int i = 0; i < 32; i++)
		_player->configPart_enable(i, *src++);
	for (int i = 0; i < 32; i++)
		_player->configPart_setType(i, *src++);
	for (int i = 0; i < 32; i++)
		_player->configPart_remap(i, *src++);
	for (int i = 0; i < 32; i++)
		_player->configPart_adjustVolume(i, *src++);
	for (int i = 0; i < 32; i++)
		_player->configPart_setTranspose(i, *src++);

	src = _musicTrackData + 1748;
	for (int i = 0; i < 6; i++)
		_player->driver()->assignPartToChannel(i, *src++);
	for (int i = 0x40; i < 0x46; i++)
		_player->driver()->assignPartToChannel(i, *src++);

	uint32 trackSize = READ_LE_UINT32(_musicTrackData + 2048);
	uint8 startTick = _musicTrackData[2052];

	_player->setTempo(_musicTrackData[2053]);

	src = _musicTrackData + 2054;
	uint32 l = READ_LE_UINT32(src + trackSize);
	trackSize += (l + 4);
	l = READ_LE_UINT32(src + trackSize);
	trackSize += (l + 4);

	_player->setLoopStatus(loop);
	_player->startTrack(src, trackSize, startTick);
}

void SoundTowns_LoK::fadeOutSoundEffects() {
	for (int i = 127; i > 0; i-= 12) {
		_player->driver()->channelVolume(0x46, i);
		_player->driver()->channelVolume(0x47, i);
		_vm->delay(_vm->tickLength());
	}
	stopAllSoundEffects();
}

} // End of namespace Kyra

