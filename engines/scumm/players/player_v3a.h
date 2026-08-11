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

#ifndef SCUMM_PLAYERS_PLAYER_V3A_H
#define SCUMM_PLAYERS_PLAYER_V3A_H

#include "common/scummsys.h"
#include "common/util.h"
#include "scumm/music.h"
#include "audio/mixer.h"
#include "audio/mods/paula.h"

class Mixer;

namespace Scumm {

class ScummEngine;

/**
 * Scumm V3 Amiga sound/music driver.
 */
class Player_V3A : public MusicEngine, Audio::Paula {
public:
	Player_V3A(ScummEngine *scumm, Audio::Mixer *mixer);
	~Player_V3A() override;

	// MusicEngine API
	void setMusicVolume(int vol) override;
	void startSound(int sound) override;
	void stopSound(int sound) override;
	void stopAllSounds() override;
	int  getSoundStatus(int sound) const override;
	int  getMusicTimer() override;

protected:
	// Paula API
	void interrupt() override;
	void interruptChannel(byte channel) override;

private:
	struct SndChan {
		int period;	/* 16.16 fixed point */
		int volume;	/* 8.8 fixed point */
		int loopCount;	/* decrement once per loop, halt playback upon reaching zero */
		int sweepRate;	/* add to period once per frame */
		int haltTimer;	/* decrement once per frame, halt playback upon reaching zero */
		int fadeRate;	/* if haltTimer is zero, subtract 0x100*this from volume once per frame */

		int resourceId;
		int priority;

		// Both of these are used exclusively by the Loom music engine
		int instrument;
		int canOverride;
	};

	struct InstData {
		int8 *mainData[6];
		uint16 mainLen[6];
		int8 *loopData[6];
		uint16 loopLen[6];
		int16 octave[6];
		int16 pitchAdjust;
		int16 volumeFade;
	};

	ScummEngine *const _vm;
	Audio::Mixer *const _mixer;
	Audio::SoundHandle _soundHandle;

	SndChan _channels[4];

	const int SFX_CHANNEL_MAP[2][2] = {
		{ 0, 1 },
		{ 3, 2 }
	};
	const uint16 NOTE_FREQS[4][12] = {
		{0x06B0, 0x0650, 0x05F4, 0x05A0, 0x054C, 0x0500, 0x04B8, 0x0474, 0x0434, 0x03F8, 0x03C0, 0x0388},
		{0x0358, 0x0328, 0x02FA, 0x02D0, 0x02A6, 0x0280, 0x025C, 0x023A, 0x021A, 0x01FC, 0x01E0, 0x01C4},
		{0x01AC, 0x0194, 0x017D, 0x0168, 0x0153, 0x0140, 0x012E, 0x011D, 0x010D, 0x00FE, 0x00F0, 0x00E2},
		{0x00D6, 0x00CA, 0x00BE, 0x00B4, 0x00A9, 0x00A0, 0x0097, 0x008E, 0x0086, 0x007F, 0x00F0, 0x00E2}
	};

	int _curSong;
	int8 *_songData;
	uint16 _songPtr;
	uint16 _songDelay;
	int _musicTimer;

	enum {
		kInitStateFailed = -1,
		kInitStateNotReady = 0,
		kInitStateReady = 1
	} _initState;

	int8 *_wavetableData;
	InstData *_wavetablePtrs;

	void updateMusicIndy();
	void updateMusicLoom();
	void updateSounds();
	void startNote(int channel, int instrument, int pitch, int volume, int duration);

	bool init();
};

} // End of namespace Scumm

#endif
