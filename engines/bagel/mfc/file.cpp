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

IMPLEMENT_DYNAMIC(CFile, CObject)

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

void CFile::Abort() {
	error("TODO: CFile::Abort");
}

Common::SeekableReadStream *CFile::readStream() const {
	Common::SeekableReadStream *rs = dynamic_cast <
	                                 Common::SeekableReadStream * > (_stream);
	assert(rs);
	return rs;
}

Common::WriteStream *CFile::writeStream() const {
	Common::WriteStream *ws = dynamic_cast <
	                          Common::WriteStream * > (_stream);
	assert(ws);
	return ws;
}

ULONGLONG CFile::SeekToEnd() {
	return readStream()->seek(0, SEEK_END);
}

void CFile::SeekToBegin() {
	readStream()->seek(0);
}

ULONGLONG CFile::Seek(LONGLONG lOff, UINT nFrom) {
	return readStream()->seek(lOff, nFrom);
}

ULONGLONG CFile::GetLength() const {
	return readStream()->size();
}

ULONGLONG CFile::GetPosition() const {
	return readStream()->pos();
}

UINT CFile::Read(void *lpBuf, UINT nCount) {
	return readStream()->read(lpBuf, nCount);
}

void CFile::Write(const void *lpBuf, UINT nCount) {
	writeStream()->write(lpBuf, nCount);
}

} // namespace MFC
} // namespace Bagel
