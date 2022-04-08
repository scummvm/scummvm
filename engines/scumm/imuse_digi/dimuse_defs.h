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

#if !defined(SCUMM_IMUSE_DIGI_DEFS_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_DEFS_H

namespace Scumm {

#define DIMUSE_MAX_GROUPS      16
#define DIMUSE_MAX_FADES       16
#define DIMUSE_MAX_TRIGGERS    8
#define DIMUSE_MAX_DEFERS      8
#define DIMUSE_MAX_TRACKS      8
#define DIMUSE_MAX_MAP_SIZE    2048
#define DIMUSE_MAX_DISPATCHES  8
#define DIMUSE_MAX_STREAMZONES 50
#define DIMUSE_MAX_FADE_VOLUME 8323072
#define DIMUSE_MAX_STREAMS     3

#define DIMUSE_LARGE_FADES    1
#define DIMUSE_SMALL_FADES    4
#define DIMUSE_LARGE_FADE_DIM 350000
#define DIMUSE_SMALL_FADE_DIM 44100

#define DIMUSE_SAMPLERATE     22050
#define DIMUSE_FEEDSIZE       512
#define DIMUSE_NUM_WAVE_BUFS  8
#define DIMUSE_SMUSH_SOUNDID  12345678
#define DIMUSE_BUN_CHUNK_SIZE 0x2000
#define DIMUSE_GROUP_SFX      1
#define DIMUSE_GROUP_SPEECH   2
#define DIMUSE_GROUP_MUSIC    3
#define DIMUSE_GROUP_MUSICEFF 4
#define DIMUSE_BUFFER_SPEECH  1
#define DIMUSE_BUFFER_MUSIC   2
#define DIMUSE_BUFFER_SMUSH   3

// Parameters IDs
#define DIMUSE_P_BOGUS_ID       0x0
#define DIMUSE_P_SND_TRACK_NUM  0x100
#define DIMUSE_P_TRIGS_SNDS     0x200
#define DIMUSE_P_MARKER         0x300
#define DIMUSE_P_GROUP          0x400
#define DIMUSE_P_PRIORITY       0x500
#define DIMUSE_P_VOLUME         0x600
#define DIMUSE_P_PAN            0x700
#define DIMUSE_P_DETUNE         0x800
#define DIMUSE_P_TRANSPOSE      0x900
#define DIMUSE_P_MAILBOX        0xA00
#define DIMUSE_P_UNKNOWN        0xF00
#define DIMUSE_P_SND_HAS_STREAM 0x1800
#define DIMUSE_P_STREAM_BUFID   0x1900
#define DIMUSE_P_SND_POS_IN_MS  0x1A00

struct IMuseDigiDispatch;
struct IMuseDigiTrack;
struct IMuseDigiStreamZone;

typedef struct {
	int sound;
	char text[256];
	int opcode;
	int a;
	int b;
	int c;
	int d;
	int e;
	int f;
	int g;
	int h;
	int i;
	int j;
	int clearLater;
} IMuseDigiTrigger;

typedef struct {
	int counter;
	int opcode;
	int a;
	int b;
	int c;
	int d;
	int e;
	int f;
	int g;
	int h;
	int i;
	int j;
} IMuseDigiDefer;

typedef struct {
	int status;
	int sound;
	int param;
	int currentVal;
	int counter;
	int length;
	int slope;
	int slopeMod;
	int modOvfloCounter;
	int nudge;
} IMuseDigiFade;

struct IMuseDigiTrack {
	IMuseDigiTrack *prev;
	IMuseDigiTrack *next;
	IMuseDigiDispatch *dispatchPtr;
	int soundId;
	int marker;
	int group;
	int priority;
	int vol;
	int effVol;
	int pan;
	int detune;
	int transpose;
	int pitchShift;
	int mailbox;
	int jumpHook;
	int32 syncSize_0;
	byte *syncPtr_0;
	int32 syncSize_1;
	byte *syncPtr_1;
	int32 syncSize_2;
	byte *syncPtr_2;
	int32 syncSize_3;
	byte *syncPtr_3;
};

struct IMuseDigiStreamZone {
	IMuseDigiStreamZone *prev;
	IMuseDigiStreamZone *next;
	int useFlag;
	int32 offset;
	int32 size;
	int fadeFlag;
};

typedef struct {
	int soundId;
	int32 curOffset;
	int32 endOffset;
	int bufId;
	uint8 *buf;
	int32 bufFreeSize;
	int32 loadSize;
	int32 criticalSize;
	int32 maxRead;
	int32 loadIndex;
	int32 readIndex;
	int paused;
	int vocLoopFlag;
	int32 vocLoopTriggerOffset;
} IMuseDigiStream;

typedef struct {
	uint8 *buffer;
	int32 bufSize;
	int32 loadSize;
	int32 criticalSize;
} IMuseDigiSndBuffer;

struct IMuseDigiDispatch {
	IMuseDigiTrack *trackPtr;
	int wordSize;
	int sampleRate;
	int channelCount;
	int32 currentOffset;
	int32 audioRemaining;
	int32 map[DIMUSE_MAX_MAP_SIZE];
	IMuseDigiStream *streamPtr;
	int streamBufID;
	IMuseDigiStreamZone *streamZoneList;
	int streamErrFlag;
	uint8 *fadeBuf;
	int32 fadeOffset;
	int32 fadeRemaining;
	int fadeWordSize;
	int fadeSampleRate;
	int fadeChannelCount;
	int fadeSyncFlag;
	int32 fadeSyncDelta;
	int fadeVol;
	int fadeSlope;
	int32 vocLoopStartingPoint;
};

typedef struct {
	int bytesPerSample;
	int numChannels;
	uint8 *mixBuf;
	int mixBufSize;
	int sizeSampleKB;
} waveOutParamsStruct;

} // End of namespace Scumm
#endif
