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

#ifndef PLAYER_MOD_H
#define PLAYER_MOD_H

#include "scumm/scumm.h"
#include "sound/mixer.h"
#include "sound/audiostream.h"
#include "sound/rate.h"

namespace Scumm {

/**
 * Generic Amiga MOD mixer - provides a 60Hz 'update' routine.
 */
class Player_MOD {
public:
	Player_MOD(ScummEngine *scumm);
	virtual ~Player_MOD();
	virtual void setMasterVolume(int vol);

	virtual void startChannel(int id, void *data, int size, int rate, uint8 vol, int loopStart = 0, int loopEnd = 0, int8 pan = 0);
	virtual void stopChannel(int id);
	virtual void setChannelVol(int id, uint8 vol);
	virtual void setChannelPan(int id, int8 pan);
	virtual void setChannelFreq(int id, int freq);

	typedef void ModUpdateProc(void *param);

	virtual void setUpdateProc(ModUpdateProc *proc, void *param, int freq);
	virtual void clearUpdateProc();

private:
	enum {
		MOD_MAXCHANS = 16
	};

	struct soundChan
	{
		int id;
		uint8 vol;
		int8 pan;
		uint16 freq;
		void *ptr;
		RateConverter *converter;
		AudioInputStream *input;
	};

	SoundMixer *_mixer;

	uint32 _mixamt;
	uint32 _mixpos;
	int _samplerate;

	soundChan _channels[MOD_MAXCHANS];

	uint8 _maxvol;

	static void premix_proc(void *param, int16 *buf, uint len);
	virtual void do_mix(int16 *buf, uint len);
	
	ModUpdateProc *_playproc;
	void *_playparam;
};

} // End of namespace Scumm

#endif
