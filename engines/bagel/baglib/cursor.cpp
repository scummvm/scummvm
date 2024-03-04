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

#include "bagel/baglib/cursor.h"

namespace Bagel {

CBagCursor *CBagCursor::m_pCurrentCursor = nullptr;

CBagCursor::CBagCursor() {
	m_pBmp = nullptr;
	m_nX = 0;
	m_nY = 0;
	m_bSharedPal = FALSE;

	// Default is that this is not a wielded cursor
	SetWieldCursor(FALSE);
}

CBagCursor::CBagCursor(CBofBitmap *pBmp) {
	m_pBmp = nullptr;
	m_nX = 0;
	m_nY = 0;
	m_bSharedPal = FALSE;

	Load(pBmp);

	// Default is that this is not a wielded cursor
	SetWieldCursor(FALSE);
}

CBagCursor::CBagCursor(const CHAR *pszFileName, BOOL bSharedPal) {
	Assert(pszFileName != nullptr);

	m_pBmp = nullptr;
	m_nX = 0;
	m_nY = 0;
	m_bSharedPal = bSharedPal;
	strncpy(m_szFileName, pszFileName, MAX_FNAME - 1);

	// Default is that this is not a wielded cursor
	SetWieldCursor(FALSE);
}

CBagCursor::~CBagCursor() {
	Assert(IsValidObject(this));

	UnLoad();
}

ERROR_CODE CBagCursor::Load(CBofBitmap *pBmp) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	UnLoad();

	m_pBmp = pBmp;

	return m_errCode;
}

ERROR_CODE CBagCursor::Load(const CHAR *pszFileName, CBofPalette *pPal) {
	Assert(IsValidObject(this));
	Assert(pszFileName != nullptr);

	UnLoad();

	m_pBmp = LoadBitmap(pszFileName, pPal, m_bSharedPal);

	return m_errCode;
}

VOID CBagCursor::UnLoad(VOID) {
	Assert(IsValidObject(this));

	if (m_pBmp != nullptr) {
		delete m_pBmp;
		m_pBmp = nullptr;
	}
}

} // namespace Bagel
