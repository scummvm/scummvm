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
 */

#ifndef PEGASUS_SOUND_H
#define PEGASUS_SOUND_H

#include "common/scummsys.h"
#include "common/str.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "pegasus/pegasus.h"

namespace Pegasus {

#define SOUND_HANDLES 10

enum sndHandleType {
	kFreeHandle,
	kUsedHandle
};

struct SndHandle {
	Audio::SoundHandle handle;
	sndHandleType type;
};

class PegasusEngine;

class SoundManager {
public:
	SoundManager(PegasusEngine *vm);

	void playSound(Common::String filename, bool loop = false);
	void stopSound();
	void pauseSound();
	void resumeSound();
	bool isPlaying();
	
private:
	PegasusEngine *_vm;

	SndHandle _handles[SOUND_HANDLES];
	SndHandle *getHandle();
};

} // End of namespace Pegasus

#endif
