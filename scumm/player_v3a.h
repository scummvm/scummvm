/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * /scummvm/scummvm/scumm/player_v3a.h
 *
 */

#ifndef PLAYER_V3A_H
#define PLAYER_V3A_H

#include "common/scummsys.h"
#include "common/system.h"
#include "scumm/music.h"

#define V3A_MAXCHANS 8

class Scumm;
class SoundMixer;

class Player_V3A : public MusicEngine  {
public:
	Player_V3A(Scumm *scumm);
	virtual ~Player_V3A();

	virtual void set_master_volume(int vol);

	virtual void startSound(int nr);
	virtual void stopSound(int nr);
	virtual void stopAllSounds();
	virtual int  getMusicTimer() const;

	virtual void playMusic();
	virtual int  getSoundStatus(int nr) const;

protected:
	SoundMixer *_mixer;
	OSystem *_system;
	Scumm *_scumm;

	uint16 _soundID[V3A_MAXCHANS];
	uint16 _timeleft[V3A_MAXCHANS];
	uint8 _maxvol;

	int _curSong;
	unsigned char *_songData;
	uint16 _songPtr;
	uint16 _songDelay;
	uint8 _lastSample;
	int _music_timer;
	bool _isinit;

	struct instData
	{
		char *_idat[6];
		uint16 _ilen[6];
		char *_ldat[6];
		uint16 _llen[6];
		uint16 _oct[6];
	} **_wavetable;

	void playSound (int nr, char *data, int size, int rate, int vol, int tl, bool looped, int loopStart = 0, int loopEnd = 0);
};

#endif
