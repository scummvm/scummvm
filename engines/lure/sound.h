/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef LURE_SOUND_H
#define LURE_SOUND_H

#include "lure/luredefs.h"
#include "lure/disk.h"
#include "lure/memory.h"
#include "common/singleton.h"

namespace Lure {

#define NUM_CHANNELS 8

class SoundManager: public Common::Singleton<SoundManager> {
private:
	MemoryBlock *_descs;
	int _numDescs;
	SoundDescResource *soundDescs() { return (SoundDescResource *) _descs->data(); }
	ManagedList<SoundDescResource *> _activeSounds;
	byte _channels[NUM_CHANNELS];

	void bellsBodge();
public:
	SoundManager();

	void killSounds();
	void addSound(uint8 soundIndex, bool tidyFlag = true);
	void addSound2(uint8 soundIndex);
	void stopSound(uint8 soundIndex);
	void killSound(uint8 soundNumber);
	void setVolume(uint8 soundNumber, uint8 volume);
	void tidySounds();
	SoundDescResource *findSound(uint8 soundNumber);
	void removeSounds();
	void restoreSounds();

	// The following methods implement the external sound player module
	void musicInterface_Play(uint8 soundNumber, bool isEffect, uint8 channelNumber);
	void musicInterface_Stop(uint8 soundNumber);
	bool musicInterface_CheckPlaying(uint8 soundNumber);
	void musicInterface_SetVolume(uint8 channelNum, uint8 volume);
	void musicInterface_KillAll();
	void musicInterface_ContinuePlaying();
	void musicInterface_TrashReverb();
};

} // End of namespace Lure

#define Sound (::Lure::SoundManager::instance())

#endif
