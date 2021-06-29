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

#ifndef SAGA2_AUDIOSYS_H
#define SAGA2_AUDIOSYS_H

namespace Saga2 {

class Buffer;
class musicBuffer;
class soundQueue;
class decoderSet;

/*******************************************************************/
/* DRIVERS subdirectory                                            */

#define DRIVER_PATH "DRIVERS"
#define UNDRIVER_PATH ".."

/*******************************************************************/
/* typedefs                                                        */

typedef Buffer *pSBUFFER;
typedef positionedSample *pSAMPLE;

/*******************************************************************/
/* Debug handling                                                  */

#define STATUS_MESSAGES DEBUG
#define ALLOW_MOVING_SAMPLES 0
#define ZERO_VOLUME_DISABLES 1
#define ALLOW_AUDIO_RECYCLING 1


/*******************************************************************/
/* Volume controls                                                 */

#define Here Point32(0,0)

/*******************************************************************/
/* Volume controls                                                 */

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
	pSBUFFER                SampHand[maxBuffers];
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
	SoundQueue              queue;              // the queue
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
	void queueMusic(soundSegment s, decoderSet *, int16 loopFactor = 1, sampleLocation where = Here);
	void stopMusic(void);
	soundSegment currentMusic(void) {
		return playing;    // ID of music currently playing
	}
	bool goodMIDICard(void);

	// sound calls
	void queueSound(soundSegment s, decoderSet *, int16 loopFactor = 1, sampleLocation where = Here);
	//void queueSoundAt( soundSegment s, decoderSet *, sampleLocation where, int16 loopFactor=1);
	void queueSoundSample(positionedSample *ss, decoderSet *sDec, int16 loopFactor);

	// loop calls
	void queueLoop(soundSegment s, decoderSet *sDec, int16 loopFactor = 0, sampleLocation where = Here);
	//void queueLoopAt( soundSegment s, decoderSet *sDec, sampleLocation where , int16 loopFactor=0 );
	void queueLoopSample(positionedSample *ss, decoderSet *sDec, int16 loopFactor = 0);
	void stopLoop(void);
	void setLoopPosition(sampleLocation newLoc);
	soundSegment currentLoop(void) {
		return looping;    // ID of music currently playing
	}

	// voice calls
	void queueVoice(soundSegment s, decoderSet *, sampleLocation where = Here);
	void queueVoice(soundSegment s[], decoderSet *, sampleLocation where = Here);
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
		return queue.getSize();
	}

	// moving sample calls
	audioAttenuationFunction setAttenuator(audioAttenuationFunction newAF);


private:
	void load_drivers(const char *driver_path = DRIVER_PATH, const char *undriver_path = UNDRIVER_PATH);
	void load_dig_driver(void);
	void load_mid_driver(void);
	bool notEmpty(void);
	BufferRequest needBuffer(positionedSample *ss, BufferRequest needBufNo);
	void format(void);
	void openSample(decoderSet *decList, Buffer *);                // open/seek function
	void closeSample(decoderSet *decList, Buffer *);               // close/flush function
	void playSample(decoderSet *decList, BufferRequest, positionedSample *);                // read/load function
	sampleFlags playQueue(decoderSet *decList, BufferRequest, soundQueue *);               // read/load function
	void playMusic(decoderSet *decList, BufferRequest targBuffer, positionedSample *ss, int16 loopFactor);
	void makeWriteable(Buffer *sb);     // buffer release

	void setSoundMasterVolume(Volume val);
	void setMusicMasterVolume(Volume val);
	void setBufferVolume(BufferRequest, Volume val);
	void setMusicVolume(Volume val);
	void setSoundVolume(Volume val);
	void setVoiceVolume(Volume val);
	void setLoopsVolume(Volume val);
	inline bool checkMask(volumeTarget t, volumeTarget m, bool e, volumeTarget vtm) {
		if ((t & m)
#if ZERO_VOLUME_DISABLES
		        && (getVolume(m) > 0) && (getVolume(vtm) > 0)
#endif
		   )
			return e;
		return true;
	}


	void crossFade(musicBuffer *mbOut, musicBuffer *mbIn);
	void fadeIn(musicBuffer *mbIn);

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
