/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_GLOBAL_H
#define GOB_GLOBAL_H

#include "common/file.h"

#include "gob/video.h"

namespace Gob {

#define VIDMODE_CGA	0x05
#define VIDMODE_EGA	0x0D
#define VIDMODE_VGA	0x13
#define VIDMODE_HER	7

#define PROAUDIO_FLAG	0x10
#define ADLIB_FLAG		0x08
#define BLASTER_FLAG	0x04
#define INTERSOUND_FLAG	0x02
#define SPEAKER_FLAG	0x01
#define MIDI_FLAG		0x4000

#define NO	0
#define YES	1
#define UNDEF	2

#define F1_KEY	0x3B00
#define F2_KEY	0x3C00
#define F3_KEY	0x3D00
#define F4_KEY	0x3E00
#define F5_KEY	0x3F00
#define F6_KEY	0x4000
#define ESCAPE	0x001B
#define ENTER	0x000D

/* Video drivers */
#define UNK_DRIVER	0
#define VGA_DRIVER	1
#define EGA_DRIVER	2
#define CGA_DRIVER	3
#define HER_DRIVER	4

class Global {
public:
	char _pressedKeys[128];

	int16 _presentCGA;
	int16 _presentEGA;
	int16 _presentVGA;
	int16 _presentHER;

	int16 _videoMode;
	int16 _fakeVideoMode;
	int16 _oldMode;

	int16 _frameWaitTime;
	int32 _startFrameTime;

	uint16 _soundFlags;

	uint16 _language;
	uint16 _languageWanted;

	char _useMouse;
	int16 _mousePresent;
	int16 _mouseXShift;
	int16 _mouseYShift;
	int16 _mouseMaxCol;
	int16 _mouseMaxRow;

	char _useJoystick;

	int16 _primaryWidth;
	int16 _primaryHeight;

	int16 _colorCount;
	char _redPalette[256];
	char _greenPalette[256];
	char _bluePalette[256];

	int16 _unusedPalette1[18];
	int16 _unusedPalette2[16];
	Video::Color _vgaPalette[16];
	Video::PalDesc _paletteStruct;
	Video::PalDesc *_pPaletteDesc;

	bool _setAllPalette;
	bool _dontSetPalette;

	SurfaceDesc::Ptr _primarySurfDesc;

	int16 _debugFlag;
	int16 _inVM;

	char _inter_resStr[200];
	int32 _inter_resVal;

	byte *_inter_variablesSizes; // 0: single byte, 1: two bytes, 3: four bytes
	byte *_inter_variables;
	byte *_inter_execPtr;
	int16 _inter_animDataSize;

	int16 _inter_mouseX;
	int16 _inter_mouseY;

	// While using the notepad or changing the font, the original executable
	// temporarily dumps Draw::_backSurface to a file. Since that's not really
	// a nice thing to do, we work around it.
	SurfaceDesc::Ptr _savedBack;
	Video::Color _savedPal[256];
	int32 _savedBackSize;

	void clearVars(uint32 count) {
		uint32 size = count * 4;

		memset(_inter_variables, 0, size);
		memset(_inter_variablesSizes, 0, size);
		for (uint32 i = 0; i < size; i += 4)
			_inter_variablesSizes[i] = 3;
	}

	void writeVarSizeStr(uint32 offset, uint32 len) {
		uint32 i;
		uint32 inVar;
		uint32 varOff;

		inVar = offset % 4;
		varOff = (offset >> 2) << 2;
		for (i = 0; i < 4; i++) {
			if (_inter_variablesSizes[varOff + i] == 3)
				_inter_variablesSizes[varOff + i] = 0;
			else if ((inVar == (i+1)) && (_inter_variablesSizes[varOff + i] == 1))
				_inter_variablesSizes[varOff + i] = 0;
		}
		memset(_inter_variablesSizes + offset, 0, len);
	}

	void writeVar(uint32 offset, uint32 val) {
		(*(uint32 *)(_inter_variables + offset)) = val;
		writeVarSize(offset, 3);
	}
	void writeVar(uint32 offset, uint16 val) {
		(*(uint16 *)(_inter_variables + offset)) = val;
		writeVarSize(offset, 1);
	}
	void writeVar(uint32 offset, uint8 val) {
		(*(uint8 *)(_inter_variables + offset)) = val;
		writeVarSize(offset, 0);
	}
	void writeVar(uint32 offset, const char *str) {
		writeVarSizeStr(offset, strlen(str));
		strcpy((char *) (_inter_variables + offset), str);
	}

	Global(GobEngine *vm);
	~Global();

protected:
	GobEngine *_vm;

	void writeVarSize(uint32 offset, byte n) {
		uint32 i;
		uint32 inVar;
		uint32 varOff;

		inVar = offset % 4;
		varOff = (offset >> 2) << 2;
		for (i = 0; i < 4; i++) {
			if (_inter_variablesSizes[varOff + i] == 3)
				_inter_variablesSizes[varOff + i] = 0;
			else if ((inVar == (i+1)) && (_inter_variablesSizes[varOff + i] == 1))
				_inter_variablesSizes[varOff + i] = 0;
		}

		_inter_variablesSizes[offset] = n;
		for (; n > 0; n--)
			_inter_variablesSizes[offset + n] = 0;
	}

};

} // End of namespace Gob

#endif // GOB_GLOBAL_H
