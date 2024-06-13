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
#include "common/macresman.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/string.h"
#include "bagel/baglib/bagel.h"
#include "bagel/bagel.h"

namespace Bagel {

bool fileExists(const char *pszFileName) {
	if (g_engine->getPlatform() == Common::kPlatformMacintosh) {
		return Common::MacResManager::exists(pszFileName);
	} else {
		return Common::File::exists(pszFileName);
	}
}

int32 fileLength(const char *pszFileName) {
	Common::SeekableReadStream *stream = nullptr;
	if (g_engine->getPlatform() == Common::kPlatformMacintosh) {
		stream = Common::MacResManager::openFileOrDataFork(pszFileName);
	} else {
		stream = SearchMan.createReadStreamForMember(pszFileName);
	}

	int32 length = -1;
	if (stream) {
		length = stream->size();
		delete stream;
	}
	return length;
}

char *fileGetFullPath(char *pszDstBuf, const char *pszSrcBuf) {
	Common::strcpy_s(pszDstBuf, MAX_DIRPATH, pszSrcBuf);
	return pszDstBuf;
}

const char *formPath(const char *dir, const char *pszFile) {
	assert(dir != nullptr && pszFile != nullptr);
	static char szBuf[MAX_DIRPATH];

	CBofString cStr(dir);
	fixPathName(cStr);
	Common::Path path(cStr.getBuffer());
	path = path.append(pszFile);

	Common::strcpy_s(szBuf, path.toString().c_str());

	return &szBuf[0];
}

} // namespace Bagel
