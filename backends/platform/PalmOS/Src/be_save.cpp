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

#include "be_base.h"
#include "common/savefile.h"
#include "be_save.h"

void PalmSaveFileManager::listSavefiles(const char *prefix, bool *marks, int num) {
	FileRef fileRef;
	// try to open the dir
	Err e = VFSFileOpen(gVars->VFS.volRefNum, getSavePath(), vfsModeRead, &fileRef);
	memset(marks, false, num*sizeof(bool));

	if (e != errNone)
		return;

	// enumerate all files
	UInt32 dirEntryIterator = vfsIteratorStart;
	Char filename[32];
	FileInfoType info = {0, filename, 32};
	UInt16 length = StrLen(prefix);
	int slot = 0;

	while (dirEntryIterator != vfsIteratorStop) {
		e = VFSDirEntryEnumerate (fileRef, &dirEntryIterator, &info);

		if (e != expErrEnumerationEmpty) {									// there is something

			if (StrLen(info.nameP) == (length + 2)) {						// consider max 99, filename length is ok
				if (StrNCaselessCompare(prefix, info.nameP, length) == 0) { // this seems to be a save file
					if (isdigit(info.nameP[length]) && isdigit(info.nameP[length+1])) {

						slot = StrAToI(filename + length);
						if (slot >= 0 && slot < num)
							*(marks+slot) = true;

					}
				}
			}

		}
	}

	VFSFileClose(fileRef);
}
