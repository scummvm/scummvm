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

#include "common/file.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/bagel.h"

namespace Bagel {

CBagCharacterObject *CBagCharacterObject::m_pPDAWand;
BOOL CBagCharacterObject::m_bPDAAnimating;

void CBagCharacterObject::initialize() {
	m_pPDAWand = nullptr;
	m_bPDAAnimating = FALSE;
}

CBagCharacterObject::CBagCharacterObject() : CBagObject() {
	m_xObjType = CHAROBJ;
	m_pBmpBuf = nullptr;
	m_pBinBuf = nullptr;
	m_nCharTransColor = -1;
	SetOverCursor(0);   // Default is now the nullptr cursor
	SetTimeless(TRUE);
	m_nStartFrame = -1;
	m_nEndFrame = -1;
	m_nPlaybackSpeed = 1;
	m_nNumOfLoops = -1;
	m_bExitAtEnd = FALSE;
	m_nPrevFrame = -1;
	m_bFirstFrame = TRUE;
	m_bPanim = FALSE;
	m_bSaveState = FALSE;
}

CBagCharacterObject::~CBagCharacterObject() {
	Detach();
}

ERROR_CODE CBagCharacterObject::Attach() {
	CHAR szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString filename(szLocalBuff, 256);
	CBofPalette *pSmackPal = CBofApp::GetApp()->GetPalette();

	filename = GetFileName();

	// Open the smacker file
	Video::SmackerDecoder *decoder = new Video::SmackerDecoder();
	if (!decoder->loadFile(filename.GetBuffer())) {
		LogError(BuildString("CHAR SmackOpen failed: %s ", filename.GetBuffer()));
		return ERR_FOPEN;
	}

	_smacker = decoder;

	// Create an offscreen bitmap to decompress smacker into
	m_bFirstFrame = TRUE;

	m_pBmpBuf = new CBofBitmap(_smacker->getWidth(), _smacker->getHeight(), pSmackPal);

	if ((pSmackPal != nullptr) && (m_pBmpBuf != nullptr)) {
		m_pBmpBuf->FillRect(nullptr, pSmackPal->GetNearestIndex(RGB(255, 255, 255)));
	}

	// Create the text filename
	filename.MakeUpper();
	filename.ReplaceStr(".SMK", ".BIN");

	if (m_pBinBuf != nullptr) {
		BofFree(m_pBinBuf);
		m_pBinBuf = nullptr;
	}

	// Does file exist
	if (FileExists(filename.GetBuffer())) {
		CBofFile cInputFile(filename.GetBuffer());

		m_nBinBufLen = cInputFile.GetLength();

		if ((m_pBinBuf = (CHAR *)BofAlloc(m_nBinBufLen + 1)) != nullptr) {
			cInputFile.Read(m_pBinBuf, m_nBinBufLen);

		} else {
			ReportError(ERR_MEMORY);
		}
	}

	// Set the start and stop frames if still default.
	if (m_nPlaybackSpeed > 0) {
		// Forward playback
		if (m_nStartFrame == -1)
			m_nStartFrame = 0;
		if (m_nEndFrame == -1)
			m_nEndFrame = _smacker->getFrameCount() - 1;

	} else {
		// Reverse playback
		if (m_nEndFrame == -1)
			m_nEndFrame = 1;
		if (m_nStartFrame == -1)
			m_nStartFrame = _smacker->getFrameCount();

		_smacker->setReverse(true);
	}

	if (m_bSaveState) {
		// Get the current state for this object
		int nState = GetState();

		// If the state is not the default(0) then move to the correct frame
		if (nState != 0)
			_smacker->seekToFrame(nState + 1);
	}

	if (m_nNumOfLoops != 0) {
		_smacker->start();
	} else {
		_smacker->stop();
	}

	SetVisible(TRUE);
	UpdatePosition();
	RefreshCurrFrame();

	// Get chroma from main app now
	m_nCharTransColor = CBagel::GetBagApp()->GetChromaColor();

	return CBagObject::Attach();
}

ERROR_CODE CBagCharacterObject::Detach() {
	if (m_bSaveState) {
		// Save off the state/frame information as we detach
		// so that we can recreate the scene when we attach again
		if (_smacker != nullptr) {
			SetState(_smacker->getCurFrame());
		}
	} else {
		// Decrement current loop from happening again
		if (m_nNumOfLoops > 0)
			m_nNumOfLoops--;
	}

	if (_smacker != nullptr) {
		_smacker->close();
		delete _smacker;
		_smacker = nullptr;
	}

	if (m_pBmpBuf != nullptr) {
		delete m_pBmpBuf;
		m_pBmpBuf = nullptr;
	}

	if (m_pBinBuf != nullptr) {
		BofFree(m_pBinBuf);
		m_pBinBuf = nullptr;
	}

	if (this == m_pPDAWand) {
		m_pPDAWand = nullptr;
	}

	return CBagObject::Detach();
}

BOOL CBagCharacterObject::RefreshCurrFrame() {
	BOOL bNewFrame = TRUE;

	if (m_pBmpBuf != nullptr) {
		if (_smacker->needsUpdate()) {
			// Decode the next frame
			const Graphics::Surface *surf = _smacker->decodeNextFrame();
			if (surf) {
				Graphics::ManagedSurface &destSurf = *m_pBmpBuf;

				// Copy the decoded frame into the offscreen bitmap
				destSurf.setPalette(_smacker->getPalette(), 0, 256);
				destSurf.blitFrom(*surf);
			} else {
				bNewFrame = false;
			}

		} else {
			// Return false so we don't update our position
			bNewFrame = false;
		}
	}

	return bNewFrame;
}

CBofRect CBagCharacterObject::GetRect() {
	CBofPoint p = GetPosition();
	CBofSize s;

	if (m_pBmpBuf)
		s = m_pBmpBuf->GetSize();

	return CBofRect(p, s);
}

VOID CBagCharacterObject::UpdatePosition() {
	// We have an input file
	if (m_pBinBuf != nullptr && _smacker != nullptr) {
		INT xpos = -1;
		INT ypos = -1;

		// Seek to correct place in the file
		int frame = _smacker->getCurFrame();

		if (frame < 0)
			frame = 0;

		LONG lSeekPos = frame * 2 * sizeof(int32);

		// Read from our memory buffer rather than going to
		// disk for the position of the smack dudes.
		// Check that we are going to fit
		if (lSeekPos + (LONG)(2 * sizeof(int32)) <= m_nBinBufLen) {
			xpos = READ_LE_INT32(&m_pBinBuf[lSeekPos]);
			lSeekPos += sizeof(int32);

			ypos = READ_LE_INT32(&m_pBinBuf[lSeekPos]);
			lSeekPos += sizeof(int32);

			// a valid number was read
			if ((xpos > -1) && (ypos > -1)) {
				CBofPoint cNewLoc(xpos, ypos);
				SetPosition(cNewLoc);
			}
		}
	}
}

BOOL CBagCharacterObject::DoAdvance() {
	// Assume we're not advancing
	BOOL bDoAdvance = FALSE;
	BOOL bPDAWand = (this == m_pPDAWand);

	if (bPDAWand) {
		m_bPDAAnimating = FALSE;
	}

	// If we are done looping just return with FALSE
	if (!m_nNumOfLoops)
		return FALSE;

	// If we got a background bitmap
	if (m_pBmpBuf != nullptr) {
		// If This Panimation is modal, or Panimations are ON, then get next
		// frame.
		//
		if (IsModal() || !m_bPanim || CBagMasterWin::GetPanimations()) {
			// TODO: Not sure what this is for, it just gets stuck in an infinite loop
#if 0
			if (bPDAWand) {
				while (!_smacker->needsUpdate()) {
					g_system->delayMillis(10);
					if (g_engine->shouldQuit())
						return FALSE;
				}
			}
#endif

			if (_smacker->needsUpdate()) {
				bDoAdvance = TRUE;

				// Paint the current frame to the BMP
				if (RefreshCurrFrame()) {
					// Get the current frame in the correct place
					UpdatePosition();
				}

				// We've looped
				if ((INT)_smacker->getCurFrame() == m_nEndFrame) {
					if (m_nNumOfLoops > 0)
						m_nNumOfLoops--;  // decrement num of loops
				}

				if (m_nPlaybackSpeed > 0) {
					// Get next frame, will loop to beginning
					if ((INT)_smacker->getCurFrame() == m_nEndFrame) {
						_smacker->rewind();
						_smacker->start();
					}
				} else {
					if (((INT)_smacker->getCurFrame() == m_nEndFrame) ||
							(_smacker->getCurFrame() == 1)) {
						SetFrame(m_nStartFrame);

					} else {
						SetFrame(_smacker->getCurFrame() - 1);
					}
				}
			}
		} else {
			// Only play the first frame
			if (m_bFirstFrame) {
				m_bFirstFrame = FALSE;

				// Get the current frame in the correct place
				UpdatePosition();

				// Paint the current frame to the BMP
				RefreshCurrFrame();
			}
		}

	}

	// We got a Bitmap
	if (bPDAWand) {
		m_bPDAAnimating = bDoAdvance;
	}

	return bDoAdvance;
}

BOOL CBagCharacterObject::IsInside(const CBofPoint &xPoint) {
	if (GetRect().PtInRect(xPoint) && m_nCharTransColor >= 0) {
		if (m_pBmpBuf) {
			int x = xPoint.x - GetRect().left;
			int y = xPoint.y - GetRect().top;
			int c = m_pBmpBuf->ReadPixel(x, y);
			return (c != m_nCharTransColor);
		} else return TRUE;
	}

	return FALSE;
}

BOOL CBagCharacterObject::RunObject() {
	CBagObject::RunObject();
	return FALSE;
}

ERROR_CODE CBagCharacterObject::Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	DoAdvance();  // Advance to next frame

