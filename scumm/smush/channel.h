/* ScummVM - Scumm int32erpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef CHANNEL_H
#define CHANNEL_H

#include "config.h"
#include "common/engine.h" // for debug, warning, error

#ifdef DEBUG
# ifndef NO_DEBUG_CHANNEL
#  define DEBUG_CHANNEL
# endif
#else
# ifdef DEBUG_CHANNEL
#  error DEBUG_CHANNEL defined without DEBUG
# endif
#endif

class Chunk;
class ContChunk;
	
/*! 	@brief int32erface for a sound channel (a track)

	This is the int32erface for sound channels. 
*/
class _Channel {
public:
	virtual ~_Channel() {};
	// called by the smush_player
	virtual bool appendData(Chunk &b, int32 size) = 0;
	virtual bool setParameters(int32, int32, int32, int32) = 0;
	virtual bool checkParameters(int32, int32, int32, int32, int32) = 0;
	// called by the mixer
	virtual bool isTerminated() const = 0;
	virtual int32 availableSoundData() const = 0;
	virtual void getSoundData(int16 *sound_buffer, int32 size) = 0; // size is in sample 
	virtual void getSoundData(int8 *sound_buffer, int32 size) = 0;
	virtual int32 getRate() = 0;
	virtual bool getParameters(int32 &rate, bool &stereo, bool &is_16bit) = 0;
	virtual int32 getTrackIdentifier() const = 0;
};

class SaudChannel : public _Channel {
private:
	int32 _track;				//!< The track identifier
	int32 _nbframes;			//!< number of frames of the track (unused)
	int32 _dataSize;			//!< the size of the sound buffer
	int32 _frequency;			//!< the frequency target of the track (always 22050)
	bool _inData;			//!< are we processing data ?
	bool _markReached;		//!< set to \c true when the SMRK tag is reached
	int32 _flags;				//!< current flags of the track (unused)
	int32 _volume;			//!< the current track volume
	int32 _balance;			//!< the current track balance
	int32 _index;				//!< the current PSAD index (for coherency checking)
	int16 _voltable[2][256];	//!< the precalculated volume table (stereo 16 bits)
	byte *_tbuffer;	//!< data temporary buffer
	int32 _tbufferSize;			//!< temporary buffer size
	byte *_sbuffer;	//!< sound buffer
	int32 _sbufferSize;			//!< sound buffer size

protected:
	void handleStrk(Chunk &c);
	void handleSmrk(Chunk &c);
	void handleShdr(Chunk &c);
	bool handleSubTags(int32 &offset);
	bool processBuffer();
	void recalcVolumeTable();

public:
	SaudChannel(int32 track, int32 freq);
	virtual ~SaudChannel();
	bool isTerminated() const;
	bool setParameters(int32 duration, int32 flags, int32 vol1, int32 vol2);
	bool checkParameters(int32 index, int32 duration, int32 flags, int32 vol1, int32 vol2);
	bool appendData(Chunk &b, int32 size);
	int32 availableSoundData() const;
	void getSoundData(int16 *sound_buffer, int32 size);
	void getSoundData(int8 *sound_buffer, int32 size) { error("16bit request for SAUD channel should never happen"); };
	int32 getRate() { return _frequency; }
	bool getParameters(int32 &rate, bool &stereo, bool &is_16bit) { 
		rate = _frequency;
		stereo = true;
		is_16bit = true;
		return true;
	};
	virtual int32 getTrackIdentifier() const { return _track; };
};

/*! 	@brief class for a IACT sound ::channel (a The Dig track)

	This class implements a channel specifically for The Dig.

	\bug for unknown reason, some sound have a too long duration, or repeat themselves.
*/
class ImuseChannel : public _Channel {
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
	bool setParameters(int32 nbframes, int32 size, int32 track_flags, int32 unk1);
	bool checkParameters(int32 index, int32 nbframes, int32 size, int32 track_flags, int32 unk1);
	bool appendData(Chunk &b, int32 size);
	int32 availableSoundData() const;
	void getSoundData(int16 *sound_buffer, int32 size);
	void getSoundData(int8 *sound_buffer, int32 size);
	int32 getRate() { return _rate; }
	bool getParameters(int32 &rate, bool &stereo, bool &is_16bit) {
		rate = _frequency;
		stereo = (_channels == 2);
		is_16bit = (_bitsize > 8);
		return true;
	};
	virtual int32 getTrackIdentifier() const { return _track; };
};

#endif
