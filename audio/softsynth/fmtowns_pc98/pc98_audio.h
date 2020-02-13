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

#ifndef PC98_AUDIO_H
#define PC98_AUDIO_H

#include "common/scummsys.h"

namespace Audio {
class Mixer;
}

class PC98AudioCoreInternal;
class PC98AudioPluginDriver {
public:
	enum EmuType {
		kTypeTowns = 0,
		kType26 = 1,
		kType86 = 2
	};

	virtual ~PC98AudioPluginDriver() {}
	virtual void timerCallbackA() {}
	virtual void timerCallbackB() {}
};

class PC98AudioCore {
public:
	PC98AudioCore(Audio::Mixer *mixer, PC98AudioPluginDriver *driver, PC98AudioPluginDriver::EmuType type);
	~PC98AudioCore();

	bool init();
	void reset();

	void writeReg(uint8 part, uint8 regAddress, uint8 value);
	uint8 readReg(uint8 part, uint8 regAddress);

	void writePort(uint16 port, uint8 value);
	uint8 readPort(uint16 port);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

	// Defines the channels used as sound effect channels for the purpose of ScummVM GUI volume control.
	// The first 6 bits are the 6 fm channels. The next 3 bits are ssg channels. The next bit is the rhythm channel.
	void setSoundEffectChanMask(int mask);

	void ssgSetVolume(int volume);

	class MutexLock {
		friend class PC98AudioCore;
	public:
		~MutexLock();
	private:
		MutexLock(PC98AudioCoreInternal *pc98int, int reverse = 0);
		PC98AudioCoreInternal *_pc98int;
		int _count;
	};

	MutexLock stackLockMutex();
	MutexLock stackUnlockMutex();

private:
	PC98AudioCoreInternal *_internal;
};

#endif
