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

#include "common/substream.h"
#include "common/archive.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/types.h"
#include "watchmaker/utils.h"
#include "watchmaker/windows_hacks.h"

namespace Watchmaker {

char bUsingFastFile = 0;

char bForceNOFastFile = 0;  // forza a non utilizzare il FastFile, nonostante questo sia attivo
char bForceNOFastFile_bUsingFastFile_backup = 0;    //valore precedente la chiamata di ForceNOFastFile

//------------------------- Memory Functions -------------------------------

//..........................................................................
void *t3dMalloc(uint32 n) {
	uint32 *res;

	if (!(res = static_cast<uint32 *>(malloc(n))))
		warning("t3dMalloc: Can't alloc %d bytes", n);

	return (res);
}

void *t3dCalloc(uint32 n) {
	uint32 *res;

	if (!(res = static_cast<uint32 *>(calloc(n, 1))))
		warning("t3dCalloc: Can't alloc %d bytes", n);

	return (res);
}

void t3dFree(void *p) {
	if (!p) return;

	free(p);
}


//------------------------- Time Functions ---------------------------------

//..........................................................................
void t3dStartTime() {
	warning("STUBBED t3dStartTime");
#if 0
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(tc.wPeriodMin);
	srand((unsigned)time(NULL));
#endif
}

void t3dEndTime() {
	warning("STUBBED t3dEndTime");
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
	warning("STUBBED t3dFastFileInit");
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
	warning("TODO: t3dGetFileDate is currently super-inefficient: %s", name);
	*date = 0;
	*time = 0;
	return checkFileExists(name);
}

Common::String adjustPath(const Common::String &path) {
	Common::String adjustedPath;
	if (path.hasPrefix("./")) {
		adjustedPath = path.substr(2, path.size());
	} else {
		adjustedPath = path;
	}
	return adjustedPath;
}

bool checkFileExists(const Common::String &filename) {
	Common::String adjustedPath = adjustPath(filename);

	return SearchMan.hasFile(adjustedPath);
}

Common::SharedPtr<Common::SeekableReadStream> openFile(const Common::String &filename, int offset, int size) {
	Common::String adjustedPath = adjustPath(filename);

	Common::SeekableReadStream *file = SearchMan.createReadStreamForMember(adjustedPath);

	if (offset != 0 || size != -1) {
		if (size == -1) {
			size = file->size();
		}
		assert(size <= file->size());
		assert(offset >= 0 && offset <= file->size());
		file = new Common::SeekableSubReadStream(file, offset, offset + size, DisposeAfterUse::YES);
	}
	return Common::SharedPtr<Common::SeekableReadStream>(file);
}

} // End of namespace Watchmaker
