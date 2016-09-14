#ifndef CRYO_CRYOLIB_H
#define CRYO_CRYOLIB_H

#include "audio/mixer.h"

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
#define PLE16(p) ( (((unsigned char*)(p))[1] << 8) | ((unsigned char*)(p))[0] )
#define PLE32(p) ( (((unsigned char*)(p))[3] << 24) | (((unsigned char*)(p))[2] << 16) | (((unsigned char*)(p))[1] << 8) | ((unsigned char*)(p))[0] )

typedef void *SndChannel;
typedef char *Handle;
enum {
	fsFromStart = 1
};

extern short __debug2;

extern short __debug, __libError, __osError;

#define CLBeginCheck    { short __oldDebug = __debug; __debug = -1;
#define CLEndCheck      __debug = __oldDebug; }
#define CLNoError       __libError = 0;

#define CLCheckError() if(__debug && __libError){   \
		char buffer_[260];           \
		sprintf(buffer_, "File %s at line %d, with __libError = %d and __osError = %d", __FILE__, __LINE__, __libError, __osError);  \
		DebugStr(c2pstr(buffer_));   \
	};

#define CLAssert(x) if(!(x)) {  \
		char buffer_[260];           \
		sprintf(buffer_, "File %s at line %d, with __libError = %d and __osError = %d", __FILE__, __LINE__, __libError, __osError);  \
		DebugStr(c2pstr(buffer_));   \
	};

struct rect_t {
	int sy, sx, ey, ex;
};
typedef struct rect_t rect_t;

struct view_t {
	unsigned char   *p_buffer;
	int     width;
	int     height;
	short   pitch;
	short   doubled;
	short   allocated;
	struct {
		int     src_left;
		int     src_top;
		int     dst_left;
		int     dst_top;
		int     width;
		int     height;
	} norm, zoom;
};
typedef struct view_t view_t;

struct color3_t {
	short   r, g, b;
};
typedef struct color3_t color3_t;

struct color_t {
	short   a, r, g, b;
};
typedef struct color_t color_t;

struct palette_t {
	color_t colors[256];
};
typedef struct palette_t palette_t;

#pragma pack(push, 1)
struct hnmheader_t {
	int             id;
	char            flag1;
	char            flag2;
	char            reseverd;
	char            bpp;
	unsigned short  width;
	unsigned short  height;
	int             filesize;
	int             nframe;
	int             table_offset;
	short           speed;
	short           maxbuffer;
	int             buffersize;
	short           ff_20;
	char            reserved2[14];
	char            copyright[16];
};
typedef struct hnmheader_t hnmheader_t;
#pragma pack(pop)

struct hnm_t {
	int             frame;
	int             ff_4;
	file_t         *file;
	hnmheader_t     header;
	unsigned char   *work_buffer[2];
	unsigned char   *final_buffer;
	unsigned char   *new_frame_buffer;
	unsigned char   *old_frame_buffer;
	unsigned char   *read_buffer;
	unsigned char   *data_ptr;
	color_t         palette[256];

	short           can_loop;

	short           ff_896;
	short           chunk_id;
	int             total_read;
};
typedef struct hnm_t hnm_t;

//struct filespec_t {
//char  puff;
//};

struct sound_t {
	Handle  sndHandle;
	short   headerLen;
	long    headerOffset;
	short   ff_A;

	char    *buffer;
	int     ff_16;
	short   ff_1A;
	float   rate;
	short   sampleSize;
	int     length;
	short   mode;
	volatile short  locked;
	long    loopStart;
	short   loopTimes;
	short   reversed;
	short   ff_32;
	short   volume;
};
typedef struct sound_t sound_t;

#define CL_MAX_SOUNDS 64

struct soundgroup_t {
	sound_t *sound[CL_MAX_SOUNDS];
	short   numSounds;
	short   soundIndex;
	short   playIndex;
	short   ff_106;
};
typedef struct soundgroup_t soundgroup_t;

#define CL_MAX_CH_SOUNDS 10

struct soundchannel_t {
	Audio::SoundHandle ch;
	int     xx;

	short   volumeLeft;
	short   volumeRight;
	short   numSounds;

	sound_t *sounds[CL_MAX_CH_SOUNDS];

