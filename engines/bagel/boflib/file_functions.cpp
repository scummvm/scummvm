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

#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/file.h"
#include "common/fs.h"
#include "bagel/boflib/file_functions.h"

namespace Bagel {

ERROR_CODE FileRename(const CHAR *pszOldName, const CHAR *pszNewName) {
	// TODO: Presume this is, if used, is for renaming savegames
	warning("TODO: FileRename %s -> %s", pszOldName, pszNewName);
	return ERR_NONE;
}

ERROR_CODE FileDelete(const CHAR *fileName) {
	g_system->getSavefileManager()->removeSavefile(fileName);
	return ERR_NONE;
}

CHAR *FileTempName(CHAR *buf) {
	// can't write to a nullptr pointer
	Assert(buf != nullptr);

	error("TODO: FileTempName - refactor to not need function");
}

BOOL FileIsDirectory(const CHAR *path) {
	Common::FSNode file(path);
	return file.isDirectory();
}

BOOL FileExists(const CHAR *pszFileName) {
	return Common::File::exists(pszFileName);
}

LONG FileLength(const CHAR *pszFileName) {
	Common::File f;
	return f.open(pszFileName) ? f.size() : -1;
}

CHAR *FileGetFullPath(CHAR *pszDstBuf, const CHAR *pszSrcBuf) {
	Common::strcpy_s(pszDstBuf, MAX_DIRPATH, pszSrcBuf);
	return pszDstBuf;
}

} // namespace Bagel
