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
 * $Header$
 *
 */

#ifndef PLAYER_V3A_H
#define PLAYER_V3A_H

#include "common/scummsys.h"
#include "common/system.h"
#include "scumm/music.h"
#include "scumm/player_mod.h"

class SoundMixer;

namespace Scumm {

#define	V3A_MAXMUS	8
#define	V3A_MAXSFX	8

class ScummEngine;

class Player_V3A : public MusicEngine {
public:
	Player_V3A(ScummEngine *scumm);
	virtual ~Player_V3A();

	virtual void setMasterVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer() const;
	virtual int  getSoundStatus(int sound) const;

private:
	OSystem *_system;
	ScummEngine *_scumm;
	Player_MOD *_mod;

	struct musChan
	{
		int id;
		int dur;
	} _mus[V3A_MAXMUS];
	int getMusChan (int id = 0) const;

	struct sfxChan
	{
		int id;
		int dur;
		// SFX will eventually have pitch bends
	} _sfx[V3A_MAXSFX];
	int getSfxChan (int id = 0) const;

	int _curSong;
	uint8 *_songData;
	uint16 _songPtr;
	uint16 _songDelay;
	int _music_timer;
	bool _isinit;

	struct instData
	{
		char *_idat[6];
		uint16 _ilen[6];
		char *_ldat[6];
		uint16 _llen[6];
		uint16 _oct[6];
		int16 _pitadjust;
	} **_wavetable;

	static void update_proc(void *param);
	void playMusic();
};

} // End of namespace Scumm

#endif
