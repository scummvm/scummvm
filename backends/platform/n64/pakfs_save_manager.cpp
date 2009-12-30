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
 */

#include <n64utils.h>
#include "pakfs_save_manager.h"

static bool matches(const char *glob, const char *name);

bool deleteSaveGame(const char *filename) {
	int res = removeFileOnPak(filename);
	flushCurrentPakData();

	return (res == 0);
}

uint32 InPAKSave::read(void *buf, uint32 cnt) {
	return pakfs_read(buf, 1, cnt, fd);
}

bool InPAKSave::seek(int32 offs, int whence) {
	pakfs_seek(fd, offs, whence);

	return true;
}

bool InPAKSave::skip(uint32 offset) {
	pakfs_seek(fd, offset, SEEK_CUR);

	return true;
}

uint32 OutPAKSave::write(const void *buf, uint32 cnt) {
	return pakfs_write(buf, 1, cnt, fd);
}

Common::StringList PAKSaveManager::listSavefiles(const Common::String &pattern) {
	PAKDIR *dirp = pakfs_opendir();
	pakfs_dirent *dp;
	Common::StringList list;

	while ((dp = pakfs_readdir(dirp)) != NULL) {
		if (matches(pattern.c_str(), dp->entryname))
			list.push_back(dp->entryname);

		free(dp);
	}

	pakfs_closedir(dirp);

	return list;
}

static bool matches(const char *glob, const char *name) {
	while (*glob)
		if (*glob == '*') {
			while (*glob == '*')
				glob++;
			do {
				if ((*name == *glob || *glob == '?') &&
				        matches(glob, name))
					return true;
			} while (*name++);
			return false;
		} else if (!*name)
			return false;
		else if (*glob == '?' || *glob == *name) {
			glob++;
			name++;
		} else
			return false;
	return !*name;
}

