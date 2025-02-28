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

#ifndef BAGEL_MFC_FILE_H
#define BAGEL_MFC_FILE_H

#include "bagel/mfc/mfc_types.h"

namespace Bagel {
namespace MFC {

class CFile : public Common::File {
public:
	enum {
		begin = 0,
		current = 1,
		end = 2
	};
	enum {
		modeRead = 1,
		typeBinary = 2
	};

	BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, void *pError = nullptr) {
		assert((nOpenFlags & modeRead) != 0);
		return Common::File::open(Common::Path(lpszFileName));
	}

	long Seek(long offset, int whence = CFile::begin) {
		return Common::File::seek(offset, whence);
	}
	void SeekToBegin() {
		Seek(0);
	}
	long GetPosition() const {
		return Common::File::pos();
	}
	long GetLength() const {
		return Common::File::size();
	}
	long Read(void *buf, size_t size) {
		return Common::File::read(buf, size);
	}
};

} // namespace MFC
} // namespace Bagel

#endif
