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


static const int CURSOR_W = 12;
static const int CURSOR_H = 20;
static const byte ARROW_CURSOR[CURSOR_W * CURSOR_H] = {
	1,1,0,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,1,1,1,1,1,
	1,2,2,2,1,2,2,1,0,0,0,0,
	1,2,2,1,1,2,2,1,0,0,0,0,
	1,2,1,0,0,1,2,2,1,0,0,0,
	1,1,0,0,0,1,2,2,1,0,0,0,
	1,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,0,1,1,0,0,
};
static const byte CURSOR_PALETTE[] = { 0x80, 0x80, 0x80, 0, 0, 0, 0xff, 0xff, 0xff };

CBagCursor *CBagCursor::m_pCurrentCursor;
CSystemCursor *CBagCursor::_systemCursor;

void CBagCursor::initialize() {
	m_pCurrentCursor = nullptr;
	_systemCursor = new CSystemCursor();
}

void CBagCursor::shutdown() {
	delete _systemCursor;
}

void CBagCursor::ShowSystemCursor() {
	_systemCursor->SetCurrent();
}


CBagCursor::CBagCursor() {
	m_pBmp = nullptr;
	m_nX = 0;
	m_nY = 0;
	m_bSharedPal = false;

	// Default is that this is not a wielded cursor
	SetWieldCursor(false);
}

CBagCursor::CBagCursor(CBofBitmap *pBmp) {
	m_pBmp = nullptr;
	m_nX = 0;
	m_nY = 0;
	m_bSharedPal = false;

	Load(pBmp);

	// Default is that this is not a wielded cursor
	SetWieldCursor(false);
}

CBagCursor::CBagCursor(const char *pszFileName, bool bSharedPal) {
	Assert(pszFileName != nullptr);

	m_pBmp = nullptr;
	m_nX = 0;
	m_nY = 0;
	m_bSharedPal = bSharedPal;
	strncpy(m_szFileName, pszFileName, MAX_FNAME - 1);

	// Default is that this is not a wielded cursor
	SetWieldCursor(false);
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

ERROR_CODE CBagCursor::Load(const char *pszFileName, CBofPalette *pPal) {
	Assert(IsValidObject(this));
	Assert(pszFileName != nullptr);

	UnLoad();

	m_pBmp = LoadBitmap(pszFileName, pPal, m_bSharedPal);

	return m_errCode;
}

void CBagCursor::UnLoad() {
	Assert(IsValidObject(this));

	if (m_pBmp != nullptr) {
		delete m_pBmp;
		m_pBmp = nullptr;
	}
}

void CBagCursor::SetCurrent() {
	m_pCurrentCursor = this;
	CursorMan.replaceCursorPalette(m_pBmp->GetPalette()->GetData(), 0, PALETTE_COUNT);
	CursorMan.replaceCursor(m_pBmp->getSurface(), m_nX, m_nY, 1);
}


void CSystemCursor::SetCurrent() {
	m_pCurrentCursor = this;

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	CursorMan.replaceCursorPalette(CURSOR_PALETTE, 0, ARRAYSIZE(CURSOR_PALETTE) / 3);
	CursorMan.replaceCursor(ARROW_CURSOR, CURSOR_W, CURSOR_H, 0, 0, 0, true, &format);
}


} // namespace Bagel
