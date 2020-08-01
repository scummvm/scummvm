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

#include "audio/softsynth/fmtowns_pc98/towns_pc98_driver.h"

#include "common/config-manager.h"

#include "backends/audiocd/audiocd.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Kyra {

SoundTownsPC98_v2::SoundTownsPC98_v2(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _currentSFX(0), _musicTrackData(0), _sfxTrackData(0), _lastTrack(-1), _driver(0), _useFmSfx(false), _currentResourceSet(0) {
	memset(&_resInfo, 0, sizeof(_resInfo));
}

SoundTownsPC98_v2::~SoundTownsPC98_v2() {
	delete[] _musicTrackData;
	delete[] _sfxTrackData;
	delete _driver;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundTownsPC98_v2::init() {
	_driver = new TownsPC98_AudioDriver(_mixer, _vm->gameFlags().platform == Common::kPlatformPC98 ?
		TownsPC98_AudioDriver::kType86 : TownsPC98_AudioDriver::kTypeTowns);

	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		if (_resInfo[_currentResourceSet])
			if (_resInfo[_currentResourceSet]->cdaTableSize)
				_vm->checkCD();

		// Initialize CD for audio
		bool hasRealCD = g_system->getAudioCDManager()->open();

		// FIXME: While checking for 'track1.XXX(X)' looks like
		// a good idea, we should definitely not be doing this
		// here. Basically our filenaming scheme could change
		// or we could add support for other audio formats. Also
		// this misses the possibility that we play the tracks
		// right off CD. So we should find another way to
		// check if we have access to CD audio.
		Resource *r = _vm->resource();
		if (_musicEnabled &&
		    (hasRealCD || r->exists("track1.mp3") || r->exists("track1.ogg") || r->exists("track1.flac") || r->exists("track1.fla")
		     || r->exists("track01.mp3") || r->exists("track01.ogg") || r->exists("track01.flac") || r->exists("track01.fla")))
				_musicEnabled = 2;
		else
			_musicEnabled = 1;
		_useFmSfx = false;

	} else {
		_useFmSfx = true;
	}

	bool reslt = _driver->init();
	updateVolumeSettings();
	return reslt;
}

void SoundTownsPC98_v2::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new SoundResourceInfo_TownsPC98V2(*(SoundResourceInfo_TownsPC98V2*)info) : 0;
	}
}

void SoundTownsPC98_v2::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundTownsPC98_v2::hasSoundFile(uint file) const {
	if (file < res()->fileListSize)
		return (res()->fileList[file] != 0);
	return false;
}

void SoundTownsPC98_v2::loadSoundFile(Common::String file) {
	delete[] _sfxTrackData;
	_sfxTrackData = _vm->resource()->fileData(file.c_str(), 0);
}

void SoundTownsPC98_v2::process() {
	g_system->getAudioCDManager()->update();
}

void SoundTownsPC98_v2::playTrack(uint8 track) {
	if (track == _lastTrack && _musicEnabled)
		return;

	int trackNum = -1;
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		for (uint i = 0; i < res()->cdaTableSize; i++) {
			if (track == (uint8)READ_LE_UINT16(&res()->cdaTable[i * 2])) {
				trackNum = (int)READ_LE_UINT16(&res()->cdaTable[i * 2 + 1]) - 1;
				break;
			}
		}
	}

	beginFadeOut();

	Common::String musicFile = res()->pattern ? Common::String::format(res()->pattern, track) : (res()->fileList ? res()->fileList[track] : 0);
	if (musicFile.empty())
		return;

	delete[] _musicTrackData;

	_musicTrackData = _vm->resource()->fileData(musicFile.c_str(), 0);
	_driver->loadMusicData(_musicTrackData, true);

	if (_musicEnabled == 2 && trackNum != -1) {
		g_system->getAudioCDManager()->play(trackNum+1, _driver->looping() ? -1 : 1, 0, 0);
		g_system->getAudioCDManager()->update();
	} else if (_musicEnabled) {
		_driver->cont();
	}

	_lastTrack = track;
}

void SoundTownsPC98_v2::haltTrack() {
	_lastTrack = -1;
	g_system->getAudioCDManager()->stop();
	g_system->getAudioCDManager()->update();
	_driver->reset();
}

void SoundTownsPC98_v2::beginFadeOut() {
	if (!_driver->musicPlaying())
		return;

	for (int i = 0; i < 20; i++) {
		_driver->fadeStep();
		_vm->delay(32);
	}

	haltTrack();
}

int32 SoundTownsPC98_v2::voicePlay(const char *file, Audio::SoundHandle *handle, uint8 volume, uint8 priority, bool) {
	static const uint16 rates[] = { 0x10E1, 0x0CA9, 0x0870, 0x0654, 0x0438, 0x032A, 0x021C, 0x0194 };
	static const char patternHOF[] = "%s.PCM";
	static const char patternLOL[] = "%s.VOC";

	int h = 0;
	if (_currentSFX) {
		while (h < kNumChannelHandles && _mixer->isSoundHandleActive(_soundChannels[h].handle))
			h++;

		if (h >= kNumChannelHandles) {
			h = 0;
			while (h < kNumChannelHandles && _soundChannels[h].priority > priority)
				++h;
			if (h < kNumChannelHandles)
				voiceStop(&_soundChannels[h].handle);
		}

		if (h >= kNumChannelHandles)
			return 0;
	}

	Common::String fileName = Common::String::format( _vm->game() == GI_LOL ? patternLOL : patternHOF, file);

	uint8 *data = _vm->resource()->fileData(fileName.c_str(), 0);
	uint8 *src = data;
	if (!src)
		return 0;

	uint16 sfxRate = rates[READ_LE_UINT16(src)];
	src += 2;
	bool compressed = (READ_LE_UINT16(src) & 1) ? true : false;
	src += 2;
	uint32 outsize = READ_LE_UINT32(src);
	uint8 *sfx = (uint8 *)malloc(outsize);
	uint8 *dst = sfx;
	src += 4;

	if (compressed) {
		for (uint32 i = outsize; i;) {
			uint8 cnt = *src++;
			if (cnt & 0x80) {
				cnt &= 0x7F;
				memset(dst, *src++, cnt);
			} else {
				memcpy(dst, src, cnt);
				src += cnt;
			}
			dst += cnt;
			i -= cnt;
		}
	} else {
		memcpy(dst, src, outsize);
	}

	for (uint32 i = 0; i < outsize; i++) {
		uint8 cmd = sfx[i];
		if (cmd & 0x80) {
			cmd = ~cmd;
		} else {
			cmd |= 0x80;
			if (cmd == 0xFF)
				cmd--;
		}
		if (cmd < 0x80)
			cmd = 0x80 - cmd;
		sfx[i] = cmd;
	}

	_currentSFX = Audio::makeRawStream(sfx, outsize, sfxRate * 10, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundChannels[h].handle, _currentSFX, -1, volume);
	_soundChannels[h].priority = priority;
	if (handle)
		*handle = _soundChannels[h].handle;

	delete[] data;
	return 1;
}

void SoundTownsPC98_v2::playSoundEffect(uint16 track, uint8) {
	if (!_useFmSfx || !_sfxTrackData)
		return;

	_driver->loadSoundEffectData(_sfxTrackData, track);
}

void SoundTownsPC98_v2::updateVolumeSettings() {
	if (!_driver)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

} // End of namespace Kyra

