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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_AUDIO_H
#define SAGA2_AUDIO_H

#include "audio/mixer.h"
#include "saga2/queues.h"

namespace Saga2 {

/* ===================================================================== *
   the library(s) must be recompiled if you change these settings
 * ===================================================================== */

#define QUEUES_EXTERNAL_ALLOCATION 1

// TODO: FIXME. STUB
typedef int HDIGDRIVER;
typedef int HTIMER;
typedef int HMDIDRIVER;

inline void audioFatal(char *msg) {
	error("Sound error %s", msg);
}

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

typedef Volume(*audioAttenuationFunction)(sampleLocation loc, Volume maxVol);
#define ATTENUATOR( name ) Volume name( sampleLocation loc, Volume maxVol )

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


/*******************************************************************/
/* DRIVERS subdirectory                                            */

#define DRIVER_PATH "DRIVERS"
#define UNDRIVER_PATH ".."

/*******************************************************************/
/* typedefs                                                        */

typedef positionedSample *pSAMPLE;

/*******************************************************************/
/* Volume controls                                                 */

#define Here Point32(0,0)

typedef int8 Volume;

enum volumeTarget {
	volSound      = 1L << 0, // sound volume
	volVoice      = 1L << 1, // voice volume
	volSandV,               // sound & voice
	volLoops      = 1L << 2, // looped sounds
	volSandL,               // sound and music
	volVandL,               // voice and music
	volSVandL,              // voice and music
	volMusic      = 1L << 3, // music
	volSandM,               // sound and music
	volVandM,               // voice and music
	volSVandM,              // sound voice and music
	volLandM,               // loops and music
	volSLandM,              // sound loops and music
	volVLandM,              // voice loops and music
	volAll,                 // all four
	volSoundMaster = 1L << 4, // master sound volume level
	volMusicMaster = 1L << 5  // master music volume level
};

enum volumeMode {
	volumeSetTo = 0L,   // absolute mode
	volumeUpDown       // relative mode
};

/*******************************************************************/
/* Audio Fade                                                      */

struct audioFade {
	int16       fadeOutTime,
	            fadeInTime,            // Actually it starts fading in immediately
	            overlapTime;
};


/*******************************************************************/
/* Audio Interface Settings                                        */

struct audioInterfaceSettings {
	int16   soundBuffers;              // Buffers aside from Music, Voice & loop buffers
	uint32  voiceBufferSize;           // Size of voice buffer
	uint32  musicBufferSize;           // Size of music buffers
	uint32  soundBufferSize;           // Size of sound effects buffers
	uint32  loopBufferSize;            // Size of looped effect buffer

	audioInterfaceSettings(int16 sb, uint32 vbs, uint32 mbs, uint32 sbs, uint32 lbs) {
		soundBuffers = sb;
		voiceBufferSize = vbs;
		musicBufferSize = mbs;
		soundBufferSize = sbs;
		loopBufferSize = lbs;
	}

private:
	audioInterfaceSettings();
};


/*******************************************************************/
/*                                                                 */
/* Audio Interface Class                                           */
/*                                                                 */
/*******************************************************************/
struct SoundInstance {
	soundSegment seg;
	bool loop;
	sampleLocation loc;
};

class audioInterface {
public:
	enum BufferRequest {
		requestRecycled = -2L,
		requestFailed   = -1L,
		requestVoice    = 0L,
		requestMusic0   = 1L,
		requestMusic1   = 2L,
		requestLoop     = 3L,
		requestSound0   = 4L,
		requestSound1   = 5L,
		requestSound2   = 6L,
		maxBuffers      = 7L,
		requestMusic    = 8L,
		requestSound    = 9L
	};

private:
	enum sampleFlags {
		sampleClear   = 0L,
		sampleMoving  = 1L << 0,
		sampleQueued  = 1L << 1
	};

	enum sampleStopLevel {
		sStopCleanup  = 0L,
		sStopSegment  = 1L,
		sStopSample   = 2L,
		sStopQueue    = 3L
	};

	enum queueFlags {
		qrQueueEmpty  = 0L,
		qrBufNotAlloc = 1L,
		qrSegNotOpen  = 2L,
		qrBufNotReady = 3L,
		qrWaiting     = 4L,
		qrPlayDone    = 5L,
		qrFinishing   = 6L,
		qrFinished    = 7L,
		qrCleanup     = 8L
	};

	int16                   instance;
	int16                   initialized;

	HTIMER                  gameTimer;

	queueFlags              queueRes;

	BufferRequest           numBuffers;

	int16                   flags[maxBuffers];
	int16                   state[maxBuffers];
	pSAMPLE                 samples[maxBuffers];
	soundSegment            lastID[maxBuffers];           // ID of music currently playing

	BufferRequest           voiceBuffer;          // buffer to feed voice into
	BufferRequest           activeLoop;       // currently playing loop
	BufferRequest           activeMusic;

	BufferRequest           nextMBuf;
	soundSegment            playing;           // ID of music currently playing
	soundSegment            looping;           // ID of music currently playing


