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

#ifndef CRUISE_SOUND_H
#define CRUISE_SOUND_H

#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "sound/mixer.h"
#include "common/serializer.h"

namespace Cruise {

class CruiseEngine;
class PCSoundDriver;
class PCSoundFxPlayer;

class PCSound {
private:
	Audio::Mixer *_mixer;
	CruiseEngine *_vm;
protected:
	PCSoundDriver *_soundDriver;
	PCSoundFxPlayer *_player;
public:
	PCSound(Audio::Mixer *mixer, CruiseEngine *vm);
	virtual ~PCSound();

	virtual void loadMusic(const char *name);
	virtual void playMusic();
	virtual void stopMusic();
	virtual void removeMusic();
	virtual void fadeOutMusic();

	virtual void playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat);
	virtual void startSound(int channelNum, const byte *ptr, int size, int speed, int volume, bool loop);
	virtual void stopSound(int channel);
	
	void doSync(Common::Serializer &s);
	const char *musicName();
	void stopChannel(int channel);
	bool isPlaying() const;
	bool songLoaded() const;
	bool songPlayed() const;
	void fadeSong();
	uint8 numOrders() const;
	void setNumOrders(uint8 v);
	void setPattern(int offset, uint8 value);
	bool musicLooping() const;
	void musicLoop(bool v);
	void startNote(int channel, int volume, int speed);
	void setVolume(int volume);
	uint8 getVolume();
};

} // End of namespace Cruise

#endif
