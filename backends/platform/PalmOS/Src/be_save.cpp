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

Common::StringList PalmSaveFileManager::listSavefiles(const char *pattern) {
	FileRef fileRef;
	Common::StringList list;

	// try to open the dir
	Err e = VFSFileOpen(gVars->VFS.volRefNum, SCUMMVM_SAVEPATH, vfsModeRead, &fileRef);
	if (e != errNone)
		return list;


	UInt32 dirEntryIterator = vfsIteratorStart;
	Char filename[32];
	FileInfoType info = {0, filename, 32};

	while (dirEntryIterator != vfsIteratorStop) {
		e = VFSDirEntryEnumerate (fileRef, &dirEntryIterator, &info);

		if (e != expErrEnumerationEmpty)					// there is something
			if (Common::matchString(info.nameP, pattern))	// this seems to be what we are looking for
				list.push_back(info.nameP);
	}

	VFSFileClose(fileRef);
	return list;
}
