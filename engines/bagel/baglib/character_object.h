
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

#ifndef BAGEL_BAGLIB_BAG_CHARACTER_OBJECT_H
#define BAGEL_BAGLIB_BAG_CHARACTER_OBJECT_H

#include "video/smk_decoder.h"
#include "bagel/baglib/object.h"

namespace Bagel {

// By setting BININMEMORY to TRUE, we preload all the bin files needed
// for a specific world thus freeing the main loop of having to
#define BININMEMORY TRUE

class CBagCharacterObject : public CBagObject {
protected:
	Video::SmackerDecoder *_smacker = nullptr;
	CBofBitmap *m_pBmpBuf = nullptr;
	INT m_nCharTransColor = 0;

	CHAR *m_pBinBuf = nullptr;
	LONG m_nBinBufLen = 0;

	INT m_nPlaybackSpeed = 0;
	INT m_nNumOfLoops = 0;
	INT m_nStartFrame = 0;
	INT m_nEndFrame = 0;

	BOOL m_bExitAtEnd : 1;
	BOOL m_bFirstFrame : 1;

	BOOL m_bSaveState : 1; // flag to save the state/frame of the character
	BOOL m_bPanim : 1;     // If affected by Panimations On/Off setting

	INT m_nPrevFrame = 0;

	VOID SetFrame(INT n);

	// Keep track of the PDA wand and the number of frames it has
	static CBagCharacterObject *m_pPDAWand;
	static BOOL m_bPDAAnimating;

public:
	CBagCharacterObject();
	virtual ~CBagCharacterObject();
	static void initStatics();

	// Return TRUE if the Object had members that are properly initialized/de-initialized
	ERROR_CODE Attach();
	ERROR_CODE Detach();

	CBofRect GetRect();

	virtual ERROR_CODE Update(CBofWindow * /*pWnd*/, CBofPoint /*pt*/, CBofRect * /*pSrcRect*/ = nullptr, INT /*nMaskColor*/ = -1);
	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT /*nMaskColor*/ = -1);

	BOOL DoAdvance();
	VOID UpdatePosition();
	BOOL RefreshCurrFrame();

	virtual BOOL RunObject();
	virtual BOOL IsInside(const CBofPoint &xPoint);

	VOID ArrangeFrames();

	INT GetNumOfLoops() const {
		return m_nNumOfLoops;
	}
	INT GetPlaybackSpeed() const {
		return m_nPlaybackSpeed;
	}
	INT GetStartFrame() const {
		return m_nStartFrame;
	}
	INT GetEndFrame() const {
		return m_nEndFrame;
	}
	INT GetCurrentFrame() const {
		return (_smacker != nullptr) ? _smacker->getCurFrame() : -1;
	}

	BOOL IsModalDone() {
		return !m_nNumOfLoops;
	}

	BOOL IsPanim() {
		return m_bPanim;
	}
	VOID SetPanim(BOOL b = TRUE) {
		m_bPanim = b;
	}

	VOID SetNumOfLoops(INT n);
	VOID SetPlaybackSpeed(INT n);
	VOID SetStartFrame(INT n);
	VOID SetEndFrame(INT n);
	VOID SetCurrentFrame(INT n);

	PARSE_CODES SetInfo(bof_ifstream &istr);

	VOID SetProperty(const CBofString &sProp, INT nVal);
	INT GetProperty(const CBofString &sProp);

	// Remember the pda wand, we'll need to know about it and
	// it's total number of frames.
	static VOID SetPDAWand(CBagCharacterObject *pWand);
	static BOOL PDAWandAnimating();

	BOOL IsStationary() const {
		return m_pBinBuf != nullptr;
	}
};

} // namespace Bagel

#endif
