/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Sound resource class header

#ifndef SAGA_SNDRES_H_
#define SAGA_SNDRES_H_

#include "saga/itedata.h"
#include "saga/sound.h"

namespace Saga {

class SndRes {
public:

	SndRes(SagaEngine *vm);
	~SndRes();

	int loadSound(uint32 resourceId);
	void playSound(uint32 resourceId, int volume, bool loop);
	void playVoice(uint32 resourceId);
	int getVoiceLength(uint32 resourceId);
	void setVoiceBank(int serial);

	FxTable *_fxTable;
	int _fxTableLen;

	int16 *_fxTableIDs;
	int _fxTableIDsLen;

 private:
	bool load(ResourceContext *context, uint32 resourceId, SoundBuffer &buffer, bool onlyHeader);
	bool loadVocSound(byte *soundResource, size_t soundResourceLength, SoundBuffer &buffer);
	bool loadWavSound(byte *soundResource, size_t soundResourceLength, SoundBuffer &buffer);

	ResourceContext *_sfxContext;
	ResourceContext *_voiceContext;

	int _voiceSerial; // voice bank number

	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
