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

#ifndef SEGA_AUDIO_H
#define SEGA_AUDIO_H

#include "common/scummsys.h"

namespace Audio {
class Mixer;
}

class SegaAudioInterfaceInternal;
class SegaAudioPluginDriver {
public:
	virtual ~SegaAudioPluginDriver() {}
	virtual void timerCallback60Hz() {}
	virtual void timerCallbackA() {}
	virtual void timerCallbackB() {}
};

class SegaAudioInterface {
public:
	SegaAudioInterface(Audio::Mixer *mixer, SegaAudioPluginDriver *driver);
	~SegaAudioInterface();

	bool init();
	void reset();

	void loadPCMData(uint16 address, const uint8 *data, uint16 dataSize);
	void playPCMChannel(uint8 channel, uint8 dataStart, uint16 loopStart, uint16 rate, uint8 pan, uint8 env);
	void stopPCMChannel(uint8 channel);

	void writeReg(uint8 part, uint8 regAddress, uint8 value);
	uint8 readReg(uint8 part, uint8 regAddress);

	void psgWrite(uint8 data);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

	// Defines the channels used as sound effect channels for the purpose of ScummVM GUI volume control.
	// The first 6 bits are 6 fm channels. The next 3 bits are psg channels. The bits that follow represent pcm channels.
	void setSoundEffectChanMask(int mask);

	class MutexLock {
		friend class SegaAudioInterface;
	public:
		~MutexLock();
	private:
		MutexLock(SegaAudioInterfaceInternal *saii, int reverse = 0);
		SegaAudioInterfaceInternal *_saii;
		int _count;
	};

	MutexLock stackLockMutex();
	MutexLock stackUnlockMutex();

private:
	SegaAudioInterfaceInternal *_internal;
};

#endif
