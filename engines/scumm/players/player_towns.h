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

#ifndef SCUMM_PLAYERS_PLAYER_TOWNS_H
#define SCUMM_PLAYERS_PLAYER_TOWNS_H

#include "scumm/scumm.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse/drivers/fmtowns.h"
#include "audio/softsynth/fmtowns_pc98/towns_euphony.h"

namespace Scumm {

class Player_Towns : public MusicEngine {
public:
	Player_Towns(ScummEngine *vm, bool isVersion2);
	~Player_Towns() override {}

	virtual bool init() = 0;

	void setSfxVolume(int vol);

	int getSoundStatus(int sound) const override;

	virtual int32 doCommand(int numargs, int args[]) = 0;

	void saveLoadWithSerializer(Common::Serializer &ser) override;
	virtual void restoreAfterLoad();

	// version 1 specific
	virtual int getCurrentCdaSound() { return 0; }
	virtual int getCurrentCdaVolume() { return 0; }
	virtual void setVolumeCD(int left, int right) {}
	virtual void setSoundVolume(int sound, int left, int right) {}
	virtual void setSoundNote(int sound, int note) {}

protected:
	void playPcmTrack(int sound, const uint8 *data, int velo = 0, int pan = 64, int note = 0, int priority = 0);
	void stopPcmTrack(int sound);

	int allocatePcmChannel(int sound, int sfxChanRelIndex, uint32 priority);

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
	friend void syncWithSerializer(Common::Serializer &, PcmCurrentSound &);

	uint8 _unkFlags;

	TownsAudioInterface *_intf;
	ScummEngine *_vm;

	const int _numSoundMax;
	const bool _v2;
};

class Player_Towns_v1 : public Player_Towns {
public:
	Player_Towns_v1(ScummEngine *vm, Audio::Mixer *mixer);
	~Player_Towns_v1() override;

	bool init() override;

	void setMusicVolume(int vol) override;
	void startSound(int sound) override;
	void stopSound(int sound) override;
	void stopAllSounds() override;

	int getSoundStatus(int sound) const override;
	int getCurrentCdaSound() override { return _cdaCurrentSound; }
	int getCurrentCdaVolume() override { return (_cdaVolLeft + _cdaVolRight + 1) >> 1; }

	int32 doCommand(int numargs, int args[]) override;

	void setVolumeCD(int left, int right) override;
	void setSoundVolume(int sound, int left, int right) override;
	void setSoundNote(int sound, int note) override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;
	void restoreAfterLoad() override;

private:
	void restartLoopingSounds();
	void startSoundEx(int sound, int velo, int pan, int note);
	void stopSoundSuspendLooping(int sound);

	void playEuphonyTrack(int sound, const uint8 *data);
	void playCdaTrack(int sound, const uint8 *data, bool skipTrackVelo = false);

	struct SoundOvrParameters {
		uint8 vLeft;
		uint8 vRight;
		uint8 note;
	};

	SoundOvrParameters *_soundOverride;

	uint8 _cdaVolLeft;
	uint8 _cdaVolRight;

	uint8 _eupCurrentSound;
	uint8 _eupLooping;
	uint8 _eupVolLeft;
	uint8 _eupVolRight;

	uint8 _cdaCurrentSound;
	uint8 _cdaNumLoops;
	uint8 _cdaForceRestart;

	uint8 _cdaCurrentSoundTemp;
	uint8 _cdaNumLoopsTemp;

	EuphonyPlayer *_player;
};

class Player_Towns_v2 : public Player_Towns {
public:
	Player_Towns_v2(ScummEngine *vm, Audio::Mixer *mixer, IMuse *imuse, bool disposeIMuse);
	~Player_Towns_v2() override;

	bool init() override;

	void setMusicVolume(int vol) override;

	int getSoundStatus(int sound) const override;
	void startSound(int sound) override;
	void stopSound(int sound) override;
	void stopAllSounds() override;

	int32 doCommand(int numargs, int args[]) override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;

private:
	void playVocTrack(const uint8 *data);

	struct SoundOvrParameters {
		uint8 velo;
		uint8 pan;
		uint8 type;
	};

	SoundOvrParameters *_soundOverride;

	uint8 *_sblData;

	IMuse *_imuse;
	const bool _imuseDispose;
};

} // End of namespace Scumm

#endif
