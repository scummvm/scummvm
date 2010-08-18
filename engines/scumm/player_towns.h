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

#ifndef SCUMM_PLAYER_TOWNS_H
#define SCUMM_PLAYER_TOWNS_H

#include "scumm/scumm.h"
#include "scumm/music.h"
#include "sound/softsynth/fmtowns_pc98/towns_euphony.h"

namespace Scumm {

class Player_Towns : public MusicEngine {
public:
	Player_Towns(ScummEngine *vm, Audio::Mixer *mixer);
	virtual ~Player_Towns();

	bool init();

	void setMusicVolume(int vol);
	void setSfxVolume(int vol);
	void startSound(int sound);
	void stopSound(int sound);
	void stopAllSounds();

	int getSoundStatus(int sound) const;
	int getCurrentCdaSound() { return _cdaCurrentSound; } 
	int getCurrentCdaVolume() { return (_cdaVolLeft + _cdaVolRight + 1) >> 1; } 

	virtual int32 doCommand(int numargs, int args[]);

	void setVolumeCD(int left, int right);
	void setSoundVolume(int sound, int left, int right);
	void setSoundNote(int sound, int note);

	void saveLoadWithSerializer(Serializer *ser);
	void restoreAfterLoad();

	TownsEuphonyDriver *driver() { return _driver; }

protected:
	virtual int getNextFreePcmChannel(int sound, int sfxChanRelIndex);

private:
	void restartLoopingSounds();
	void startSoundEx(int sound, int velo, int pan, int note);
	void stopSoundSuspendLooping(int sound);

	void playEuphonyTrack(int sound, const uint8 *data);
	void playPcmTrack(int sound, const uint8 *data, int velo = 0, int pan = 64, int note = 0);
	void playCdaTrack(int sound, const uint8 *data, bool skipTrackVelo = false);

	void stopPcmTrack(int sound);

	uint8 _cdaVolLeft;
	uint8 _cdaVolRight;

	struct SoundOvrParameters {
		uint8 vLeft;
		uint8 vRight;
		uint8 note;
	};

	SoundOvrParameters *_soundOverride;
	SoundOvrParameters _ovrCur;
	
	uint8 _unkFlags;

	struct PcmCurrentSound {
		uint16 index;
		uint16 chan;
		uint8 note;
		uint8 velo;
		uint8 pan;
		uint8 paused;
		uint8 looping;
		uint32 priority;
	} _pcmCurrentSound[9];

	uint8 _eupCurrentSound;
	uint8 _eupLooping;
	uint8 _eupVolLeft;
	uint8 _eupVolRight;

	uint8 _cdaCurrentSound;
	uint8 _cdaNumLoops;
	uint8 _cdaForceRestart;

	uint8 _cdaCurrentSoundTemp;
	uint8 _cdaNumLoopsTemp;

	TownsEuphonyDriver *_driver;
	ScummEngine *_vm;
};

} // End of namespace Scumm

#endif