	audioFade               fader;


	Volume                  SoundVolume,       // Sound Master Volume
	                        MusicVolume,       // Music Master Volume
	                        mVolume,           // music volume
	                        sVolume,           // sound volume
	                        vVolume,           // voice volume
	                        lVolume;           // loops volume

	int16                   sEnabled,          // sound enabled
	                        vEnabled,          // voice enabled
	                        mEnabled,          // music enabled
	                        lEnabled;          // loops enabled
	bool                    suspended;

	int32                   mQuality;          // MIDI driver quality
	int32                   suspendCalls;

public:
	Audio::SoundHandle _speechSoundHandle;
	Audio::SoundHandle _sfxSoundHandle;
	Audio::SoundHandle _bgmSoundHandle;
	Common::Queue<SoundInstance> _speechQueue;
	Common::Queue<SoundInstance> _sfxQueue;
	Common::Queue<SoundInstance> _bgmQueue;
	HDIGDRIVER              dig;               // AIL sample driver
	HMDIDRIVER              mid;               // AIL MIDI driver
	audioAttenuationFunction attenuator;


private:
	char                    status[256];       // audio status messages
	int16                   verbosity;

public:
	// ctor, dtor, initialization
	audioInterface(const char *driver_path = DRIVER_PATH, const char *undriver_path = UNDRIVER_PATH);
	~audioInterface();

	// init, cleanup
	void initAudioInterface(audioInterfaceSettings &ais);
	void cleanupAudioInterface(void);

	// timer calls
	void adjustGameSpeed(int32 multiplyBy, int32 thenDivideBy);
	void suspendGameClock(void);
	void resumeGameClock(void);

	// event loop calls
	bool playFlag(void);
	void playMe(void);

	// music calls
	void playMusic(soundSegment s, int16 loopFactor = 1, sampleLocation where = Here);
	void stopMusic(void);
	soundSegment currentMusic(void) {
		return playing;    // ID of music currently playing
	}
	bool goodMIDICard(void);

	// sound calls
	void queueSound(soundSegment s, int16 loopFactor = 1, sampleLocation where = Here);
	void queueSoundSample(positionedSample *ss, int16 loopFactor);

	// loop calls
	void queueLoop(soundSegment s, int16 loopFactor = 0, sampleLocation where = Here);
	void queueLoopSample(positionedSample *ss, int16 loopFactor = 0);
	void stopLoop(void);
	void setLoopPosition(sampleLocation newLoc);
	soundSegment currentLoop(void) {
		return looping;    // ID of music currently playing
	}

	// voice calls
	void queueVoice(soundSegment s, sampleLocation where = Here);
	void queueVoice(soundSegment s[], sampleLocation where = Here);
	void stopVoice(void);
	void endVoice(sampleStopLevel ssl = sStopCleanup);
	void resetState(BufferRequest br);
	bool talking(void);
	bool saying(soundSegment s);

	// volume and enabled calls
	bool active(void);
	bool activeDIG(void) {
		return true;
	}
	bool enabled(volumeTarget i);
	void enable(volumeTarget i, bool onOff);
	void disable(volumeTarget i) {
		enable(i, false);
	}
	void setVolume(volumeTarget targ, volumeMode op, Volume val);
	Volume getVolume(volumeTarget src);
	void setMusicFadeStyle(int16 tOut, int16 tIn, int16 tOver);
	void suspend(void);
	void resume(void);

	//debugging calls
	char *statusMessage(void);
	void shutoffAudio(void);
	void setVerbosity(int16 n) {
		verbosity = n;
	}
	int16 getQueueSize(void) {
		return _speechQueue.size() + _sfxQueue.size() + _bgmQueue.size();
	}

	// moving sample calls
	audioAttenuationFunction setAttenuator(audioAttenuationFunction newAF);


private:
	void playMusic(BufferRequest targBuffer, positionedSample *ss, int16 loopFactor);

	void setSoundMasterVolume(Volume val);
	void setMusicMasterVolume(Volume val);
	void setBufferVolume(BufferRequest, Volume val);
	void setMusicVolume(Volume val);
	void setSoundVolume(Volume val);
	void setVoiceVolume(Volume val);
	void setLoopsVolume(Volume val);
	inline bool checkMask(volumeTarget t, volumeTarget m, bool e, volumeTarget vtm) {
		if ((t & m) && (getVolume(m) > 0) && (getVolume(vtm) > 0))
			return e;
		return true;
	}

	void audioFatalError(char *s);
	void audioError(char *s);
	void audioEPrintf(char *s, ...);
	void audioErrorID(int);
	void audioStatus(char *s);
	void audioStatCat(char *s);

	void setSample(BufferRequest sampNo, positionedSample *ss);

#ifdef __WATCOMC__
#pragma off ( unreferenced ) ;
#endif
	static ATTENUATOR(defaultAttenuator) {
		return maxVol;
	}
#ifdef __WATCOMC__
#pragma on ( unreferenced ) ;
#endif
};

void disableAudio(void);

} // end of namespace Saga2

#endif
