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

#ifndef MADS_CORE_GENERAL_H
#define MADS_CORE_GENERAL_H

#include "common/stream.h"

namespace MADS {
namespace MADSV2 {

/* Code optimization parameters */

#define fastcall			/* register calling convention */
#define aggressive true     /* allow "Z" optimizations     */

/* universal interlanguage data types */

typedef uint32  dword;           /* generic 32 bit data */
typedef uint16  word;            /* generic 16 bit data */

/* vertical data types */

/* A single palette color    */
struct RGBcolor {
	byte r, g, b;

	static constexpr int SIZE = 3;
	inline void load(Common::SeekableReadStream *src) {
		r = src->readByte();
		g = src->readByte();
		b = src->readByte();
	}
};
typedef RGBcolor Palette[256];          /* An entire Mcga palette    */
typedef byte PaletteMap[256][3];        /* An entire Mcga palette #2 */

#define pal_color(p,i,c)  (*(((byte *)&p[i])+c))

typedef struct {                        /* Video buffer structure         */
	int y;                                /* Wrap value for buffer (y size) */
	int x;                                /* X size of buffer               */
	byte *data;                       /* Pointer to actual data         */
} Buffer;

#define far_string(v,s)   char _based(_segname("FARSTRING")) v[] = s;


/* Timer and clock stuff */

// ScummVM: replace the DOS hardware address with a safe writable dummy so
// that game code which writes *timer_address doesn't touch unmapped memory.
extern long _timer_clock;
#define clock_address         (&_timer_clock)
#define dos_timer_address     (&_timer_clock)

#define interrupt_controller  0x20    /* Interrupt controller  */
#define end_of_interrupt      0x20    /* End of interrupt ack  */

#define timer_controller      0x43    /* Timer controller port */
#define timer_channel_0       0x40    /* Timer channel 0 data  */
#define timer_channel_1       0x41    /* Timer channel 1 data  */
#define timer_channel_2       0x42    /* Timer channel 2 data  */

#define time_set              0x36    /* Set timer countdown 00110110b */

#define timer_speed_600       1880    /* 600/s 1960? */
#define timer_speed_300       3920    /* 300/s */
#define timer_speed_60        19600   /* 60/s  */


/* video defines */

#define video_x               320     /* Screen max X size */
#define video_y               200     /* Screen max Y size */

#define display_y             156     /* Picture area max Y size */

#define mcga_video            (byte *)g_engine->getScreen()->getPixels()

#define text_mode             0x03
#define mono_text_mode        0x07
#define tandy_mode            0x09
#define ega_mode              0x0d
#define mcga_mode             0x13

#define secret_video_area       ((color_text_video) + (PACK_EXPLODE_SIZE))
#define secret_video_size       (32768L - PACK_EXPLODE_SIZE)

/* Things-you-need-for-Tandy */

#define dos_memory            (byte *)0x00400013 /* Dos memory # */
#define abs_memory            (byte *)0x00400015 /* Abs memory # */

/* logical defines (boolean etc) */
#define yes                     true	//(-1)
#define no                      false	//0
#if 0
#define true                  (-1)
#define false                 0
#endif

constexpr int none = 0;
constexpr int stop = 99;


/* misc functional defines */

#define getrandom( min, max )   ((rand() % (int)(((max)+1) - (min))) + (min))


#define hibyte(x) ( (byte) ( (x) >> 8 ) )
#define lobyte(x) ( (byte) ( ( (word) ( (x) << 8 ) ) >> 8 ) )

#define neg(x) ((~(x))+1)                  /* Negate  w/o multiply */
#define abs(x) ( ((x)>0) ? (x) : neg(x) )  /* Abs val w/o multiply */

#define sgn(x) ( ((x)>0) ? 1 : ( ((x)<0) ? -1 : 0 ) )          /* sign          */
#define sign(x) ( ((x) > 0.0) ? 1 : ( ((x) < 0.0) ? -1 : 0 ) ) /* sign (double) */

#define sgn_in(x,s) ( ((s) >= 0) ? (x) : (neg(x)) )            /* Incorporate sign */

inline char *mads_strupr(char *str) {
	for (char *s = str; *s; ++s)
		*s = toupper(*s);
	return str;
}

inline char *mads_strlwr(char *str) {
	for (char *s = str; *s; ++s)
		*s = tolower(*s);
	return str;
}

inline char *mads_itoa(int value, char *buffer, int radix) {
	assert(radix == 10);
	Common::strcpy_s(buffer, 16, Common::String::format("%d", value).c_str());
	return buffer;
}

inline void mads_chdir(const char *path) {}
inline void mads_chdrive(int drive) {}
inline int mads_getdrive() { return 3; }
inline void mads_fullpath(char *buffer, const char *path, uint bufferCount) {
	Common::strcpy_s(buffer, bufferCount, path);
}
inline char *mads_getcwd(char *buffer, int count) {
	*buffer = '\0';
	return buffer;
}

} // namespace MADSV2
} // namespace MADS

#endif
