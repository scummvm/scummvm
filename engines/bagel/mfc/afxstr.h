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

#ifndef BAGEL_MFC_AFXSTR_H
#define BAGEL_MFC_AFXSTR_H

#include "bagel/mfc/minwindef.h"
#include "common/str.h"

namespace Bagel {
namespace MFC {

class CString : public Common::String {
public:
	CString() : Common::String() {}
	CString(const char *s) : Common::String(s) {}

	char *GetBufferSetLength(size_t nNewLength) {
		ensureCapacity(nNewLength + 1, false);
		return const_cast<char *>(c_str());
	}
	void ReleaseBuffer(int nNewLength = -1) {
		assert(nNewLength == -1);
		*this = CString(c_str());
	}
	size_t GetLength() const {
		return size();
	}
};

} // namespace MFC
} // namespace Bagel

#endif
