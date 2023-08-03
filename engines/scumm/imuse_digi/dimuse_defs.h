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

#define DIMUSE_BASE_SAMPLERATE 22050
#define DIMUSE_BASE_FEEDSIZE   512
#define DIMUSE_NUM_WAVE_BUFS   8
#define DIMUSE_SMUSH_SOUNDID   12345678
#define DIMUSE_BUN_CHUNK_SIZE  0x2000
#define DIMUSE_GROUP_SFX       1
#define DIMUSE_GROUP_SPEECH    2
#define DIMUSE_GROUP_MUSIC     3
#define DIMUSE_GROUP_MUSICEFF  4
#define DIMUSE_BUFFER_SPEECH   1
#define DIMUSE_BUFFER_MUSIC    2
#define DIMUSE_BUFFER_SFX      3

#define DIMUSE_TIMER_BASE_RATE_HZ       50
#define DIMUSE_TIMER_BASE_RATE_USEC     20000  // 1000000 / 50Hz
#define DIMUSE_TIMER_GAIN_RED_RATE_USEC 100000 // 1000000 / 10Hz
#define DIMUSE_TIMER_FADES_RATE_USEC    16667  // 1000000 / 60Hz

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

// Soundkludge command IDs
#define DIMUSE_C_KLUDGE_SET_STATE        0x1000
#define DIMUSE_C_KLUDGE_SET_SEQUENCE     0x1001
#define DIMUSE_C_KLUDGE_SET_CUE_POINT    0x1002
#define DIMUSE_C_KLUDGE_SET_ATTRIBUTE    0x1003
#define DIMUSE_C_KLUDGE_SET_SFX_VOLUME   0x2000
#define DIMUSE_C_KLUDGE_SET_VOICE_VOLUME 0x2001
#define DIMUSE_C_KLUDGE_SET_MUSIC_VOLUME 0x2002
#define DIMUSE_C_KLUDGE_STOP_ALL_SNDS    10
#define DIMUSE_C_KLUDGE_SET_PARAM        12
#define DIMUSE_C_KLUDGE_FADE_PARAM       14
#define DIMUSE_C_KLUDGE_START_STREAM     25
#define DIMUSE_C_KLUDGE_SWITCH_STREAM    26

// Script command IDs
#define DIMUSE_C_SCRIPT_INIT          0
#define DIMUSE_C_SCRIPT_TERMINATE     1
#define DIMUSE_C_SCRIPT_SAVE          2
#define DIMUSE_C_SCRIPT_RESTORE       3
#define DIMUSE_C_SCRIPT_REFRESH       4
#define DIMUSE_C_SCRIPT_SET_STATE     5
#define DIMUSE_C_SCRIPT_SET_SEQUENCE  6
#define DIMUSE_C_SCRIPT_CUE_POINT     7
#define DIMUSE_C_SCRIPT_SET_ATTRIBUTE 8

// Internal command IDs
#define DIMUSE_C_INIT             0
#define DIMUSE_C_PAUSE            3
#define DIMUSE_C_RESUME           4
#define DIMUSE_C_SET_GRP_VOL      7
#define DIMUSE_C_START_SND        8
#define DIMUSE_C_STOP_SND         9
#define DIMUSE_C_STOP_ALL_SNDS    10
#define DIMUSE_C_GET_NEXT_SND     11
#define DIMUSE_C_SET_PARAM        12
#define DIMUSE_C_GET_PARAM        13
#define DIMUSE_C_FADE_PARAM       14
#define DIMUSE_C_SET_HOOK         15
#define DIMUSE_C_GET_HOOK         16
#define DIMUSE_C_SET_TRIGGER      17
#define DIMUSE_C_CHECK_TRIGGER    18
#define DIMUSE_C_CLEAR_TRIGGER    19
#define DIMUSE_C_DEFER_CMD        20
#define DIMUSE_C_GET_MARKER_SYNCS 21
#define DIMUSE_C_START_STREAM     25
#define DIMUSE_C_SWITCH_STREAM    26
#define DIMUSE_C_PROCESS_STREAMS  27
#define DIMUSE_C_FEED_STREAM      29

// Trigger callback command ID
#define DIMUSE_C_SCRIPT_CALLBACK  0

// Block IDs for the Creative Voice File format
// used within Full Throttle and The Dig (demo)
#define VOC_DIGI_DATA_BLOCK  1
#define VOC_MARKER_BLOCK     4
#define VOC_LOOP_START_BLOCK 6
#define VOC_LOOP_END_BLOCK   7

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
	int index;
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
