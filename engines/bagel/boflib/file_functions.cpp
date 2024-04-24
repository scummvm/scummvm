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
#include "common/file.h"
#include "common/fs.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/string.h"
#include "bagel/baglib/bagel.h"

namespace Bagel {

bool FileExists(const char *pszFileName) {
	return Common::File::exists(pszFileName);
}

int32 FileLength(const char *pszFileName) {
	Common::File f;
	return f.open(pszFileName) ? f.size() : -1;
}

char *FileGetFullPath(char *pszDstBuf, const char *pszSrcBuf) {
	Common::strcpy_s(pszDstBuf, MAX_DIRPATH, pszSrcBuf);
	return pszDstBuf;
}

const char *formPath(const char *dir, const char *pszFile) {
	Assert(dir != nullptr && pszFile != nullptr);
	static char szBuf[MAX_DIRPATH];

	CBofString cStr(dir);
	MACROREPLACE(cStr);
	Common::Path path(cStr.GetBuffer());
	path = path.append(pszFile);

	Common::strcpy_s(szBuf, path.toString().c_str());

	return &szBuf[0];
}

} // namespace Bagel
