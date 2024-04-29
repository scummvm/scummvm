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
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {

CBagCharacterObject *CBagCharacterObject::_pdaWand;
bool CBagCharacterObject::_pdaAnimating;

void CBagCharacterObject::initialize() {
	_pdaWand = nullptr;
	_pdaAnimating = false;
}

CBagCharacterObject::CBagCharacterObject() : CBagObject() {
	m_xObjType = CHAROBJ;
	_bmpBuf = nullptr;
	_binBuf = nullptr;
	_charTransColor = -1;
	// Default is now the nullptr cursor
	CBagObject::SetOverCursor(0);
	SetTimeless(true);
	_startFrame = -1;
	_endFrame = -1;
	_playbackSpeed = 1;
	_numOfLoops = -1;
	_exitAtEnd = false;
	_prevFrame = -1;
	_firstFrame = true;
	_pAnim = false;
	_saveState = false;
}

CBagCharacterObject::~CBagCharacterObject() {
	CBagCharacterObject::detach();
}

ErrorCode CBagCharacterObject::attach() {
	char localBuff[256];
	localBuff[0] = '\0';
	CBofString filename(localBuff, 256);
	CBofPalette *smackPal = CBofApp::GetApp()->GetPalette();

	filename = GetFileName();

	// Open the smacker file
	Video::SmackerDecoder *decoder = new Video::SmackerDecoder();
	decoder->setSoundType(Audio::Mixer::kSFXSoundType);
	if (!decoder->loadFile(filename.GetBuffer())) {
		LogError(BuildString("CHAR SmackOpen failed: %s ", filename.GetBuffer()));
		return ERR_FOPEN;
	}

	_smacker = decoder;

	// Create an offscreen bitmap to decompress smacker into
	_firstFrame = true;

	_bmpBuf = new CBofBitmap(_smacker->getWidth(), _smacker->getHeight(), smackPal);

	if ((smackPal != nullptr) && (_bmpBuf != nullptr)) {
		_bmpBuf->fillRect(nullptr, smackPal->GetNearestIndex(RGB(255, 255, 255)));
	}

	// Create the text filename
	filename.MakeUpper();
	filename.ReplaceStr(".SMK", ".BIN");

	if (_binBuf != nullptr) {
		BofFree(_binBuf);
		_binBuf = nullptr;
	}

	// Does file exist
	if (FileExists(filename.GetBuffer())) {
		CBofFile cInputFile(filename.GetBuffer());

		_binBufLen = cInputFile.GetLength();

		if ((_binBuf = (char *)BofAlloc(_binBufLen + 1)) != nullptr) {
			cInputFile.Read(_binBuf, _binBufLen);

		} else {
			ReportError(ERR_MEMORY);
		}
	}

	// Set the start and stop frames if still default.
	if (_playbackSpeed > 0) {
		// Forward playback
		if (_startFrame == -1)
			_startFrame = 0;
		if (_endFrame == -1)
			_endFrame = _smacker->getFrameCount() - 1;

	} else {
		// Reverse playback
		if (_endFrame == -1)
			_endFrame = 1;
		if (_startFrame == -1)
			_startFrame = _smacker->getFrameCount();

		//_smacker->setReverse(true); // TODO: Not supported by SMK
	}

	if (_saveState) {
		// Get the current state for this object
		int nState = getState();

		// If the state is not the default(0) then move to the correct frame
		if (nState != 0)
			setFrame(nState + 1);
	}

	if (_numOfLoops != 0) {
		_smacker->start();
	} else {
		_smacker->stop();
	}

	SetVisible(true);
	updatePosition();
	refreshCurrentFrame();

	// Get chroma from main app now
	_charTransColor = CBagel::getBagApp()->getChromaColor();

	return CBagObject::attach();
}

ErrorCode CBagCharacterObject::detach() {
	if (_saveState) {
		// Save off the state/frame information as we detach
		// so that we can recreate the scene when we attach again
		if (_smacker != nullptr) {
			setState(_smacker->getCurFrame());
		}
	} else {
		// Decrement current loop from happening again
		if (_numOfLoops > 0)
			_numOfLoops--;
	}

	if (_smacker != nullptr) {
		_smacker->close();
		delete _smacker;
		_smacker = nullptr;
	}

	if (_bmpBuf != nullptr) {
		delete _bmpBuf;
		_bmpBuf = nullptr;
	}

	if (_binBuf != nullptr) {
		BofFree(_binBuf);
		_binBuf = nullptr;
	}

	if (this == _pdaWand) {
		_pdaWand = nullptr;
	}

	return CBagObject::detach();
}

bool CBagCharacterObject::refreshCurrentFrame() {
	bool newFrameFl = true;

	if (_bmpBuf != nullptr) {
		if (_smacker != nullptr) {
			// Decode the next frame
			const Graphics::Surface *surf = _smacker->decodeNextFrame();
			if (surf) {
				Graphics::ManagedSurface &destSurf = *_bmpBuf;

				// Copy the decoded frame into the offscreen bitmap
				destSurf.setPalette(_smacker->getPalette(), 0, 256);
				destSurf.blitFrom(*surf);
			} else {
				newFrameFl = false;
			}

		} else {
			// Return false so we don't update our position
			newFrameFl = false;
		}
	}

	return newFrameFl;
}

CBofRect CBagCharacterObject::getRect() {
	CBofPoint pos = getPosition();
	CBofSize size;

	if (_bmpBuf)
		size = _bmpBuf->GetSize();

	return CBofRect(pos, size);
}

void CBagCharacterObject::updatePosition() {
	// We have an input file
	if (_binBuf != nullptr && _smacker != nullptr) {
		// Seek to correct place in the file
		int frame = _smacker->getCurFrame();

		if (frame < 0)
			frame = 0;

		int32 lSeekPos = frame * 2 * sizeof(int32);

		// Read from our memory buffer rather than going to
		// disk for the position of the smack dudes.
		// Check that we are going to fit
		if (lSeekPos + (int32)(2 * sizeof(int32)) <= _binBufLen) {
			int xPos = READ_LE_INT32(&_binBuf[lSeekPos]);
			lSeekPos += sizeof(int32);

			int yPos = READ_LE_INT32(&_binBuf[lSeekPos]);
			lSeekPos += sizeof(int32);

			// A valid number was read
			if ((xPos > -1) && (yPos > -1)) {
				CBofPoint newPos(xPos, yPos);
				setPosition(newPos);
			}
		}
	}
}

bool CBagCharacterObject::doAdvance() {
	// Assume we're not advancing
	bool doAdvanceFl = false;
	bool pdaWandFl = (this == _pdaWand);

	if (pdaWandFl) {
		_pdaAnimating = false;
	}

	// If we are done looping just return with false
	if (!_numOfLoops)
		return false;

	// If we got a background bitmap
	if (_bmpBuf != nullptr) {
		// If This Panimation is modal, or Panimations are ON, then get next frame.
		if (IsModal() || !_pAnim || CBagMasterWin::GetPanimations()) {
			if (_smacker->needsUpdate()) {
				doAdvanceFl = true;

				// Paint the current frame to the BMP
				if (refreshCurrentFrame()) {
					// Get the current frame in the correct place
					updatePosition();
				}

				if (_playbackSpeed > 0) {
					// We've looped
					if (_smacker->getCurFrame() == _endFrame) {
						if (_numOfLoops > 0)
							_numOfLoops--; // decrement num of loops

						// Get next frame, will loop to beginning
						_smacker->rewind();
						_smacker->start();
					}
				} else if ((_smacker->getCurFrame() == _endFrame) || (_smacker->getCurFrame() == 1)) {
					if (_numOfLoops > 0)
						_numOfLoops--; // decrement num of loops

					// Get next frame, will loop to beginning
					setFrame(_startFrame);
				} else {
					setFrame(_smacker->getCurFrame() - 2); // HACK: Reverse playback
				}
			}
		} else if (_firstFrame) {
			// Only play the first frame
			_firstFrame = false;

			// Get the current frame in the correct place
			updatePosition();

			// Paint the current frame to the BMP
			refreshCurrentFrame();
		}

	}

	// We got a Bitmap
	if (pdaWandFl) {
		_pdaAnimating = doAdvanceFl;
	}

	return doAdvanceFl;
}

bool CBagCharacterObject::isInside(const CBofPoint &point) {
	if (getRect().PtInRect(point) && _charTransColor >= 0) {
		if (_bmpBuf) {
			int x = point.x - getRect().left;
			int y = point.y - getRect().top;
			int color = _bmpBuf->ReadPixel(x, y);
			return (color != _charTransColor);
		}

		return true;
	}

	return false;
}

bool CBagCharacterObject::runObject() {
	CBagObject::runObject();
	return false;
}

ErrorCode CBagCharacterObject::update(CBofBitmap *bmp, CBofPoint pt, CBofRect * /*srcRect, unused*/, int /*maskColor, unused*/) {
	// Get the original position for character
	CBofPoint originalPos = getPosition();

	bool doAdvanceFl = doAdvance();

	// If we have more frames advance this, else exit and detach if needed
	if (!doAdvanceFl && _exitAtEnd) {
		// Run the ending objects
		detach();
	}

	if (_bmpBuf) {
		// Get the new position for the character
		CBofPoint newPos = getPosition();
		// Get access to the current sDev

		// Paint in the new pos
		return _bmpBuf->paint(bmp, (pt.x + (newPos.x - originalPos.x)), (pt.y + (newPos.y - originalPos.y)), nullptr, _charTransColor);
	}

	return ERR_NONE;
}

PARSE_CODES CBagCharacterObject::setInfo(bof_ifstream &istr) {
	bool objectUpdatedFl = false;

	while (!istr.eof()) {
		char ch = (char)istr.peek();
		switch (ch) {
		//  SAVESTATE - Maintain the state of the character
		case 'K': {
			char localStr[256];
			localStr[0] = 0;
			CBofString str(localStr, 256);

			GetAlphaNumFromStream(istr, str);

			if (!str.Find("KEEPSTATE")) {
				istr.EatWhite();

				_saveState = true;

				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, str);
			}
			break;
		}

		//  LOOP n - n number of times to loop (-1 infinate)
		case 'L': {
			char localStr[256];
			localStr[0] = 0;
			CBofString str(localStr, 256);

			GetAlphaNumFromStream(istr, str);

			if (!str.Find("LOOP")) {
				istr.EatWhite();
				GetIntFromStream(istr, _numOfLoops);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, str);
			}
			break;
		}

		//  SPEED n - n pace of playback (negative is backward), (0 to hold at current frame)
		case 'S': {
			char localStr[256];
			localStr[0] = 0;
			CBofString str(localStr, 256);

			GetAlphaNumFromStream(istr, str);

			if (!str.Find("SPEED")) {
				istr.EatWhite();
				GetIntFromStream(istr, _playbackSpeed);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, str);
			}
			break;
		}

		//  EXITATEND - detach at end of looping (call run after objects)
		case 'E': {
			char localStr[256];
			localStr[0] = 0;
			CBofString str(localStr, 256);

			GetAlphaNumFromStream(istr, str);

			if (!str.Find("EXITATEND")) {
				istr.EatWhite();

				_exitAtEnd = true;
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, str);
			}
			break;
		}

		//  PANIM - Specifies if this object should be affected by the user
		// option Panimations On/Off
		case 'P': {
			CBofString str;

			GetAlphaNumFromStream(istr, str);

			if (!str.Find("PANIM")) {
				istr.EatWhite();

				_pAnim = true;
				objectUpdatedFl = true;

			} else {
				PutbackStringOnStream(istr, str);
			}

			break;
		}

		//  FRAME [start, end]- start and end frames of the move
		case 'F': {
			char localStr[256];
			localStr[0] = 0;
			CBofString str(localStr, 256);

			GetAlphaNumFromStream(istr, str);

			if (!str.Find("FRAME")) {
				CBofRect r;
				istr.EatWhite();

				getRectFromStream(istr, r);

				_startFrame = r.left;
				_endFrame = r.top;

				if (r.width() && r.height()) {
					// error of some type
				}

				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, str);
			}
			break;
		}

		// No match return from function
		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				objectUpdatedFl = true;
			} else { // rc==UNKNOWN_TOKEN
				if (objectUpdatedFl)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

void CBagCharacterObject::arrangeFrames() {
	int start = getStartFrame();
	int end = getEndFrame();

	if (_playbackSpeed < 0) {
		_startFrame = MAX(start, end);
		_endFrame = MIN(start, end);
	} else {
		_startFrame = MIN(start, end);
		_endFrame = MAX(start, end);
	}
}


void CBagCharacterObject::setNumOfLoops(int n) {
	_numOfLoops = n;
	if (_smacker != nullptr) {
		if (_numOfLoops != 0) {
			_smacker->start();
		} else {
			_smacker->stop();
		}
	}

	// If this character is modal run until done looping
	if (IsModal() && isAttached()) {
		CBagStorageDevWnd *win = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();

		if (win != nullptr) {
			win->RunModal(this);
		}
	}
}

void CBagCharacterObject::setPlaybackSpeed(int n) {
	if (_playbackSpeed != n) {
		if (n < 0) {
			_startFrame++;
			_endFrame++;
		} else {
			_startFrame--;
			_endFrame--;
		}

		_playbackSpeed = n;
		arrangeFrames();
		setCurrentFrame(getStartFrame());
	}
}

void CBagCharacterObject::setStartFrame(int n) {
	arrangeFrames();
	_startFrame = n;
	arrangeFrames();
}

void CBagCharacterObject::setEndFrame(int n) {
	arrangeFrames();
	_endFrame = n;
	arrangeFrames();
}

void CBagCharacterObject::setCurrentFrame(int n) {
	// Make sure that it is within specified values?
	//
	// Due to some distinctly bogus code that manipulates the
	// start and end frame, the current frame passed in can be negative, which
	// will cause slacker-smacker to go toes up, handle that here.

	setFrame(n);

	// Added updatePosition() because if any movies go backwards, and use
	// a .BIN file, then it would not have worked.
	updatePosition();

	refreshCurrentFrame();
}

void CBagCharacterObject::setFrame(int n) {
	// Make sure that it is within specified values?
	if (_smacker != nullptr) {
		if (n == (int)_smacker->getFrameCount()) {
			n -= 3; // HACK: Reverse rewind
		}
		n = CLIP<int>(n, 0, _smacker->getFrameCount() - 1);
		_smacker->forceSeekToFrame(n);
	}
}

void CBagCharacterObject::setProperty(const CBofString &prop, int val) {
	if (!prop.Find("LOOP"))
		setNumOfLoops(val);
	else if (!prop.Find("SPEED"))
		setPlaybackSpeed(val);
	else if (!prop.Find("START_FRAME"))
		setStartFrame(val);
	else if (!prop.Find("END_FRAME"))
		setEndFrame(val);
	else if (!prop.Find("CURR_FRAME"))    // This one will not work currently
		setCurrentFrame(val);
	else
		CBagObject::setProperty(prop, val);
}

int CBagCharacterObject::getProperty(const CBofString &prop) {
	if (!prop.Find("LOOP"))
		return getNumberOfLoops();

	if (!prop.Find("SPEED"))
		return getPlaybackSpeed();

	if (!prop.Find("START_FRAME"))
		return getStartFrame();

	if (!prop.Find("END_FRAME"))
		return getEndFrame();

	if (!prop.Find("CURR_FRAME"))    // This one will not work currently
		return getCurrentFrame();

	return CBagObject::getProperty(prop);
}

void CBagCharacterObject::setPdaWand(CBagCharacterObject *pdaWand) {
	_pdaWand = pdaWand;
}

bool CBagCharacterObject::pdaWandAnimating() {
	if (_pdaWand == nullptr || !_pdaWand->isAttached()) {
		CBagStorageDev *pda = SDEVMNGR->GetStorageDevice("BPDA_WLD");
		if (pda != nullptr) {
			CBagCharacterObject *wand = (CBagCharacterObject *)pda->GetObject("WANDANIM");
			if (wand != nullptr) {
				CBagCharacterObject::setPdaWand(wand);
			}
		}
	}

	return _pdaAnimating;
}

} // namespace Bagel
