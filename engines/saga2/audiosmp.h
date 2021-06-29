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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_AUDIOSMP_H
#define SAGA2_AUDIOSMP_H

namespace Saga2 {

class decoderSet;

/* ===================================================================== *

   Sound Sample Attributes

     id - resource id, file offset, etc
     status - processing status

     channels - mono or stereo
     rate - sampling rate 11K/22K/44K
     granularity - 8 bit or 16 bit

     flags:
       looped - keep playing that song Sam

 * ===================================================================== */

#define ENDSAMP 0xFFFFFFFF

typedef uint32 soundSampleID;
typedef soundSegment *segmentArray;
typedef int8 Volume;
typedef Point32 sampleLocation;

enum soundSampleRate {
	soundRate11K = 11025,
	soundRate22K = 22050,
	soundRate44K = 44100
};


class soundSample { //: private DList
public:
	// sampleFlags
	enum soundSampleStatus {
		sampleNone,
		sampleMore,
		sampleStop,
		samplePart,
		sampleDone,
		sampleKill
	};

	enum soundSampleChannels {
		channelMono,
		channelStereo,
		channelLeftOnly,
		channelRightOnly
	};

	enum soundSampleGranularity {
		granularity8Bit,
		granularity16Bit
	};


	// sampleFlags
	enum soundSampleFlags {
		sampleLooped = 0x0001,
		sampleSigned = 0x0002,
		sampleRvrsed = 0x0004
	};
private:
	bool                    initialized;
	soundSampleChannels     defaultChannels;
	soundSampleRate         defaultSpeed;
	soundSampleGranularity  defaultDataSize;
protected:
//	sampleLocation           location;
	Volume                  volume;

public:
	soundSampleChannels     channels;
	soundSampleRate         speed;
	soundSampleGranularity  dataSize;
	uint32                  sampleFlags;
	uint32                  loopCount;
	soundSampleStatus       status;
	soundSegment            curSeg;
	soundSegment            headSeg;
	void                    *sourceBuffer;
	decoderSet              *decoder;
	PublicQueue<uint32>     segmentList;

	soundSample(soundSegment sa[]);  //, sampleLocation pos=Point32( 0, 0 ));
	soundSample(soundSegment seg);  //, sampleLocation pos=Point32( 0, 0 ));
	virtual ~soundSample();

	soundSample &operator=(const soundSample &src);
	soundSample(const soundSample &src);
	bool operator==(const soundSample &src2) const;

	virtual Volume getVolume(void);
	virtual void setVolume(Volume v);

	virtual void moveTo(Point32) {}

	int init(void) {
		defaultChannels = channelMono;
		defaultSpeed = soundRate22K;
		defaultDataSize = granularity16Bit;
		initialized = true;
		return 0;
	}

	void setDefaultProfile(soundSampleChannels  c, soundSampleRate r, soundSampleGranularity  g) {
		if (initialized != true) init();
		defaultChannels = c;
		defaultSpeed = r;
		defaultDataSize = g;
	}

	soundSampleChannels     getDefaultChannels(void) {
		return defaultChannels;
	}
	soundSampleRate         getDefaultSpeed(void)    {
		return defaultSpeed;
	}
	soundSampleGranularity  getDefaultDataSize(void) {
		return defaultDataSize;
	}

	uint32 format(void);
	uint32 flags(void);
};

/* ===================================================================== *
   A moving sample class (not currently enabled)
 * ===================================================================== */

typedef Volume(*audioAttenuationFunction)(sampleLocation loc, Volume maxVol);
#define ATTENUATOR( name ) Volume name( sampleLocation loc, Volume maxVol )
typedef Point32 sampleVelocity;
typedef Point32 sampleAcceleration;


class positionedSample : public soundSample {
	sampleLocation          Pos;
public:
	positionedSample(soundSegment sa[], sampleLocation pos = Point32(0, 0));
	positionedSample(soundSegment seg, sampleLocation pos = Point32(0, 0));
	virtual Volume getVolume(void);
	void moveTo(Point32 newLoc) {
		Pos = newLoc;
	}
	virtual void setVolume(Volume v);

};

class movingSample : public positionedSample {
private:
	sampleLocation          Pos;
	sampleVelocity          dPos;
	sampleAcceleration      ddPos;
	uint32                  t0;
	void updateLocation(void);
public:
	movingSample(soundSegment sa[], sampleLocation pos, sampleVelocity vel, sampleAcceleration acc);
	movingSample(soundSegment seg, sampleLocation pos, sampleVelocity vel, sampleAcceleration acc);
	~movingSample() {};

	Volume getVolume(void);
};

} // end of namespace Saga2

#endif
