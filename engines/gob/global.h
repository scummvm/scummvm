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

#include "gob/dataio.h"
#include "gob/video.h"

#include "common/file.h"

namespace Gob {

#define VIDMODE_CGA	0x05
#define VIDMODE_EGA	0x0d
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

#define F1_KEY	0x3b00
#define F2_KEY	0x3c00
#define F3_KEY	0x3d00
#define F4_KEY	0x3e00
#define F5_KEY	0x3f00
#define F6_KEY	0x4000
#define ESCAPE	0x001b
#define ENTER	0x000d

#define MAX_FILES	30

/* Video drivers */
#define UNK_DRIVER	0
#define VGA_DRIVER	1
#define EGA_DRIVER	2
#define CGA_DRIVER	3
#define HER_DRIVER	4

class Global {
public:
	char _pressedKeys[128];

	char _useMouse;
	int16 _mousePresent;

	int16 _presentCGA;
	int16 _presentEGA;
	int16 _presentVGA;
	int16 _presentHER;

	int16 _videoMode;

	int16 _disableVideoCfg;

	uint16 _soundFlags;
	int16 _disableSoundCfg;
	int16 _blasterPort;

	uint16 _disableLangCfg;
	uint16 _language;

	// Timer variables
	int32 _startTime;
	int16 _timer_delta;

	int16 _frameWaitTime;
	int32 _startFrameTime;

	// Mouse
	int16 _disableMouseCfg;

	int16 _mouseXShift;
	int16 _mouseYShift;
	int16 _mouseMaxCol;
	int16 _mouseMaxRow;

	// Timer and delays
	int16 _delayTime;

	// Joystick
	char _useJoystick;

	// Files
	Common::File _filesHandles[MAX_FILES];

	// Data files
	struct DataIO::ChunkDesc *_dataFiles[MAX_DATA_FILES];
	int16 _numDataChunks[MAX_DATA_FILES];
	int16 _dataFileHandles[MAX_DATA_FILES];
	int32 _chunkPos[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _chunkOffset[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _chunkSize[MAX_SLOT_COUNT * MAX_DATA_FILES];
	char _isCurrentSlot[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _packedSize;

	int16 _sprAllocated;

	int16 _primaryWidth;
	int16 _primaryHeight;

	int16 _doRangeClamp;

	char _redPalette[256];
	char _greenPalette[256];
	char _bluePalette[256];

	int16 _setAllPalette;

	Video::SurfaceDesc _primarySurfDesc;
	Video::SurfaceDesc *_pPrimarySurfDesc;

	int16 _oldMode;
	char _dontSetPalette;

	Video::PalDesc *_pPaletteDesc;

	int16 _unusedPalette1[18];
	int16 _unusedPalette2[16];
	Video::Color _vgaPalette[16];
	Video::PalDesc _paletteStruct;

	int16 _debugFlag;
	int16 _inVM;
	int16 _colorCount;

	char _inter_resStr[200];
	int32 _inter_resVal;

	byte *_inter_variablesSizes; // 0: single byte, 1: two bytes, 3: four bytes
	char *_inter_variables;
	char *_inter_execPtr;
	int16 _inter_animDataSize;

	int16 _inter_mouseX;
	int16 _inter_mouseY;

	// While using the notepad or changing the font, the original executable
	// temporarily dumps Draw::_backSurface to a file. Since that's not really
	// a nice thing to do, we work around it.
	Video::SurfaceDesc *_savedBack;
	Video::Color _savedPal[256];
	int32 _savedBackSize;

	inline void clearVars(uint32 count)
	{
		uint32 i;

		for (i = 0; i < count; i++) {
			_inter_variablesSizes[i * 4] = 3;
			_inter_variablesSizes[i * 4 + 1] = 0;
			_inter_variablesSizes[i * 4 + 2] = 0;
			_inter_variablesSizes[i * 4 + 3] = 0;
			_inter_variables[i * 4] = 0;
			_inter_variables[i * 4 + 1] = 0;
			_inter_variables[i * 4 + 2] = 0;
			_inter_variables[i * 4 + 3] = 0;
		}
	}

	inline void writeVarSizeStr(uint32 offset, uint32 len) {
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

	inline void writeVar(uint32 offset, uint32 val) {
		(*(uint32 *)(_inter_variables + offset)) = val;
		writeVarSize(offset, 3);
	}
	inline void writeVar(uint32 offset, uint16 val) {
		(*(uint16 *)(_inter_variables + offset)) = val;
		writeVarSize(offset, 1);
	}
	inline void writeVar(uint32 offset, uint8 val) {
		(*(uint8 *)(_inter_variables + offset)) = val;
		writeVarSize(offset, 0);
	}
	inline void writeVar(uint32 offset, const char *str) {
		writeVarSizeStr(offset, strlen(str));
		strcpy(_inter_variables + offset, str);
	}

	Global(GobEngine *vm);

protected:
	GobEngine *_vm;

	inline void writeVarSize(uint32 offset, byte n) {
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

#endif
