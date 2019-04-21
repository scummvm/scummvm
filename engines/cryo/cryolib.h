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

class CryoEngine;

#define SW16(n) ( (((n) & 0xFF) << 8) | (((n) >> 8) & 0xFF) )
#define SW32(n) ( (((n) & 0xFF) << 24) | (((n) >> 24) & 0xFF) | (((n) & 0xFF00) << 8) | (((n) >> 8) & 0xFF00))
#ifdef SCUMM_BIG_ENDIAN
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

class View {
public:
	View(int w, int h);
	~View();

	void setSrcZoomValues(int x, int y);
	void setDisplayZoomValues(int w, int h);
	void initDatas(int w, int h, void *buffer);
	void centerIn(View *parent);

	int      _width;
	int      _height;
	byte    *_bufferPtr;
	int16    _pitch;
	BlitView _normal;
	BlitView _zoom;
};

struct color3_t {
	uint16   r, g, b;
};

struct color_t {
	uint16   a, r, g, b;
};

struct HNMHeader {
	int32   _signature;
	uint16  _width;
	uint16  _height;
	int32   _numbFrame;
	int32   _bufferSize;
};

class Sound {
private:
	int32  _headerOffset;
	int16  _mode;
	int16  _volume;

public:
	Sound(int16 length, float rate, int16 sampleSize, int16 mode);
	~Sound();

	void assignBuffer(void *buffer, int bufferOffs, int length);
	void prepareSample(int16 mode);
	void setWantsDesigned(int16 designed);

	char  *_sndHandle;
	char  *_buffer;

	float  _rate;

	int16  _maxLength;
	int16  _headerLen;
	int16  _sampleSize;

	int    _length;
};

#define kCryoMaxChSounds 10

class SoundChannel {
private:
	int16   _volumeLeft;
	int16   _volumeRight;
	int16   _numSounds;

	Sound *_sounds[kCryoMaxChSounds];

public:
	SoundChannel(int arg1);
	~SoundChannel();

	void stop();
	void play(Sound *sound);
	int16 getVolume();
	void setVolume(int16 volume);
	void setVolumeRight(int16 volume);
	void setVolumeLeft(int16 volume);
};

void SysBeep(int x);
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
void CLPalette_BeSystem();

void CRYOLib_ManagersInit();
void CRYOLib_ManagersDone();

} // End of namespace Cryo

#endif