	if (m_pBmpBuf)
		return m_pBmpBuf->Paint(pWnd, pt.x, pt.y, pSrcRect, nMaskColor);
	else
		return ERR_NONE;
}

ERROR_CODE CBagCharacterObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/, INT /*nMaskColor*/) {
	// Get the original position for character
	CBofPoint OrigPos = GetPosition();

	BOOL bDoAdvance = DoAdvance();

	// If we have more frames advance this, else exit and detach if needed
	if (!bDoAdvance && m_bExitAtEnd) {
		// Run the ending objects
		Detach();
	}

	if (m_pBmpBuf) {
		// Get the new position for the character
		CBofPoint NewPos = GetPosition();
		// Get access to the current sDev

		// Paint in the new pos
		return m_pBmpBuf->Paint(pBmp, (pt.x + (NewPos.x - OrigPos.x)), (pt.y + (NewPos.y - OrigPos.y)), nullptr, m_nCharTransColor);
	}

	return ERR_NONE;
}

PARSE_CODES CBagCharacterObject::SetInfo(bof_ifstream &istr) {
	BOOL    nObjectUpdated = FALSE;

	while (!istr.eof()) {
		int nChanged = 0;

		char ch;
		switch (ch = (char)istr.peek()) {
		//  SAVESTATE - Maintain the state of the character
		//
		case 'K': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("KEEPSTATE")) {
				istr.EatWhite();

				m_bSaveState = TRUE;

				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  LOOP n - n number of times to loop (-1 infinate)
		//
		case 'L': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("LOOP")) {
				istr.EatWhite();
				GetIntFromStream(istr, m_nNumOfLoops);
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  SPEED n - n pace of playback (negative is backward), (0 to hold at current frame)
		//
		case 'S': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SPEED")) {
				istr.EatWhite();
				GetIntFromStream(istr, m_nPlaybackSpeed);
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  EXITATEND - detach at end of looping (call run after objects)
		//
		case 'E': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("NOEXITATEND")) {
				istr.EatWhite();

				m_bExitAtEnd = FALSE;

				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  PANIM - Specifies if this object should be affected by the user
		// option Panimations On/Off
		//
		case 'P': {
			CBofString sStr;

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("PANIM")) {
				istr.EatWhite();

				m_bPanim = TRUE;

				nObjectUpdated = TRUE;
				nChanged++;

			} else {
				PutbackStringOnStream(istr, sStr);
			}

			break;
		}

		//
		//  FRAME [start, end]- start and end frames of the move
		//
		case 'F': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FRAME")) {
				CBofRect r;
				istr.EatWhite();

				GetRectFromStream(istr, r);

				m_nStartFrame = r.left;
				m_nEndFrame = r.top;

				if (r.Width() && r.Height()) {
					// error of some type
				}

				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  no match return from funtion
		//
		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = TRUE;
			} else if (!nChanged) { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

VOID CBagCharacterObject::ArrangeFrames() {
	int start = GetStartFrame();
	int end = GetEndFrame();

	if (m_nPlaybackSpeed < 0) {

		m_nStartFrame = max(start, end);
		m_nEndFrame = min(start, end);

	} else {
		m_nStartFrame = min(start, end);
		m_nEndFrame = max(start, end);
	}
}


VOID CBagCharacterObject::SetNumOfLoops(INT n) {
	m_nNumOfLoops = n;
	if (_smacker != nullptr) {
		if (m_nNumOfLoops != 0) {
			_smacker->start();
		} else {
			_smacker->stop();
		}
	}

	// If this character is modal run until done looping
	if (IsModal() && IsAttached()) {
		CBagStorageDevWnd *pWin;

		if ((pWin = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) != nullptr) {
			pWin->RunModal(this);
		}
	}
}


VOID CBagCharacterObject::SetPlaybackSpeed(INT n) {
	if (m_nPlaybackSpeed != n) {
		if (n < 0) {
			m_nStartFrame++;
			m_nEndFrame++;
		} else {
			m_nStartFrame--;
			m_nEndFrame--;
		}

		m_nPlaybackSpeed = n;
		ArrangeFrames();
		SetCurrentFrame(GetStartFrame());
	}
}

VOID CBagCharacterObject::SetStartFrame(INT n) {
	ArrangeFrames();
	m_nStartFrame = n;
	ArrangeFrames();
}

VOID CBagCharacterObject::SetEndFrame(INT n) {
	ArrangeFrames();
	m_nEndFrame = n;
	ArrangeFrames();
}

VOID CBagCharacterObject::SetCurrentFrame(INT n) {
	// Make sure that it is within specified values?
	//
	// Due to some distinctly bogus code that manipulates the
	// start and end frame, the current frame passed in can be negative, which
	// will cause slacker-smacker to go toes up, handle that here.

	if (!_smacker) {
		warning("CBagCharacterObject::SetCurrentFrame(%d): no smacker", n);
		return;
	}

	n = (n <= 0 ? 1 : n);
	_smacker->seekToFrame(n);

	// Added UpdatePosition() because if any movies go backwards, and use
	// a .BIN file, then it would not have worked.
	UpdatePosition();

	RefreshCurrFrame();
}

VOID CBagCharacterObject::SetFrame(INT n) {
	// Make sure that it is within specified values?
	if ((_smacker != nullptr) && (n >= 0) && (n <= (INT)_smacker->getFrameCount())) {
		_smacker->seekToFrame(n);
	}
}

VOID CBagCharacterObject::SetProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("LOOP"))
		SetNumOfLoops(nVal);
	else if (!sProp.Find("SPEED"))
		SetPlaybackSpeed(nVal);
	else if (!sProp.Find("START_FRAME"))
		SetStartFrame(nVal);
	else if (!sProp.Find("END_FRAME"))
		SetEndFrame(nVal);
	else if (!sProp.Find("CURR_FRAME"))    // This one will not work currently
		SetCurrentFrame(nVal);
	else
		CBagObject::SetProperty(sProp, nVal);
}

INT CBagCharacterObject::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("LOOP"))
		return GetNumOfLoops();
	else if (!sProp.Find("SPEED"))
		return GetPlaybackSpeed();
	else if (!sProp.Find("START_FRAME"))
		return GetStartFrame();
	else if (!sProp.Find("END_FRAME"))
		return GetEndFrame();
	else if (!sProp.Find("CURR_FRAME"))    // This one will not work currently
		return GetCurrentFrame();
	else
		return CBagObject::GetProperty(sProp);
}

VOID CBagCharacterObject::SetPDAWand(CBagCharacterObject *pWand) {
	m_pPDAWand = pWand;
}

BOOL CBagCharacterObject::PDAWandAnimating() {
	if (m_pPDAWand == nullptr || !m_pPDAWand->IsAttached()) {
		CBagStorageDev *pPda = SDEVMNGR->GetStorageDevice("BPDA_WLD");
		if (pPda != nullptr) {
			CBagCharacterObject *pWand = (CBagCharacterObject *)pPda->GetObject("WANDANIM");
			if (pWand != nullptr) {
				CBagCharacterObject::SetPDAWand(pWand);
			}
		}
	}

	return m_bPDAAnimating;
}

} // namespace Bagel
