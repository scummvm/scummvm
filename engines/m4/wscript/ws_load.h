
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

#ifndef M4_WSCRIPT_WS_LOAD_H
#define M4_WSCRIPT_WS_LOAD_H

#include "m4/m4_types.h"
#include "m4/fileio/sys_file.h"
#include "m4/mem/reloc.h"
#include "m4/wscript/ws_machine.h"
#include "m4/wscript/ws_univ.h"

namespace M4 {

#define _WS_ASSET_MACH	0
#define _WS_ASSET_SEQU	1
#define _WS_ASSET_CELS	2
#define _WS_ASSET_DATA	3

#define CHUNK_MACH	0x4D414348	//'MACH'
#define CHUNK_SEQU	0x53455155	//'SEQU'
#define CHUNK_DATA	0x44415441	//'DATA'
#define CHUNK_CELS	0x43454C53	//'CELS'

#define CHUNK_NECS	0x4E454353	//INTEL 'SCEN'
#define CHUNK_HCAM	0x4843414D	//INTEL 'MACH'
#define CHUNK_UQES	0x55514553	//INTEL 'SEQU'
#define CHUNK_SLEC	0x534C4543	//INTEL 'CELS'
#define CHUNK_ATAD	0x41544144	//INTEL 'DATA'

#define MACH_NUM_STATES		0
#define MACH_OFFSETS		1

#define SEQU_NUM_VARS		0
#define SEQU_SEQU_START		1

#define DATA_REC_COUNT		0
#define DATA_REC_SIZE		1
#define DATA_REC_START		2

#define MAX_ASSET_HASH		255

struct WSLoad_Globals {
	bool _wsloaderInitialized = false;
	char **_globalMACHnames = nullptr;
	char **_globalSEQUnames = nullptr;
	char **_globalDATAnames = nullptr;
	char **_globalCELSnames = nullptr;

	MemHandle *_globalMACHHandles = nullptr;
	int32 *_globalMACHoffsets = nullptr;
	MemHandle *_globalSEQUHandles = nullptr;
	int32 *_globalSEQUoffsets = nullptr;
	MemHandle *_globalDATAHandles = nullptr;
	int32 *_globalDATAoffsets = nullptr;
	MemHandle *_globalCELSHandles = nullptr;
	int32 *_globalCELSoffsets = nullptr;
	int32 *_globalCELSPaloffsets = nullptr;
};

bool InitWSAssets();
bool ClearWSAssets(uint32 assetType, int32 minHash, int32 maxHash);
void ShutdownWSAssets();

bool LoadWSAssets(const char *wsAssetName, RGB8 *myPalette);
int32 AddWSAssetCELS(const char *wsAssetName, int32 hash, RGB8 *myPalette);
M4sprite *GetWSAssetSprite(char *spriteName, uint32 hash, uint32 index, M4sprite *mySprite, bool *streamSeries);

CCB *GetWSAssetCEL(uint32 hash, uint32 index, CCB *myCCB);
int32 GetWSAssetCELCount(uint32 hash);
int32 GetWSAssetCELFrameRate(uint32 hash);
int32 GetWSAssetCELPixSpeed(uint32 hash);
int32 ws_get_sprite_width(uint32 hash, int32 index);
int32 ws_get_sprite_height(uint32 hash, int32 index);
MemHandle ws_GetSEQU(uint32 hash, int32 *numLocalVars, int32 *offset);
MemHandle ws_GetMACH(uint32 hash, int32 *numStates, int32 *stateTableOffset, int32 *machInstrOffset);
MemHandle ws_GetDATA(uint32 hash, uint32 index, int32 *rowOffset);
int32 ws_GetDATACount(uint32 hash);
int32 GetSSHeaderInfo(Common::SeekableReadStream *stream, uint32 **data, RGB8 *myPalette);
bool ws_GetSSMaxWH(MemHandle ssHandle, int32 ssOffset, int32 *maxW, int32 *maxH);

// USING SPRITES WITHOUT GOING THROUGH THE WOODSCRIPT TREE
int32 LoadSpriteSeries(const char *assetName, MemHandle *seriesHandle, int32 *celsOffset, int32 *palOffset, RGB8 *myPalette);
int32 LoadSpriteSeriesDirect(const char *assetName, MemHandle *seriesHandle, int32 *celsOffset, int32 *palOffset, RGB8 *myPalette);
M4sprite *CreateSprite(MemHandle resourceHandle, int32 handleOffset, int32 index, M4sprite *mySprite, bool *streamSeries);

// WOODSCRIPT STREAMING API
bool ws_OpenSSstream(SysFile *streamFile, Anim8 *anim8);
bool ws_GetNextSSstreamCel(Anim8 *anim8);
void ws_CloseSSstream(CCB *myCCB);

} // End of namespace M4

#endif
