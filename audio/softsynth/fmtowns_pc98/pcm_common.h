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

#ifndef PCM_COMMON_H
#define PCM_COMMON_H

#include "common/scummsys.h"

// The SegaCD and the FM-Towns have (almost) the same PCM sound chip. And while each platform has low-level driver
// stuff going on top of that it still makes sense to identify and abstract some commmon code.

class PCMChannel_Base {
public:
	PCMChannel_Base();
	virtual ~PCMChannel_Base();

	virtual void clear();

	void updateOutput();
	int32 currentSampleLeft();
	int32 currentSampleRight();

	virtual bool isPlaying() const = 0;
	bool isActive() const;
	void activate();
	void deactivate();

protected:
	void setData(const int8 *data, uint32 dataEnd, uint32 dataStart = 0);
	void setVolume(uint8 vol);
	void setPanPos(uint8 setPanPos);
	void setupLoop(uint32 loopStart, uint32 loopLen);
	void setRate(uint16 rate);

private:
	virtual void stopInternal() = 0;

	uint8 _panLeft;
	uint8 _panRight;
	uint8 _vol;
	bool _activeOutput;

	uint32 _loopStart;
	uint32 _loopLen;
	uint32 _dataEnd;
	uint32 _pos;
	uint16 _step;
	const int8 *_data;
};

class PCMDevice_Base {
public:
	PCMDevice_Base(int samplingRate, int deviceVolume, int numChannels);
	~PCMDevice_Base();

	void assignChannel(uint8 id, PCMChannel_Base *const chan);
	void setMusicVolume(uint16 vol);
	void setSfxVolume(uint16 vol);
	void setSfxChanMask(int mask);

	void readBuffer(int32 *buffer, uint32 bufferSize);

private:
	const uint32 _intRate;
	const uint32 _extRate;
	const int _deviceVolume;
	uint32 _timer;

	uint16 _musicVolume;
	uint16 _sfxVolume;
	int _pcmSfxChanMask;

	PCMChannel_Base **_channels;
	const int _numChannels;
};

#endif