	short   ff_536;
};
typedef struct soundchannel_t soundchannel_t;

extern volatile long TimerTicks;
extern view_t ScreenView;


soundgroup_t *CLSoundGroup_New(short numSounds, short arg4, short sampleSize, float rate, short mode);
void CLSoundGroup_Free(soundgroup_t *sg);
void CLSoundGroup_Reverse16All(soundgroup_t *sg);
void *CLSoundGroup_GetNextBuffer(soundgroup_t *sg);
short CLSoundGroup_AssignDatas(soundgroup_t *sg, void *buffer, int length, short isSigned);
short CLSoundGroup_SetDatas(soundgroup_t *sg, void *data, int length, short isSigned);
void CLSoundGroup_PlayNextSample(soundgroup_t *sg, soundchannel_t *ch);

sound_t *CLSoundRaw_New(short arg1, float rate, short sampleSize, short mode);
void CLSoundRaw_Free(sound_t *sound);
void CLSoundRaw_AssignBuffer(sound_t *sound, void *buffer, int bufferOffs, int length);

char *c2pstr(char *s);
void DebugStr(char *s);
void *CLMemory_Alloc(int size);
void CLMemory_Free(void *ptr);
short MemError();
void SysBeep(int x);
long TickCount();
void FlushEvents(short arg1, short arg2);

void CLBlitter_CopyViewRect(view_t *view1, view_t *view2, rect_t *rect1, rect_t *rect2);
void CLBlitter_Send2ScreenNextCopy(color_t *palette, unsigned short first, unsigned short count);
void CLBlitter_OneBlackFlash();
void CLBlitter_CopyView2ViewSimpleSize(unsigned char *src, short srcw, short srcp, short srch,
                                       unsigned char *dst, short dstw, short dstp, short dsth);
void CLBlitter_CopyView2ScreenCUSTOM(view_t *view);
void CLBlitter_CopyView2Screen(view_t *view);
void CLBlitter_UpdateScreen();
void CLBlitter_FillView(view_t *view, unsigned int fill);
void CLBlitter_FillScreenView(unsigned int fill);

void CLPalette_Init();
void CLPalette_SetLastPalette(color_t *palette, short first, short count);
void CLPalette_GetLastPalette(color_t *palette);
void CLPalette_SetRGBColor(color_t *palette, unsigned short index, color3_t *rgb);
void CLPalette_Macintize(short macintize);
void CLPalette_SetInterval(unsigned short first, unsigned short last);
void CLPalette_DeactivateInterval();
void CLPalette_Send2Screen(struct color_t *palette, unsigned short first, unsigned short count);
void CLPalette_BeBlack();
void CLPalette_BeSystem();

void CLFile_SetFilter(int a3, int a4, int a5, int a6, int a7);
void CLFile_SetFinderInfos(void *fs, int a4, int a5);
void CLFile_GetFullPath(void *a3, char *a4);
void CLFile_MakeStruct(int a3, int a4, char *name, filespec_t *fs);
void CLFile_Create(filespec_t *fs);
void CLFile_Open(filespec_t *fs, short mode, file_t &handle);
void CLFile_Close(file_t &handle);
void CLFile_SetPosition(file_t &handle, short mode, long pos);
void CLFile_Read(file_t &handle, void *buffer, long *size);
void CLFile_Write(file_t &handle, void *buffer, long *size);

void CLSound_PrepareSample(sound_t *sound, short mode);
void CLSound_SetWantsDesigned(short designed);
void CLSound_SetLength(sound_t *sound, int length);

soundchannel_t *CLSoundChannel_New(int arg1);
void CLSoundChannel_Free(soundchannel_t *ch);
void CLSoundChannel_Stop(soundchannel_t *ch);
void CLSoundChannel_Play(soundchannel_t *ch, sound_t *sound);
short CLSoundChannel_GetVolume(soundchannel_t *ch);
void CLSoundChannel_SetVolume(soundchannel_t *ch, short volume);
void CLSoundChannel_SetVolumeRight(soundchannel_t *ch, short volume);
void CLSoundChannel_SetVolumeLeft(soundchannel_t *ch, short volume);

short CLKeyboard_HasCmdDown();
void CLKeyboard_Read();
unsigned char CLKeyboard_GetLastASCII();
short CLKeyboard_IsScanCodeDown(short scancode);

