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
 * Graphics Memory Manager data structures
 */

#ifndef TINSEL_HANDLE_H
#define TINSEL_HANDLE_H

#include "tinsel/dw.h"			// new data types

namespace Common {
class File;
}

namespace Tinsel {

struct MEMHANDLE;

class Handle {
public:
	Handle() : g_handleTable(0), g_numHandles(0), g_cdPlayHandle((uint32)-1), g_cdBaseHandle(0), g_cdTopHandle(0), g_cdGraphStream(nullptr) {}

	/**
	 * Loads the graphics handle table index file and preloads all the permanent graphics etc.
	 */
	void SetupHandleTable();
	void FreeHandleTable();

	byte *LockMem(SCNHANDLE offset);

	void LockScene(SCNHANDLE offset);
	void UnlockScene(SCNHANDLE offset);

	bool IsCdPlayHandle(SCNHANDLE offset);

	void TouchMem(SCNHANDLE offset);

	// Called at scene startup
	void SetCdPlaySceneDetails(int sceneNum, const char *fileName);

	// Called at game startup
	void SetCdPlayHandle(int	fileNum);

	void LoadExtraGraphData(
		SCNHANDLE start,		// Handle of start of range
		SCNHANDLE next);		// Handle of end of range + 1

	int CdNumber(SCNHANDLE offset);

#ifdef BODGE
	bool ValidHandle(SCNHANDLE offset);
#endif

private:
	void LoadFile(MEMHANDLE *pH); // load a memory block as a file
	void OpenCDGraphFile();
	void LoadCDGraphData(MEMHANDLE *pH);

	// handle table gets loaded from index file at runtime
	MEMHANDLE *g_handleTable;

	// number of handles in the handle table
	uint g_numHandles;

	uint32 g_cdPlayHandle;

	SCNHANDLE g_cdBaseHandle, g_cdTopHandle;
	Common::File *g_cdGraphStream;

	char g_szCdPlayFile[100];
};

} // End of namespace Tinsel

#endif	// TINSEL_HANDLE_H
