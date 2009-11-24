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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.h $
 * $Id: osys_psp.h 43618 2009-08-21 22:44:49Z joostp $
 *
 */

#ifndef PSPKEYBOARD_H
#define PSPKEYBOARD_H

#include "common/events.h"
#include "common/stream.h"
#include <pspctrl.h>

//number of modes
#define MODE_COUNT 4
#define guiStringsSize 8 /* size of guistrings array */

class PSPKeyboard {

private:
	enum State {
		kInvisible,
		kDefault,
		kCornersSelected,
		kLTriggerDown,
		kRTriggerDown,
		kMove
	};

public:
	PSPKeyboard();
	~PSPKeyboard();
	bool load();												// Load keyboard into memory
	bool isInit() { return _init; }								// Check for initialization
	bool isDirty() { return _dirty; }							// Check if needs redrawing
	bool isVisible() { return _state != kInvisible; }			// Check if visible
	bool processInput(Common::Event &event, SceCtrlData &pad, bool &usedInput);	// Process input
	void moveTo(const int newX, const int newY);				// Move keyboard
	void render();												// Draw the keyboard onscreen

private:
	struct gu_surface {
		u32		surface_width;
		u32		surface_height;
		u32		texture_width;
		u32		texture_height;
		u8		*texture;
		u32		*palette;
		u32		paletteSize;
	};

	
// structures used for drawing the keyboard
	struct Vertex {
		float u, v;
		unsigned int color;
		float x,y,z;
	};
	
	void surface_draw_offset(struct gu_surface* surface, 
				int screenX, int screenY, int offsetX, int offsetY, int intWidth, int intHeight);
	void block_copy(gu_surface* surface, u8 *texture);
	int load_png_image(Common::SeekableReadStream *, unsigned char *ImageBuffer, uint32 *palette);
	int get_png_image_size(Common::SeekableReadStream *, uint32 *png_width, uint32 *png_height, u32 *paletteSize);
	uint32 convert_pow2(uint32 size);
	void flipNibbles(gu_surface* surface);		// Convert to PSP 4-bit format
			
	static short _modeChar[MODE_COUNT][5][6];
	static const char *_guiStrings[];
	bool _init;
	unsigned int _prevButtons;	// A bit pattern.
	bool _dirty;        		// keyboard needs redrawing
	int _mode;          		// charset selected. (0 - letters or 1 - numbers)
	int _moved_x;				// location we've moved the KB to onscreen
	int _moved_y;
	bool _moved;				// whether the keyboard was moved
	gu_surface	_keyTextures[guiStringsSize];
		
	State _state;				// State of keyboard Keyboard state machine
	State _lastState;
		
	enum Cursor {
		kUp = 0,
		kRight,
		kDown,
		kLeft,
		kCenter
	};
	
	Cursor _oldCursor;			// Point to place of last cursor

};

#endif /* PSPKEYBOARD_H */