/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SEGA_AUDIO_H
#define SEGA_AUDIO_H

#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/stream.h"

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
	void playPCMChannel(uint8 channel, uint8 dataStart, uint16 loopStart, uint16 rate, uint8 pan, uint8 vol);
	void stopPCMChannel(uint8 channel);
	// Convenience function. The chip only allows to read out the data position and derive the state from that.
	// But this wouldn't work with our stream play function, since it requires tracking the stream state, too.
	// So, we just return the data that we actually want...
	bool isPCMChannelPlaying(uint8 channel) const;

	// Convenience function, since it is so much easier to implement this on this level than having the samples buffer
	// invoke a callback into the engine
	// @ memstart: same 8-bit format as dataStart (in playPCMChannel), begin of the buffer in the PCM memory area
	// @ memAreaSize: byte size of the buffer in the PCM memory area
	void playPCMStream(Common::SharedPtr<Common::SeekableReadStream> &stream, uint8 memStart, uint16 memAreaSize, uint8 channel, uint16 rate, uint8 pan, uint8 vol);

	// OPN registers
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
