/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef SMUSH_MIXER_H
#define SMUSH_MIXER_H

#include "stdafx.h"
#include "sound/mixer.h"

namespace Scumm {

class SmushChannel;

class SmushMixer {
	enum {
		NUM_CHANNELS = 16
	};
private:

	SoundMixer *_mixer;
	struct channels {
		int id;
		SmushChannel *chan;
		PlayingSoundHandle handle;
	} _channels[NUM_CHANNELS];

	int _soundFrequency;

public:

	SmushMixer(SoundMixer *);
	virtual ~SmushMixer();
	SmushChannel *findChannel(int32 track);
	void addChannel(SmushChannel *c);
	bool handleFrame();
	bool stop();
	bool update();
};

} // End of namespace Scumm

#endif
