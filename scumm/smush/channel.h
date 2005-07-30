/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#ifndef SMUSH_CHANNEL_H
#define SMUSH_CHANNEL_H

#include "common/util.h"

namespace Scumm {

class Chunk;
class ContChunk;

class SmushChannel {
public:

	virtual ~SmushChannel() {};
	virtual bool appendData(Chunk &b, int32 size) = 0;
	virtual bool setParameters(int32, int32, int32, int32, int32) = 0;
	virtual bool checkParameters(int32, int32, int32, int32, int32) = 0;
	virtual bool isTerminated() const = 0;
	virtual int32 availableSoundData() const = 0;
	virtual void getSoundData(int16 *sound_buffer, int32 size) = 0;
	virtual void getSoundData(int8 *sound_buffer, int32 size) = 0;
	virtual int32 getRate() = 0;
	virtual bool getParameters(int32 &rate, bool &stereo, bool &is_16bit, int32 &vol, int32 &pan) = 0;
	virtual int32 getTrackIdentifier() const = 0;
};

class SaudChannel : public SmushChannel {
private:
	int32 _track;
	int32 _nbframes;
	int32 _dataSize;
	int32 _frequency;
	bool _inData;
	bool _markReached;
	int32 _flags;
	int32 _volume;
	int32 _pan;
	int32 _index;
	byte *_tbuffer;
	int32 _tbufferSize;
	byte *_sbuffer;
	int32 _sbufferSize;
	bool _keepSize;

protected:
	void handleStrk(Chunk &c);
	void handleSmrk(Chunk &c);
	void handleShdr(Chunk &c);
	bool handleSubTags(int32 &offset);
	bool processBuffer();

public:
	SaudChannel(int32 track, int32 freq);
	virtual ~SaudChannel();
	bool isTerminated() const;
	bool setParameters(int32 duration, int32 flags, int32 vol1, int32 vol2, int32 index);
	bool checkParameters(int32 index, int32 duration, int32 flags, int32 vol1, int32 vol2);
	bool appendData(Chunk &b, int32 size);
	int32 availableSoundData() const;
	void getSoundData(int16 *sound_buffer, int32 size);
	void getSoundData(int8 *sound_buffer, int32 size) { error("8bit request for SAUD channel should never happen"); };
	int32 getRate() { return _frequency; }
	bool getParameters(int32 &rate, bool &stereo, bool &is_16bit, int32 &vol, int32 &pan) {
		rate = _frequency;
		stereo = true;
		is_16bit = true;
		vol = _volume;
		pan = _pan;
		return true;
	};
	virtual int32 getTrackIdentifier() const { return _track; };
};

class ImuseChannel : public SmushChannel {
private:
	int32 _track;				//!< the track number
	byte *_tbuffer;	//!< data temporary buffer
	int32 _tbufferSize;			//!< temporary buffer size
	byte *_sbuffer;	//!< sound buffer
	int32 _sbufferSize;			//!< sound buffer size
	int32 _srbufferSize;
	int32 _frequency;			//!< the target frequency of the ::mixer
	int32 _dataSize;			//!< remaining size of sound data in the iMUS buffer
	bool _inData;
	int32 _volume;
	int32 _pan;

	int32 _bitsize;			//!< the bitsize of the original data
	int32 _rate;				//!< the sampling rate of the original data
	int32 _channels;			//!< the number of channels of the original data

protected:
	int32 decode(int32 size, int32 &ret);
	void decode();
	bool processBuffer();
	bool handleMap(Chunk &);
	bool handleFormat(Chunk &);
	bool handleText(Chunk &);
	bool handleRegion(Chunk &);
	bool handleStop(Chunk &);
	bool handleSubTags(int32 & offset);

public:
	ImuseChannel(int32 track, int32 freq);
	virtual ~ImuseChannel();
	bool isTerminated() const;
	bool setParameters(int32 nbframes, int32 size, int32 track_flags, int32 unk1, int32);
	bool checkParameters(int32 index, int32 nbframes, int32 size, int32 track_flags, int32 unk1);
	bool appendData(Chunk &b, int32 size);
	int32 availableSoundData() const;
	void getSoundData(int16 *sound_buffer, int32 size);
	void getSoundData(int8 *sound_buffer, int32 size);
	int32 getRate() { return _rate; }
	bool getParameters(int32 &rate, bool &stereo, bool &is_16bit, int32 &vol, int32 &pan) {
		rate = _rate;
		stereo = (_channels == 2);
		is_16bit = (_bitsize > 8);
		vol = _volume;
		pan = _pan;
		return true;
	};
	virtual int32 getTrackIdentifier() const { return _track; };
};

} // End of namespace Scumm

#endif
