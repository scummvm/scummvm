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

#include "common/str.h"
#include "tinsel/dw.h"			// new data types

namespace Common {
class File;
}

namespace Tinsel {

struct MEMHANDLE;

class Handle {
public:
	Handle();
	virtual ~Handle();

	/**
	 * Loads the graphics handle table index file and preloads all the permanent graphics etc.
	 */
	void SetupHandleTable();

	byte *LockMem(SCNHANDLE offset);

	void LockScene(SCNHANDLE offset);
	void UnlockScene(SCNHANDLE offset);

	bool IsCdPlayHandle(SCNHANDLE offset);

	void TouchMem(SCNHANDLE offset);

	// Called at scene startup
	void SetCdPlaySceneDetails(const char *fileName);

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
	MEMHANDLE *_handleTable;

	// number of handles in the handle table
	uint _numHandles;

	uint32 _cdPlayHandle;

	SCNHANDLE _cdBaseHandle, _cdTopHandle;
	Common::File *_cdGraphStream;

	Common::String _szCdPlayFile;
};

} // End of namespace Tinsel

#endif	// TINSEL_HANDLE_H
