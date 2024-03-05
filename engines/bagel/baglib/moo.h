
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

#ifndef BAGEL_BAGLIB_MOO_H
#define BAGEL_BAGLIB_MOO_H

#include "bagel/baglib/storage_dev_bmp.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/pda.h"

namespace Bagel {

class CBagMoo : public CBagStorageDevBmp {
protected:
private:
	static CBagCharacterObject *m_pMovie;
	static PDAMODE m_eSavePDAMode;
	static PDAPOS m_eSavePDAPos;

public:
	CBagMoo() : CBagStorageDevBmp() {}
	virtual ~CBagMoo();
	static void initStatics();

	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT nMaskColor = -1);

	// Grab the button event of the bagbmobj and send them to the cbagsdev
	BOOL OnLButtonUp(UINT /*nFlags*/, CBofPoint /*xPoint*/, void * /*info*/) {
		return ERR_NONE;
	}
	virtual BOOL OnLButtonDown(UINT /*nFlags*/, CPoint /*xPoint*/, void * = nullptr) {
		return ERR_NONE;
	}

	// For public access to our movie object

	ERROR_CODE SetPDAMovie(CBofString &s);
	CBagCharacterObject *GetPDAMovie() {
		return m_pMovie;
	}

	VOID StopMovie(BOOL);
	BOOL MoviePlaying() {
		return m_pMovie != nullptr;
	}

	VOID SavePDAMode(PDAMODE pdaMode) {
		m_eSavePDAMode = pdaMode;
	}
	VOID SavePDAPosition(PDAPOS pdaPos) {
		m_eSavePDAPos = pdaPos;
	}
};

} // namespace Bagel

#endif
