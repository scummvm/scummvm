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

#ifndef GRIM_IMUSE_DEFS_H
#define GRIM_IMUSE_DEFS_H

namespace Common {
class SeekableReadStream;
}

namespace Grim {

#define IMUSE_MAX_GROUPS      16
#define IMUSE_MAX_FADES       16
#define IMUSE_MAX_TRIGGERS    8
#define IMUSE_MAX_DEFERS      8
#define IMUSE_MAX_TRACKS      9
#define IMUSE_MAX_MAP_SIZE    512
#define IMUSE_MAX_DISPATCHES  9
#define IMUSE_MAX_STREAMZONES 50
#define IMUSE_MAX_FADE_VOLUME 8323072
#define IMUSE_MAX_STREAMS     3
#define IMUSE_MAX_SOUND_LIST  10
#define IMUSE_MAX_OPEN_FILES  5
#define IMUSE_MAX_CACHED_SOUNDS 32
#define IMUSE_SOUNDS_MEMORY_LIMIT 300000
#define IMUSE_MAX_LIPSYNCS    4
#define IMUSE_LIPSYNC_MAX_SIZE 2000
#define IMUSE_PHONEME_NONE    95
#define IMUSE_HMI_HEAP_SIZE   150000
#define IMUSE_HMI_TAIL_SIZE   18000

#define IMUSE_LARGE_FADES    1
#define IMUSE_SMALL_FADES    2
#define IMUSE_LARGE_FADE_DIM 100000
#define IMUSE_SMALL_FADE_DIM 44100

#define IMUSE_BASE_SAMPLERATE 22050
#define IMUSE_BASE_FEEDSIZE   512
#define IMUSE_NUM_WAVE_BUFS   8
#define IMUSE_SMUSH_SOUNDID   12345678

enum {
	kTalkSoundID = 10000
};

#define IMUSE_BUN_CHUNK_SIZE  0x2000
#define IMUSE_GROUP_MASTER    0
#define IMUSE_GROUP_SFX       1
#define IMUSE_GROUP_SPEECH    2
#define IMUSE_GROUP_MUSIC     3
#define IMUSE_GROUP_MUSICEFF  4
#define IMUSE_BUFFER_SPEECH   1
#define IMUSE_BUFFER_MUSIC    2
#define IMUSE_BUFFER_SFX      3

#define IMUSE_TIMER_BASE_RATE_HZ       50
#define IMUSE_TIMER_BASE_RATE_USEC     20000  // 1000000 / 50Hz
#define IMUSE_TIMER_GAIN_RED_RATE_USEC 200000 // 1000000 / 5Hz
#define IMUSE_TIMER_FADES_RATE_USEC    20000  // 1000000 / 50Hz

// Parameters IDs
#define IMUSE_P_BOGUS_ID       0x0
#define IMUSE_P_SND_TRACK_NUM  0x100
#define IMUSE_P_TRIGS_SNDS     0x200
#define IMUSE_P_MARKER         0x300
#define IMUSE_P_GROUP          0x400
#define IMUSE_P_PRIORITY       0x500
#define IMUSE_P_VOLUME         0x600
#define IMUSE_P_PAN            0x700
#define IMUSE_P_DETUNE         0x800
#define IMUSE_P_TRANSPOSE      0x900
#define IMUSE_P_MAILBOX        0xA00
#define IMUSE_P_UNKNOWN        0xF00
#define IMUSE_P_SND_HAS_STREAM 0x1800
#define IMUSE_P_STREAM_BUFID   0x1900
#define IMUSE_P_SND_POS_IN_MS  0x1A00

// Soundkludge command IDs
#define IMUSE_C_KLUDGE_SET_STATE        0x1000
#define IMUSE_C_KLUDGE_SET_SEQUENCE     0x1001
#define IMUSE_C_KLUDGE_SET_CUE_POINT    0x1002
#define IMUSE_C_KLUDGE_SET_ATTRIBUTE    0x1003
#define IMUSE_C_KLUDGE_SET_SFX_VOLUME   0x2000
#define IMUSE_C_KLUDGE_SET_VOICE_VOLUME 0x2001
#define IMUSE_C_KLUDGE_SET_MUSIC_VOLUME 0x2002
#define IMUSE_C_KLUDGE_STOP_ALL_SNDS    10
#define IMUSE_C_KLUDGE_SET_PARAM        12
#define IMUSE_C_KLUDGE_FADE_PARAM       14
#define IMUSE_C_KLUDGE_START_STREAM     25
#define IMUSE_C_KLUDGE_SWITCH_STREAM    26

// Script command IDs
#define IMUSE_C_SCRIPT_INIT          0
#define IMUSE_C_SCRIPT_TERMINATE     1
#define IMUSE_C_SCRIPT_SAVE          2
#define IMUSE_C_SCRIPT_RESTORE       3
#define IMUSE_C_SCRIPT_REFRESH       4
#define IMUSE_C_SCRIPT_SET_STATE     5
#define IMUSE_C_SCRIPT_SET_SEQUENCE  6
#define IMUSE_C_SCRIPT_CUE_POINT     7
#define IMUSE_C_SCRIPT_SET_ATTRIBUTE 8

// Internal command IDs
#define IMUSE_C_INIT             0
#define IMUSE_C_TERMINATE        1
#define IMUSE_C_PAUSE            3
#define IMUSE_C_RESUME           4
#define IMUSE_C_SET_GRP_VOL      7
#define IMUSE_C_START_SND        8
#define IMUSE_C_STOP_SND         9
#define IMUSE_C_STOP_ALL_SNDS    10
#define IMUSE_C_GET_NEXT_SND     11
#define IMUSE_C_SET_PARAM        12
#define IMUSE_C_GET_PARAM        13
#define IMUSE_C_FADE_PARAM       14
#define IMUSE_C_SET_HOOK         15
#define IMUSE_C_GET_HOOK         16
#define IMUSE_C_SET_TRIGGER      17
#define IMUSE_C_CHECK_TRIGGER    18
#define IMUSE_C_CLEAR_TRIGGER    19
#define IMUSE_C_DEFER_CMD        20
#define IMUSE_C_START_STREAM     25
#define IMUSE_C_SWITCH_STREAM    26
#define IMUSE_C_PROCESS_STREAMS  27
#define IMUSE_C_FEED_STREAM      29

// Trigger callback command ID
#define IMUSE_C_SCRIPT_CALLBACK  0

class IMuse;

struct IMuseDispatch;
struct IMuseTrack;
struct IMuseStreamZone;

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
} IMuseTrigger;

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
} IMuseDefer;

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
} IMuseFade;

