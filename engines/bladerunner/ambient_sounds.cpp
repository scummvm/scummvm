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

#include "bladerunner/ambient_sounds.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/gameinfo.h"

#include "common/debug.h"
#include "common/system.h"

namespace BladeRunner {

#define NON_LOOPING_SOUNDS 25
#define LOOPING_SOUNDS      3

AmbientSounds::AmbientSounds(BladeRunnerEngine *vm) : _vm(vm) {
	_nonLoopingSounds = new NonLoopingSound[NON_LOOPING_SOUNDS];
	_loopingSounds = new LoopingSound[LOOPING_SOUNDS];
	_ambientVolume = 65;

	for (int i = 0; i != NON_LOOPING_SOUNDS; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];
		track.isActive = false;
	}

	for (int i = 0; i != LOOPING_SOUNDS; ++i) {
		LoopingSound &track = _loopingSounds[i];
		track.isActive = false;
	}
}

AmbientSounds::~AmbientSounds() {
	delete[] _nonLoopingSounds;
	delete[] _loopingSounds;
}

static inline void sort(int &a, int &b) {
	if (a > b) {
		int t = a;
		a = b;
		b = t;
	}
}

void AmbientSounds::addSound(
		int id,
		int timeRangeBegin, int timeRangeEnd,
		int volumeRangeBegin, int volumeRangeEnd,
		int unk1RangeBegin, int unk1RangeEnd,
		int unk2RangeBegin, int unk2RangeEnd,
		int priority, int unk3) {
	const char *name = _vm->_gameInfo->getSfxTrack(id);

	sort(volumeRangeBegin, volumeRangeEnd);
	sort(unk1RangeBegin, unk1RangeEnd);
	sort(unk2RangeBegin, unk2RangeEnd);

	addSoundByName(
		name,
		timeRangeBegin, timeRangeEnd,
		volumeRangeBegin, volumeRangeEnd,
		unk1RangeBegin, unk1RangeEnd,
		unk2RangeBegin, unk2RangeEnd,
		priority, unk3
	);
}

void AmbientSounds::addLoopingSound(int sfx_id, int volume, int unk, int fadeInTime) {
	const char *name = _vm->_gameInfo->getSfxTrack(sfx_id);

	int32 hash = mix_id(name);

	if (findLoopingTrackByHash(hash) >= 0)
		return;

	int i = findAvailableLoopingTrack();
	if (i == -1)
		return;

	int actualVolume = volume * _ambientVolume / 100;

	int balance = 0;

	_vm->_audioPlayer->playAud(name, actualVolume, balance, balance, 100, AudioPlayer::LOOP | AudioPlayer::OVERRIDE_VOLUME);
}

void AmbientSounds::tick() {
	uint32 now = g_system->getMillis();

	for (int i = 0; i != NON_LOOPING_SOUNDS; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];

		if (!track.isActive || track.nextPlayTime > now)
			continue;

		int pan1, pan2;

		pan1 = _vm->_rnd.getRandomNumberRng(track.pan1begin, track.pan1end);
		if (track.pan2begin == -101) {
			pan2 = pan1;
		} else {
			pan2 = _vm->_rnd.getRandomNumberRng(track.pan2begin, track.pan2end);
		}

		track.volume = _vm->_rnd.getRandomNumberRng(track.volume1, track.volume2);

		track.audio_player_track = _vm->_audioPlayer->playAud(
			track.name,
			track.volume * _ambientVolume / 100,
			pan1, pan2,
			track.priority,
			AudioPlayer::OVERRIDE_VOLUME
		);

		track.nextPlayTime = now + _vm->_rnd.getRandomNumberRng(track.time1, track.time2);

	}
}

int AmbientSounds::findAvailableNonLoopingTrack() {
	for (int i = 0; i != NON_LOOPING_SOUNDS; ++i) {
		if (!_nonLoopingSounds[i].isActive)
			return i;
	}

	return -1;
}

int AmbientSounds::findNonLoopingTrackByHash(int32 hash) {
	for (int i = 0; i != NON_LOOPING_SOUNDS; ++i) {
		NonLoopingSound &track = _nonLoopingSounds[i];

		if (track.isActive && track.hash == hash)
			return i;
	}

	return -1;
}

int AmbientSounds::findAvailableLoopingTrack() {
	for (int i = 0; i != LOOPING_SOUNDS; ++i) {
		if (!_loopingSounds[i].isActive)
			return i;
	}

	return -1;
}

int AmbientSounds::findLoopingTrackByHash(int32 hash) {
	for (int i = 0; i != LOOPING_SOUNDS; ++i) {
		LoopingSound &track = _loopingSounds[i];

		if (track.isActive && track.hash == hash)
			return i;
	}

	return -1;
}

void AmbientSounds::addSoundByName(
		const char *name,
		int timeRangeBegin, int timeRangeEnd,
		int volumeRangeBegin, int volumeRangeEnd,
		int pan1begin, int pan1end,
		int pan2begin, int pan2end,
		int priority, int unk3) {
	if (strlen(name) > 12) {
		error("AmbientSounds::addSoundByName: Overlong name '%s'", name);
	}

	int i = findAvailableNonLoopingTrack();
	if (i < 0)
		return;

	NonLoopingSound &track = _nonLoopingSounds[i];

	uint32 now = g_system->getMillis();

	track.isActive      = true;
	strcpy(track.name, name);
	track.hash          = mix_id(name);
	track.time1         = 1000 * timeRangeBegin;
	track.time2         = 1000 * timeRangeEnd;
	track.nextPlayTime  = now + _vm->_rnd.getRandomNumberRng(track.time1, track.time2);
	track.volume1       = volumeRangeBegin;
	track.volume2       = volumeRangeEnd;
	track.volume        = 0;
	track.pan1begin     = pan1begin;
	track.pan1end       = pan1end;
	track.pan2begin     = pan2begin;
	track.pan2end       = pan2end;
	track.priority      = priority;
}

} // End of namespace BladeRunner
