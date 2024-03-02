
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

#ifndef BAGEL_BAGLIB_AREA_OBJECT_H
#define BAGEL_BAGLIB_AREA_OBJECT_H

#include "bagel/baglib/object.h"

namespace Bagel {

/**
 * CBagAreaObject is an object that can be place within the slide window.
 */
class CBagAreaObject : public CBagObject {
private:
	CBofSize m_xSize;

public:
	CBagAreaObject();
	virtual ~CBagAreaObject();

	// Return TRUE if the Object had members that are properly initialized/de-initialized
	ERROR_CODE Attach();
	ERROR_CODE Detach();

	CBofRect GetRect();
	CBofSize GetSize() const { return m_xSize; }

	void SetSize(const CBofSize &xSize) { m_xSize = xSize; }
};

} // namespace Bagel

#endif