struct IMuseTrack {
	int index;
	IMuseTrack *prev;
	IMuseTrack *next;
	IMuseDispatch *dispatchPtr;
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
};

struct IMuseStreamZone {
	IMuseStreamZone *prev;
	IMuseStreamZone *next;
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
} IMuseStream;

typedef struct {
	uint8 *buffer;
	int32 bufSize;
	int32 loadSize;
	int32 criticalSize;
} IMuseSndBuffer;

struct IMuseSoundList {
	IMuseSoundList *prev;
	IMuseSoundList *next;
	int soundId;
	char filename[36];
	int refCount;
};

struct IMuseResource {
	Common::SeekableReadStream *resId;
	bool hasMcmp;
	int32 resSize;
	int32 fileCurOffset;
	int32 compBlocksNum;
	uint8 *mcmpBlockPtr;
	uint8 **compBlockPtr;
	int32 *decompBlockSize;
	int32 *compBlockOffset;
	int32 *compBlockSize;
	int32 *vimaOffsets;
	int32 curProcessedCompBlock;
	int32 curDecompBlockSize;
	uint8 *decompDataBuffer;
	char filename[80];
};

struct IMuseCachedSound {
	char soundFilename[32];
	int luaSoundId;
	uint32 tsOfAllocation;
	uint8 *soundUncompData;
	uint8 *soundAddrData;
	uint8 *soundMap;
	int uncompResourceSize;
	int flag;
	int priority;
};

struct IMuseOpenAudioFile {
	IMuseResource *resourcePtr;
	char soundName[32];
	int refCount;
};

struct IMuseDispatch {
	IMuseTrack *trackPtr;
	int wordSize;
	int sampleRate;
	int channelCount;
	int swapEndiannessFlag;
	int32 currentOffset;
	int32 audioRemaining;
	int32 map[IMUSE_MAX_MAP_SIZE];
	IMuseStream *streamPtr;
	int streamBufID;
	IMuseStreamZone *streamZoneList;
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
};

typedef struct {
	int bytesPerSample;
	int numChannels;
	uint8 *mixBuf;
	int mixBufSize;
	int sizeSampleKB;
} waveOutParamsStruct;

} // End of namespace Grim
#endif
