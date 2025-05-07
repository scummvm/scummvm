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

#include "common/file.h"
#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

CFile::CFile(const char *lpszFileName, UINT nOpenFlags) {
	if (!Open(lpszFileName, nOpenFlags))
		error("Could not open - %s", lpszFileName);
}

BOOL CFile::Open(const char *lpszFileName, UINT nOpenFlags, CFileException *pError) {
	Close();

	Common::File *f = new Common::File();
	if (f->open(lpszFileName)) {
		_stream = f;
		return true;
	} else {
		delete f;
		return false;
	}
}

void CFile::Close() {
	delete _stream;
	_stream = nullptr;
}

} // namespace MFC
} // namespace Bagel
