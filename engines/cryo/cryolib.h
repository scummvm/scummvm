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

#ifndef CRYO_CRYOLIB_H
#define CRYO_CRYOLIB_H

#include "audio/mixer.h"
#include "common/system.h"

#include "cryo/platdefs.h"

namespace Cryo {

#define SW16(n) ( (((n) & 0xFF) << 8) | (((n) >> 8) & 0xFF) )
#define SW32(n) ( (((n) & 0xFF) << 24) | (((n) >> 24) & 0xFF) | (((n) & 0xFF00) << 8) | (((n) >> 8) & 0xFF00))
#if 0
//big-endian host
#define LE16(n) SW16(n)
#define LE32(n) SW32(n)
#define BE16(n) (n)
#define BE32(n) (n)
#else
//little-endian host
#define LE16(n) (n)
#define LE32(n) (n)
#define BE16(n) SW16(n)
#define BE32(n) SW32(n)
#endif
#define PLE16(p) ( (((byte*)(p))[1] << 8) | ((byte*)(p))[0] )
#define PLE32(p) ( (((byte*)(p))[3] << 24) | (((byte*)(p))[2] << 16) | (((byte*)(p))[1] << 8) | ((byte*)(p))[0] )

enum {
	fsFromStart = 1
};

struct BlitView{
	int     _srcLeft;
	int     _srcTop;
	int     _dstLeft;
	int     _dstTop;
	int     _width;
	int     _height;
};

struct View {
	byte    *_bufferPtr;
	int      _width;
	int      _height;
	int16    _pitch;
	bool     _doubled;
	bool     _allocated;
	BlitView _normal;
	BlitView _zoom;
};

struct color3_t {
	int16   r, g, b;
};
typedef struct color3_t color3_t;

struct color_t {
	int16   a, r, g, b;
};
typedef struct color_t color_t;

#pragma pack(push, 1)
struct HNMHeader {
	int     _signature;
	char    _unusedFlag1;
	char    _unusedFlag2;
	char    _unusedReserved;
	char    _unusedBpp;
	uint16  _width;
	uint16  _height;
	int     _unusedFileSize;
	int     _numbFrame;
	int     _unusedTableOffset;
	int16   _unusedSpeed;
	int16   _unusedMaxBuffer;
	int     _bufferSize;
	int16   _unusedUnknown;
	char    _unusedReserved2[14];
	char    _unusedCopyright[16];
};
typedef struct HNMHeader HNMHeader;
#pragma pack(pop)

struct hnm_t {
	int  _frameNum;
	int  ff_4;
	file_t  *_file;
	HNMHeader     _header;
	byte   *tmpBuffer[2];
	byte   *finalBuffer;
	byte   *_newFrameBuffer;
	byte   *_oldFrameBuffer;
	byte   *_readBuffer;
	byte   *_dataPtr;
	color_t  _palette[256];

	bool  _canLoop;
	int16 ff_896;
	int16 _chunkId;
	int   _totalRead;
};
typedef struct hnm_t hnm_t;

struct sound_t {
	char *sndHandle;
	int16 _headerLen;
	long  _headerOffset;
	int16   ff_A;

	char    *_buffer;
	int     ff_16;
	int16   ff_1A;
	float   _rate;
	int16   _sampleSize;
	int     _length;
	int16   _mode;
	volatile int16  _locked;
	long    _loopStart;
	int16   _loopTimes;
	bool   _reversed;
	int16   ff_32;
	int16   _volume;
};
typedef struct sound_t sound_t;

#define kCryoMaxClSounds 64

struct soundgroup_t {
	sound_t *_sound[kCryoMaxClSounds];
	int16   _numSounds;
	int16   _soundIndex;
	int16   _playIndex;
	int16   ff_106;
};
typedef struct soundgroup_t soundgroup_t;

#define kCryoMaxChSounds 10

struct soundchannel_t {
	Audio::SoundHandle ch;
	int     xx;

	int16   _volumeLeft;
	int16   _volumeRight;
	int16   _numSounds;

	sound_t *_sounds[kCryoMaxChSounds];

