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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BE_OS5_H
#define BE_OS5_H

#include "be_base.h"

#if !defined(SYSTEM_CALLBACK) || defined(PALMOS_68K)
#	define SYSTEM_CALLBACK
#	ifdef PALMOS_ARM
#		define CALLBACK_PROLOGUE \
			__asm { \
				stmfd  r13!,{r9,r10}; \
				ldr	   r9,[r0]; \
				ldr	   r10,[r0,#4]; \
			}
#		define CALLBACK_EPILOGUE __asm { ldmfd  r13!,{r9,r10} }
#		define CALLBACK_INIT(regs) \
			__asm { \
				ldr  r0, = regs; \
				add	 r0,r0,r10; \
				str	 r9,[r0]; \
				str	 r10,[r0,#4]; \
			}
#	else
#		define CALLBACK_PROLOGUE \
			asm (	\
				movem.l a4-a5, -(sp);	\
				move.l UserDataP, a0;	\
				move.l 0(a0), a4;	\
				move.l 4(a0), a5;	\
			);
#		define CALLBACK_EPILOGUE asm ( movem.l (sp)+, a4-a5 );
#		define CALLBACK_INIT(regs) \
			{	\
				void *ptr = &regs; \
				asm (	\
					move.l	ptr, a0;	\
					move.l	a4, 0(a0);	\
					move.l	a5, 4(a0);	\
				); \
			}
#	endif
#else
#	define CALLBACK_PROLOGUE
#	define CALLBACK_EPILOGUE
#	define CALLBACK_INIT(regs)
#endif

// TODO : change / remove this
#define gfxMakeDisplayRGB_BigEndian(_r,_g,_b) \
  ( (((_g) & 0xFC) << 11) | (((_b) & 0xF8) << 5) | ((_r) & 0xF8) | (((_g) & 0xFF) >> 5) )

#define gfxMakeDisplayRGB_LittleEndian(_r,_g,_b) \
  ( (((_r) & 0xF8) << 8) | (((_g) & 0xFC) << 3) | (((_b) & 0xF8) >> 3) )

#if CPU_TYPE == CPU_68K
#define gfxMakeDisplayRGB(_r,_g,_b) gfxMakeDisplayRGB_BigEndian(_r,_g,_b)
#else
#define gfxMakeDisplayRGB(_r,_g,_b) gfxMakeDisplayRGB_LittleEndian(_r,_g,_b)
#endif

typedef struct {
	// for real thread version only
	UInt32 __reg1;
	UInt32 __reg2;

	// no real thread version
	Boolean set;
	UInt32	size;
	void *dataP;

	// default sound stuff
	SndStreamRef handle;
	SoundPtr sound;
} SoundExType, *SoundExPtr;
extern SoundExType _soundEx;

class OSystem_PalmOS5 : public OSystem_PalmBase {
protected:
	uint16 _nativePal[256], _mousePal[256];

private:
	uint16 _scaleTableX[512];
	uint32 _scaleTableY[512];

	typedef void (OSystem_PalmOS5::*RendererProc)(RectangleType &r, PointType &p);
	RendererProc _render;

	Graphics::Surface _framebuffer;

	OverlayColor *_overlayP;
	WinHandle _overlayH, _workScreenH;
	uint16 *_workScreenP;

	Boolean _isSwitchable, _wasRotated;

	virtual void int_initBackend();
	virtual void int_updateScreen();
	virtual void int_initSize(uint w, uint h);

	virtual void unload_gfx_mode();
	virtual void load_gfx_mode();
	virtual void hotswap_gfx_mode(int mode);

	void draw_mouse();
	void undraw_mouse();
	virtual bool check_event(Common::Event &event, EventPtr ev);
	void extras_palette(uint8 index, uint8 r, uint8 g, uint8 b);
	void calc_scale();

	void render_landscapeAny(RectangleType &r, PointType &p);
	void render_landscape15x(RectangleType &r, PointType &p);
	void render_1x(RectangleType &r, PointType &p);
	WinHandle alloc_screen(Coord w, Coord h);
	virtual void draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color = 0);

	virtual SndStreamVariableBufferCallback sound_callback();
	virtual void sound_handler();
	virtual bool setupMixer();
	void clearSoundCallback();

protected:
	UInt16 _sysOldCoord, _sysOldOrientation, _sysOldTriggerState;
	Boolean _stretched, _cursorPaletteDisabled;

	enum {
		kRatioNone = 0,
		kRatioHeight,
		kRatioWidth
	};
	struct {
		UInt8 adjustAspect;
		Coord width;	// (width x 320)
		Coord height;	// (480 x height)
	} _ratio;

	void calc_rect(Boolean fullscreen);
	void get_coordinates(EventPtr ev, Coord &x, Coord &y);
	void clear_screen();

public:
	OSystem_PalmOS5();
	static OSystem *create();

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);

	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();

	void setCursorPalette(const byte *colors, uint start, uint num);
	void disableCursorPalette(bool disable);

	void showOverlay();
	void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);

	void setWindowCaption(const char *caption);

};

#endif
