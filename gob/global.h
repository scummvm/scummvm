/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
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
	char pressedKeys[128];

	char useMouse;
	int16 mousePresent;

	int16 presentCGA;
	int16 presentEGA;
	int16 presentVGA;
	int16 presentHER;

	int16 videoMode;

	int16 disableVideoCfg;

	uint16 presentSound;
	uint16 soundFlags;
	int16 disableSoundCfg;
	int16 blasterPort;

	uint16 disableLangCfg;
	uint16 language;

	// Timer variables
	int32 startTime;
	int16 timer_delta;

	int16 frameWaitTime;
	int32 startFrameTime;

	// Mouse
	int16 disableMouseCfg;

	int16 mouseXShift;
	int16 mouseYShift;
	int16 mouseMaxCol;
	int16 mouseMaxRow;

	// Timer and delays
	int16 delayTime;

	// Joystick
	char useJoystick;

	// Files
	Common::File filesHandles[MAX_FILES];

	// Data files
	struct DataIO::ChunkDesc *dataFiles[MAX_DATA_FILES];
	int16 numDataChunks[MAX_DATA_FILES];
	int16 dataFileHandles[MAX_DATA_FILES];
	int32 chunkPos[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 chunkOffset[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 chunkSize[MAX_SLOT_COUNT * MAX_DATA_FILES];
	char isCurrentSlot[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 packedSize;

	int16 sprAllocated;

	int16 primaryWidth;
	int16 primaryHeight;

	int16 doRangeClamp;

	char redPalette[256];
	char greenPalette[256];
	char bluePalette[256];

	int16 setAllPalette;

	Video::SurfaceDesc *curPrimaryDesc;
	Video::SurfaceDesc *allocatedPrimary;
	Video::SurfaceDesc_t primarySurfDesc;
	Video::SurfaceDesc *pPrimarySurfDesc;

	int16 oldMode;
	char dontSetPalette;

	Video::PalDesc *pPaletteDesc;

	int16 unusedPalette1[18];
	int16 unusedPalette2[16];
	Video::Color vgaPalette[16];
	Video::PalDesc paletteStruct;

	int16 debugFlag;
	int16 inVM;
	int16 colorCount;

	char inter_resStr[200];
	int32 inter_resVal;

	char *inter_variables;
	char *inter_execPtr;
	int16 inter_animDataSize;

	int16 inter_mouseX;
	int16 inter_mouseY;

	char *tmpPalBuffer;

	Global(GobEngine *vm);

protected:
	GobEngine *_vm;
};

} // End of namespace Gob

#endif