	int16   ff_536;
};
typedef struct soundchannel_t soundchannel_t;

extern volatile long TimerTicks;
extern View ScreenView;


soundgroup_t *CLSoundGroup_New(int16 numSounds, int16 arg4, int16 sampleSize, float rate, int16 mode);
void CLSoundGroup_Free(soundgroup_t *sg);
void CLSoundGroup_Reverse16All(soundgroup_t *sg);
void *CLSoundGroup_GetNextBuffer(soundgroup_t *sg);
int16 CLSoundGroup_AssignDatas(soundgroup_t *sg, void *buffer, int length, int16 isSigned);
int16 CLSoundGroup_SetDatas(soundgroup_t *sg, void *data, int length, int16 isSigned);
void CLSoundGroup_PlayNextSample(soundgroup_t *sg, soundchannel_t *ch);

sound_t *CLSoundRaw_New(int16 arg1, float rate, int16 sampleSize, int16 mode);
void CLSoundRaw_Free(sound_t *sound);
void CLSoundRaw_AssignBuffer(sound_t *sound, void *buffer, int bufferOffs, int length);

void SysBeep(int x);
long TickCount();
void FlushEvents(int16 arg1, int16 arg2);

void CLBlitter_CopyViewRect(View *view1, View *view2, Common::Rect *rect1, Common::Rect *rect2);
void CLBlitter_Send2ScreenNextCopy(color_t *palette, uint16 first, uint16 count);
void CLBlitter_OneBlackFlash();
void CLBlitter_CopyView2ViewSimpleSize(byte *src, int16 srcw, int16 srcp, int16 srch,
                                       byte *dst, int16 dstw, int16 dstp, int16 dsth);
void CLBlitter_CopyView2ScreenCUSTOM(View *view);
void CLBlitter_CopyView2Screen(View *view);
void CLBlitter_UpdateScreen();
void CLBlitter_FillView(View *view, unsigned int fill);
void CLBlitter_FillScreenView(unsigned int fill);

void CLPalette_Init();
void CLPalette_SetLastPalette(color_t *palette, int16 first, int16 count);
void CLPalette_GetLastPalette(color_t *palette);
void CLPalette_SetRGBColor(color_t *palette, uint16 index, color3_t *rgb);
void CLPalette_Macintize(int16 macintize);
void CLPalette_SetInterval(uint16 first, uint16 last);
void CLPalette_DeactivateInterval();
void CLPalette_Send2Screen(struct color_t *palette, uint16 first, uint16 count);
void CLPalette_BeBlack();
void CLPalette_BeSystem();

void CLFile_SetFilter(int a3, int a4, int a5, int a6, int a7);
void CLFile_SetFinderInfos(void *fs, int a4, int a5);
void CLFile_GetFullPath(void *a3, char *a4);
void CLFile_MakeStruct(int a3, int a4, const char *name, filespec_t *fs);
void CLFile_Create(filespec_t *fs);
void CLFile_Open(filespec_t *fs, int16 mode, file_t &handle);
void CLFile_Close(file_t &handle);
void CLFile_SetPosition(file_t &handle, int16 mode, long pos);
void CLFile_Read(file_t &handle, void *buffer, long *size);
void CLFile_Write(file_t &handle, void *buffer, long *size);

void CLSound_PrepareSample(sound_t *sound, int16 mode);
void CLSound_SetWantsDesigned(int16 designed);
void CLSound_SetLength(sound_t *sound, int length);

soundchannel_t *CLSoundChannel_New(int arg1);
void CLSoundChannel_Free(soundchannel_t *ch);
void CLSoundChannel_Stop(soundchannel_t *ch);
void CLSoundChannel_Play(soundchannel_t *ch, sound_t *sound);
int16 CLSoundChannel_GetVolume(soundchannel_t *ch);
void CLSoundChannel_SetVolume(soundchannel_t *ch, int16 volume);
void CLSoundChannel_SetVolumeRight(soundchannel_t *ch, int16 volume);
void CLSoundChannel_SetVolumeLeft(soundchannel_t *ch, int16 volume);

int16 CLKeyboard_HasCmdDown();
void CLKeyboard_Read();
byte CLKeyboard_GetLastASCII();
int16 CLKeyboard_IsScanCodeDown(int16 scancode);

void CLMouse_Hide();
void CLMouse_Show();
void CLMouse_GetPosition(int16 *x, int16 *y);
void CLMouse_SetPosition(int16 x, int16 y);
uint16 CLMouse_IsDown();

void CLView_SetSrcZoomValues(View *view, int x, int y);
void CLView_SetDisplayZoomValues(View *view, int w, int h);
void CLView_Free(View *view);
void CLView_InitDatas(View *view, int w, int h, void *buffer);
View *CLView_New(int w, int h);
void CLView_CenterIn(View *parent, View *child);

void CLScreenView_Init();
void CLScreenView_CenterIn(View *view);

void CRYOLib_InstallExitPatch();
void CRYOLib_RemoveExitPatch();
void CRYOLib_Init();
void CRYOLib_Done();
void CRYOLib_MinimalInit();
void CRYOLib_ManagersInit();
void CRYOLib_ManagersDone();
void CRYOLib_SetDebugMode(int16 enable);
void CRYOLib_InstallEmergencyExit(void(*proc)());
void CRYOLib_SetupEnvironment();
void CRYOLib_RestoreEnvironment();
void CRYOLib_TestConfig();

int16 CLComputer_Has68030();
int16 CLComputer_Has68040();
void CLDesktop_TestOpenFileAtStartup();


void CLHNM_DecompLempelZiv(byte *buffer, byte *output);
void CLHNM_DecompUBA(byte *output, byte *curr_buffer, byte *prev_buffer,
                     byte *input, int width, char flags);
void CLHNM_Init();
void CLHNM_Done();
void CLHNM_SetupTimer(float rate);
void CLHNM_WaitLoop(hnm_t *hnm);
void CLHNM_SetupSound(int16 numSounds, int16 arg4, int16 sampleSize, float rate, int16 mode);
void CLHNM_SetupSoundADPCM(int16 numSounds, int16 arg4, int16 sampleSize, float rate, int16 mode);
void CLHNM_CloseSound();
void CLHNM_SetForceZero2Black(bool forceblack);
hnm_t *CLHNM_New(int preload_size);
void CLHNM_Dispose(hnm_t *hnm);
void CLHNM_SetFile(hnm_t *hnm, file_t *file);
void CLHNM_SetFinalBuffer(hnm_t *hnm, byte *buffer);
void CLHNM_AllocMemory(hnm_t *hnm);
void CLHNM_DeallocMemory(hnm_t *hnm);
void CLHNM_Read(hnm_t *hnm, int size);
void CLHNM_CanLoop(hnm_t *hnm, bool canLoop);
void CLHNM_SelectBuffers(hnm_t *hnm);
void CLHNM_ChangePalette(hnm_t *hnm);
void CLHNM_Desentrelace(hnm_t *hnm);
void CLHNM_FlushPreloadBuffer(hnm_t *hnm);
soundchannel_t *CLHNM_GetSoundChannel();
void CLHNM_TryRead(hnm_t *hnm, int size);
void CLHNM_ResetInternalTimer();
void CLHNM_Reset(hnm_t *hnm);
int16 CLHNM_LoadFrame(hnm_t *hnm);
void CLHNM_WantsSound(int16 sound);
void CLHNM_LoadDecompTable(int16 *buffer);
void CLHNM_DecompADPCM(byte *buffer, int16 *output, int size);
void CLHNM_SoundInADPCM(int16 is_adpcm);
void CLHNM_SoundMono(int16 is_mono);
bool CLHNM_NextElement(hnm_t *hnm);
void CLHNM_ReadHeader(hnm_t *hnm);
int16 CLHNM_GetVersion(hnm_t *hnm);
int CLHNM_GetFrameNum(hnm_t *hnm);
void CLHNM_Prepare2Read(hnm_t *hnm, int mode);
void CLHNM_SetPosIntoFile(hnm_t *hnm, long pos);
void CLHNM_Desentrelace320(byte *frame_buffer, byte *final_buffer, uint16 height);


} // End of namespace Cryo

#endif
