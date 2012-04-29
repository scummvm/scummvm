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
 *
 */

#include "tony/mpal/mpalutils.h"
#include "tony/tony.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       RMRes methods
\****************************************************************************/

/**
 * Constructor
 * @param resId					MPAL resource to open
 */
RMRes::RMRes(uint32 resID) {
	m_h = _vm->_resUpdate.QueryResource(resID);
	if (m_h == NULL)
		m_h = mpalQueryResource(resID);
	if (m_h != NULL)
		m_buf = (byte *)GlobalLock(m_h);
}

/**
 * Destructor
 */
RMRes::~RMRes() {
	if (m_h != NULL) {
		GlobalUnlock(m_h);
		GlobalFree(m_h);
	}
}

/**
 * Returns a pointer to the resource
 */
const byte *RMRes::DataPointer() {
	return m_buf;
}

/**
 * Returns a pointer to the resource
 */
RMRes::operator const byte *() {
	return DataPointer();
}

/**
 * Returns the size of the resource
 */
unsigned int RMRes::Size() {
	return GlobalSize(m_h);
}

/****************************************************************************\
*       RMResRaw methods
\****************************************************************************/

RMResRaw::RMResRaw(uint32 resID) : RMRes(resID) {
}

RMResRaw::~RMResRaw() {
}

const byte *RMResRaw::DataPointer() {
	return m_buf + 8;
}

RMResRaw::operator const byte *() {
	return DataPointer();
}

int RMResRaw::Width() {
	return READ_LE_UINT16(m_buf + 4);
}

int RMResRaw::Height() {
	return READ_LE_UINT16(m_buf + 6);
}

} // end of namespace MPAL

} // end of namespace Tony
