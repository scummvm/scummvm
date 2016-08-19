#pragma once
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

#include "audio/mixer.h"

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