void CLMouse_Hide();
void CLMouse_Show();
void CLMouse_GetPosition(short *x, short *y);
void CLMouse_SetPosition(short x, short y);
unsigned short CLMouse_IsDown();

void CLView_SetSrcZoomValues(view_t *view, int x, int y);
void CLView_SetDisplayZoomValues(view_t *view, int w, int h);
void CLView_Free(view_t *view);
void CLView_InitDatas(view_t *view, int w, int h, void *buffer);
view_t *CLView_New(int w, int h);
void CLView_CenterIn(view_t *parent, view_t *child);

void CLScreenView_Init();
void CLScreenView_CenterIn(view_t *view);

void CRYOLib_InstallExitPatch();
void CRYOLib_RemoveExitPatch();
void CRYOLib_Init();
void CRYOLib_Done();
void CRYOLib_MinimalInit();
void CRYOLib_ManagersInit();
void CRYOLib_ManagersDone();
void CRYOLib_SetDebugMode(short enable);
void CRYOLib_InstallEmergencyExit(void(*proc)());
void CRYOLib_SetupEnvironment();
void CRYOLib_RestoreEnvironment();
void CRYOLib_TestConfig();

short CLComputer_Has68030();
short CLComputer_Has68040();
void CLDesktop_TestOpenFileAtStartup();


void CLHNM_DecompLempelZiv(unsigned char *buffer, unsigned char *output);
void CLHNM_DecompUBA(unsigned char *output, unsigned char *curr_buffer, unsigned char *prev_buffer,
                     unsigned char *input, int width, char flags);
void CLHNM_Init();
void CLHNM_Done();
void CLHNM_SetupTimer(float rate);
void CLHNM_WaitLoop(hnm_t *hnm);
void CLHNM_SetupSound(short numSounds, short arg4, short sampleSize, float rate, short mode);
void CLHNM_SetupSoundADPCM(short numSounds, short arg4, short sampleSize, float rate, short mode);
void CLHNM_CloseSound();
void CLHNM_SetForceZero2Black(short forceblack);
hnm_t *CLHNM_New(int preload_size);
void CLHNM_Dispose(hnm_t *hnm);
void CLHNM_SetFile(hnm_t *hnm, file_t *file);
void CLHNM_SetFinalBuffer(hnm_t *hnm, unsigned char *buffer);
void CLHNM_AllocMemory(hnm_t *hnm);
void CLHNM_DeallocMemory(hnm_t *hnm);
void CLHNM_Read(hnm_t *hnm, int size);
void CLHNM_GiveTime(hnm_t *hnm);
void CLHNM_CanLoop(hnm_t *hnm, short can_loop);
void CLHNM_SelectBuffers(hnm_t *hnm);
void CLHNM_ChangePalette(hnm_t *hnm);
void CLHNM_Desentrelace(hnm_t *hnm);
void CLHNM_FlushPreloadBuffer(hnm_t *hnm);
soundchannel_t *CLHNM_GetSoundChannel();
void CLHNM_TryRead(hnm_t *hnm, int size);
void CLHNM_ResetInternalTimer();
void CLHNM_Reset(hnm_t *hnm);
short CLHNM_LoadFrame(hnm_t *hnm);
void CLHNM_WantsSound(short sound);
void CLHNM_LoadDecompTable(short *buffer);
void CLHNM_DecompADPCM(unsigned char *buffer, short *output, int size);
void CLHNM_SoundInADPCM(short is_adpcm);
void CLHNM_SoundMono(short is_mono);
short CLHNM_NextElement(hnm_t *hnm);
void CLHNM_ReadHeader(hnm_t *hnm);
short CLHNM_GetVersion(hnm_t *hnm);
int CLHNM_GetFrameNum(hnm_t *hnm);
void CLHNM_DeactivatePreloadBuffer();
void CLHNM_Prepare2Read(hnm_t *hnm, int mode);
void CLHNM_SetPosIntoFile(hnm_t *hnm, long pos);
void CLHNM_Desentrelace320(unsigned char *frame_buffer, unsigned char *final_buffer, unsigned short height);


} // End of namespace Cryo

#endif
