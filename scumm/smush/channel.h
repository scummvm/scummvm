/* ScummVM - Scumm Interpreter
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

#ifndef __CHANNEL_H_
#define __CHANNEL_H_

#include "config.h"

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
	
/*! 	@brief interface for a sound channel (a track)

	This is the interface for sound channels. 
*/
class _Channel {
public:
	virtual ~_Channel() {};
	// called by the smush_player
	virtual bool appendData(Chunk & b, int size) = 0;
	virtual bool setParameters(int, int, int, int) = 0;
	virtual bool checkParameters(int, int, int, int, int) = 0;
	// called by the mixer
	virtual bool isTerminated() const = 0;
	virtual int availableSoundData() const = 0;
	virtual void getSoundData(short * sound_buffer, int size) = 0; // size is in sample 
	virtual void getSoundData(char * sound_buffer, int size) = 0;
	virtual bool getParameters(int &rate, bool &stereo, bool &is_16bit) = 0;
	virtual int getTrackIdentifier() const = 0;
};

class SaudChannel : public _Channel {
private:
	int _track;				//!< The track identifier
	int _nbframes;			//!< number of frames of the track (unused)
	int _dataSize;			//!< the size of the sound buffer
	int _frequency;			//!< the frequency target of the track (always 22050)
	bool _inData;			//!< are we processing data ?
	bool _markReached;		//!< set to \c true when the SMRK tag is reached
	int _flags;				//!< current flags of the track (unused)
	int _volume;			//!< the current track volume
	int _balance;			//!< the current track balance
	int _index;				//!< the current PSAD index (for coherency checking)
	short _voltable[2][256];	//!< the precalculated volume table (stereo 16 bits)
	unsigned char * _tbuffer;	//!< data temporary buffer
	int _tbufferSize;			//!< temporary buffer size
	unsigned char * _sbuffer;	//!< sound buffer
	int _sbufferSize;			//!< sound buffer size

protected:
	void handleStrk(Chunk & c);
	void handleSmrk(Chunk & c);
	void handleShdr(Chunk & c);
	bool handleSubTags(int & offset);
	bool processBuffer();
	void recalcVolumeTable();

public:
	SaudChannel(int track, int freq);
	virtual ~SaudChannel();
	bool isTerminated() const;
	bool setParameters(int duration, int flags, int vol1, int vol2);
	bool checkParameters(int index, int duration, int flags, int vol1, int vol2);
	bool appendData(Chunk & b, int size);
	int availableSoundData() const;
	void getSoundData(short * sound_buffer, int size);
	void getSoundData(char * sound_buffer, int size) { error("16bit request for SAUD channel should never happen"); };
	bool getParameters(int &rate, bool &stereo, bool &is_16bit) { 
		rate = _frequency;
		stereo = true;
		is_16bit = true;
		return true;
	};
	virtual int getTrackIdentifier() const { return _track; };
};

/*! 	@brief class for a IACT sound ::channel (a The Dig track)

	This class implements a channel specifically for The Dig.

	\bug for unknown reason, some sound have a too long duration, or repeat themselves.
*/
class ImuseChannel : public _Channel {
private:
	int _track;				//!< the track number
	unsigned char * _tbuffer;	//!< data temporary buffer
	int _tbufferSize;			//!< temporary buffer size
	unsigned char * _sbuffer;	//!< sound buffer
	int _sbufferSize;			//!< sound buffer size
	int _srbufferSize;
	int _frequency;			//!< the target frequency of the ::mixer
	int _dataSize;			//!< remaining size of sound data in the iMUS buffer
	bool _inData;

	int _bitsize;			//!< the bitsize of the original data
	int _rate;				//!< the sampling rate of the original data
	int _channels;			//!< the number of channels of the original data

protected:
	int decode(int size, int &ret);
	void decode();
	bool processBuffer();
	bool handleMap(Chunk &);
	bool handleFormat(Chunk &);
	bool handleText(Chunk &);
	bool handleRegion(Chunk &);
	bool handleStop(Chunk &);
	bool handleSubTags(int & offset);

public:
	ImuseChannel(int track, int freq);
	virtual ~ImuseChannel();
	bool isTerminated() const;
	bool setParameters(int nbframes, int size, int unk1, int unk2);
	bool checkParameters(int index, int nbframes, int size, int unk1, int unk2);
	bool appendData(Chunk & b, int size);
	int availableSoundData() const;
	void getSoundData(short * sound_buffer, int size);
	void getSoundData(char * sound_buffer, int size);
	bool getParameters(int &rate, bool &stereo, bool &is_16bit) {
		rate = _frequency;
		stereo = (_channels == 2);
		is_16bit = (_bitsize > 8);
		return true;
	};
	virtual int getTrackIdentifier() const { return _track; };
};

#endif
