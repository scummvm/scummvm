
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

#include "bagel/spacebar/baglib/storage_dev_bmp.h"
#include "bagel/spacebar/baglib/character_object.h"
#include "bagel/spacebar/baglib/pda.h"

namespace Bagel {
namespace SpaceBar {

class CBagMoo : public CBagStorageDevBmp {
private:
	static CBagCharacterObject *_pMovie;
	static PdaMode _eSavePDAMode;
	static PdaPos _eSavePDAPos;

public:
	CBagMoo() : CBagStorageDevBmp() {
	}
	virtual ~CBagMoo();
	static void initialize();

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int nMaskColor = -1) override;

	// Grab the button event of the bagbmobj and send them to the cbagsdev
	void onLButtonUp(uint32 /*nFlags*/, CBofPoint */*xPoint*/, void */*info*/) override {
	}
	void onLButtonDown(uint32 /*nFlags*/, CBofPoint */*xPoint*/, void * = nullptr) override {
	}

	// For public access to our movie object

	ErrorCode setPDAMovie(CBofString &s);
	CBagCharacterObject *getPDAMovie() {
		return _pMovie;
	}

	void stopMovie(bool);
	bool moviePlaying() {
		return _pMovie != nullptr;
	}

	void savePDAMode(PdaMode pdaMode) {
		_eSavePDAMode = pdaMode;
	}
	void savePDAPosition(PdaPos pdaPos) {
		_eSavePDAPos = pdaPos;
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
