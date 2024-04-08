
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
	int m_nCharTransColor = 0;

	char *m_pBinBuf = nullptr;
	int32 m_nBinBufLen = 0;

	int m_nPlaybackSpeed = 0;
	int m_nNumOfLoops = 0;
	int m_nStartFrame = 0;
	int m_nEndFrame = 0;

	bool m_bExitAtEnd : 1;
	bool m_bFirstFrame : 1;

	bool m_bSaveState : 1; // flag to save the state/frame of the character
	bool m_bPanim : 1;     // If affected by Panimations On/Off setting

	int m_nPrevFrame = 0;

	void SetFrame(int n);

	// Keep track of the PDA wand and the number of frames it has
	static CBagCharacterObject *m_pPDAWand;
	static bool m_bPDAAnimating;

public:
	CBagCharacterObject();
	virtual ~CBagCharacterObject();
	static void initialize();

	// Return TRUE if the Object had members that are properly initialized/de-initialized
	ERROR_CODE Attach();
	ERROR_CODE Detach();

	CBofRect GetRect();

	virtual ERROR_CODE Update(CBofWindow * /*pWnd*/, CBofPoint /*pt*/, CBofRect * /*pSrcRect*/ = nullptr, int /*nMaskColor*/ = -1);
	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int /*nMaskColor*/ = -1);

	bool DoAdvance();
	void UpdatePosition();
	bool RefreshCurrFrame();

	virtual bool RunObject();
	virtual bool IsInside(const CBofPoint &xPoint);

	void ArrangeFrames();

	int GetNumOfLoops() const {
		return m_nNumOfLoops;
	}
	int GetPlaybackSpeed() const {
		return m_nPlaybackSpeed;
	}
	int GetStartFrame() const {
		return m_nStartFrame;
	}
	int GetEndFrame() const {
		return m_nEndFrame;
	}
	int GetCurrentFrame() const {
		return (_smacker != nullptr) ? _smacker->getCurFrame() : -1;
	}

	bool IsModalDone() {
		return !m_nNumOfLoops;
	}

	bool IsPanim() {
		return m_bPanim;
	}
	void SetPanim(bool b = TRUE) {
		m_bPanim = b;
	}

	void SetNumOfLoops(int n);
	void SetPlaybackSpeed(int n);
	void SetStartFrame(int n);
	void SetEndFrame(int n);
	void SetCurrentFrame(int n);

	PARSE_CODES SetInfo(bof_ifstream &istr);

	void SetProperty(const CBofString &sProp, int nVal);
	int GetProperty(const CBofString &sProp);

	// Remember the pda wand, we'll need to know about it and
	// it's total number of frames.
	static void SetPDAWand(CBagCharacterObject *pWand);
	static bool PDAWandAnimating();

	bool IsStationary() const {
		return m_pBinBuf != nullptr;
	}
};

} // namespace Bagel

#endif
