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

#ifdef __APPLE__
#include <malloc/malloc.h>
#define MALLOC_SIZE  malloc_size
#else
#include <malloc.h>
#ifdef __linux__
#define MALLOC_SIZE  malloc_usable_size
#else
#define MALLOC_SIZE  _msize
#endif
#endif
#include "common/archive.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/types.h"
#include "watchmaker/utils.h"
#include "watchmaker/windows_hacks.h"
#include <cstdlib>

namespace Watchmaker {

uint32 t3dAllocatedMemory = 0;

char bUsingFastFile = 0;

char bForceNOFastFile = 0;  // forza a non utilizzare il FastFile, nonostante questo sia attivo
char bForceNOFastFile_bUsingFastFile_backup = 0;    //valore precedente la chiamata di ForceNOFastFile

//------------------------- Memory Functions -------------------------------

//..........................................................................
void *t3dMalloc(uint32 n) {
	uint32 *res;

	if (!(res = static_cast<uint32 *>(malloc(n))))
		warning("t3dMalloc: Can't alloc %d bytes", n);

	t3dAllocatedMemory += n;

	return (res);
}

void *t3dCalloc(uint32 n) {
	uint32 *res;

	if (!(res = static_cast<uint32 *>(calloc(n, 1))))
		warning("t3dCalloc: Can't alloc %d bytes", n);

	t3dAllocatedMemory += n;

	return (res);
}

void *t3dRealloc(void *pp, uint32 additionalBytes) {


	uint32 *res = (uint32 *)pp;
	uint32 size = 0;
	if (pp == nullptr)    size = 0;
	else
		size = (uint32) MALLOC_SIZE(pp);
	//Reallocate and show new size:
	if ((res = (uint32 *)realloc(pp, (additionalBytes + size))) == nullptr) {
		warning("t3dRealloc: Memory allocation error: can't alloc %d bytes", additionalBytes + size);
		return res;
	}

	t3dAllocatedMemory += additionalBytes;
	//malloc_size()
	pp = res;//realloc(pp, additionalBytes);
	//warning("t3dRealloc() size: %d, additionalBytes: %d, newSize = %d\n", size, additionalBytes, malloc_size(pp));
	return (pp);
}

void t3dFree(void *p) {
	if (!p) return;

	t3dAllocatedMemory -= (uint32) MALLOC_SIZE(p);

	free(p);
}


//------------------------- Time Functions ---------------------------------

//..........................................................................
void t3dStartTime() {
	warning("STUBBED t3dStartTime\n");
#if 0
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(tc.wPeriodMin);
	srand((unsigned)time(NULL));
#endif
}

void t3dEndTime() {
	warning("STUBBED t3dEndTime\n");
#if 0
	timeEndPeriod(tc.wPeriodMin);
#endif
}

uint32 t3dReadTime() {
	return (timeGetTime());
}

//--------------------------------------------------------------------------
//------------------------- File I/O Functions -----------------------------

bool t3dFastFileInit(const char *name) {
	warning("STUBBED t3dFastFileInit\n");
#if 0
	bUsingFastFile = FastFileInit(name);
	return (BOOL)(bUsingFastFile);
#endif
	return false;
}

void t3dForceNOFastFile(char valore) {
	if (valore) {
		bForceNOFastFile_bUsingFastFile_backup = bUsingFastFile;
		bUsingFastFile = 0;
	} else bUsingFastFile = bForceNOFastFile_bUsingFastFile_backup;
}

int t3dAccessFile(char *name) {
	error("STUBBED: t3dAccessFile\n");
#if 0
	FILE *tempFile = nullptr;
	if (bUsingFastFile)        tempFile = (FILE *)FastFileOpen(name);
	else                        tempFile = fopen(name, "rb");

	if (tempFile == NULL)
		return 0;

	if (bUsingFastFile)        FastFileClose((LPFILEHANDLE)tempFile);
	else                        fclose(tempFile);

	return 1;
#endif
	return 0;
}

bool t3dGetFileDate(uint32 *date, uint32 *time, const char *name) {
#if 0
	FILE *f = fopen(name, "rb");
	//warning("STUBBED: t3dGetFileDate(%s)\n", name);
	if (f) {
		fclose(f);
		return true;
	}
#endif
	error("TODO: t3dGetFileDate");
	return false;
}

Common::SharedPtr<Common::SeekableReadStream> openFile(const Common::String &filename) {
	Common::String adjustedPath;
	if (filename.hasPrefix("./")) {
		adjustedPath = filename.substr(2, filename.size());
	} else {
		adjustedPath = filename;
	}
	Common::SeekableReadStream *file = nullptr;
	// Try directly from SearchMan first
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, adjustedPath);

	for (Common::ArchiveMemberList::iterator it = files.begin(); it != files.end(); ++it) {
		if ((*it)->getName().equalsIgnoreCase(lastPathComponent(adjustedPath,'/'))) {
			file = (*it)->createReadStream();
			break;
		}
	}
	return Common::SharedPtr<Common::SeekableReadStream>(file);
}

} // End of namespace Watchmaker