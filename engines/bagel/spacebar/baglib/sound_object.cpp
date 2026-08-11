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
#include "bagel/boflib/event_loop.h"
#include "bagel/spacebar/baglib/sound_object.h"
#include "bagel/bagel.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {
namespace SpaceBar {

// There is only one allowed at a time
CBofSound *CBagSoundObject::_pMidiSound;

void CBagSoundObject::initialize() {
	_pMidiSound = nullptr;
}

CBagSoundObject::CBagSoundObject() {
	_xObjType = SOUND_OBJ;
	_pSound = nullptr;

	// Assume MIX if not specified
	_wFlags = SOUND_MIX;

	_nVol = VOLUME_INDEX_DEFAULT;
	CBagObject::setState(0);
	_bWait = false;

	_nLoops = 1;

	setVisible(false);
	CBagObject::setOverCursor(3);
}

CBagSoundObject::~CBagSoundObject() {
	CBagSoundObject::detach();
}

ErrorCode CBagSoundObject::attach(CBofWindow *pWnd) {
	newSound(pWnd);

	return CBagObject::attach();
}

void CBagSoundObject::newSound(CBofWindow *pWin) {
	killSound();

	_pSound = new CBofSound(pWin, getFileName(), _wFlags, _nLoops);
	_pSound->setVolume(_nVol);
	_pSound->setQSlot(getState());
}

void CBagSoundObject::killSound() {
	delete _pSound;
	_pSound = nullptr;
}

ErrorCode CBagSoundObject::detach() {
	killSound();
	return CBagObject::detach();
}

bool CBagSoundObject::runObject() {
	if (((_wFlags & SOUND_MIDI) && CBagMasterWin::getMidi()) || (((_wFlags & SOUND_WAVE) || (_wFlags & SOUND_MIX)) && CBagMasterWin::getDigitalAudio())) {

		if (_pSound && _pMidiSound != _pSound) {
			_pSound->setQSlot(getState());
			_pSound->play();

			// If waiting until this sound finishes
			if (_bWait) {
				// Show busy cursor
				CBagMasterWin::setActiveCursor(6);

				EventLoop limiter(EventLoop::FORCE_REPAINT);
				while (_pSound->isPlaying()) {
					CBofSound::audioTask();

					if (limiter.frame()) {
						_pSound->stop();
						break;
					}
				}
			}

			if (_wFlags & SOUND_MIDI)
				_pMidiSound = _pSound;
		} else if (!(_wFlags & SOUND_MIDI)) {
			/* if no sound */
			int nExt = getFileName().getLength() - 4; // ".EXT"

			if (nExt <= 0) {
				logError("Sound does not have a file name or proper extension.  Please write better scripts.");
				return false;
			}

			CBofString sBaseStr = getFileName().left(nExt) + ".TXT";

			Common::File f;
			if (fileExists(sBaseStr) && f.open(sBaseStr.getBuffer())) {
				Common::String line = f.readLine();

				bofMessageBox(line.c_str(), "Using .TXT for missing .WAV!");
				f.close();
				return true;
			}

			logError(buildString("Sound TEXT file could not be read: %s.  Why? because we like you ...", getFileName().getBuffer()));
			return false;
		}
	}

	return CBagObject::runObject();
}

ParseCodes CBagSoundObject::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;

	while (!istr.eof()) {
		istr.eatWhite(); // Eat any white space between script elements
		char ch = (char)istr.peek();
		switch (ch) {

		// VOLUME
		//
		case 'V': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("VOLUME")) {
				istr.eatWhite();
				int n;
				getIntFromStream(istr, n);
				setVolume(n);
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  AS [WAVE|MIDI]  - how to run the link
		//
		case 'A': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // performance improvement
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("AS")) {

				istr.eatWhite();
				getAlphaNumFromStream(istr, sStr);

				if (!sStr.find("WAVE")) {
					setWave();
					nObjectUpdated = true;

				} else if (!sStr.find("MIDI")) {
					setMidi();
					nObjectUpdated = true;

				} else if (!sStr.find("SYNC")) {
					setSync();
					nObjectUpdated = true;

				} else if (!sStr.find("ASYNC")) {
					setASync();
					nObjectUpdated = true;

					// Mix and Wait
					//
				} else if (!sStr.find("WAITMIX")) {

					setMix();
					_bWait = true;
					nObjectUpdated = true;

					// Queue and Wait
					//
				} else if (!sStr.find("WAITQUEUE")) {

					setQueue();
					_bWait = true;
					nObjectUpdated = true;

				} else if (!sStr.find("QUEUE")) {

					setQueue();
					nObjectUpdated = true;

				} else if (!sStr.find("MIX")) {
					setMix();
					nObjectUpdated = true;

				} else {
					putbackStringOnStream(istr, sStr);
					putbackStringOnStream(istr, "AS ");
				}

			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		// LOOP PROPERTY FOR SOUNDS
		case 'L': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // performance improvement

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("LOOP")) {
				istr.eatWhite();
				getIntFromStream(istr, _nLoops);
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		// Oversound attribute for sound object
		case 'S': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // performance improvement

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("SOUNDOVEROK")) {
				setSoundOver();
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		//  No match return from function
		//
		default: {
			ParseCodes parseCode = CBagObject::setInfo(istr);
			if (parseCode == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (parseCode == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
		}
		break;
		} // end switch

	} // end while

	return PARSING_DONE;
}

void CBagSoundObject::setQueue(bool b) {
	if (b) {
		_wFlags = SOUND_MIX | SOUND_QUEUE;

	} else {
		_wFlags &= ~SOUND_QUEUE;
	}
}

int CBagSoundObject::getVolume() {
	return _nVol;
}

void CBagSoundObject::setVolume(int nVol) {
	_nVol = (byte)nVol;
	if (isAttached()) {

		if (_pSound != nullptr) {
			_pSound->setVolume(_nVol);
		}
	}
}

bool CBagSoundObject::isPlaying() {
	bool bPlaying = false;
	if (_pSound != nullptr) {
		bPlaying = _pSound->playing();
	}

	return bPlaying;
}

bool CBagSoundObject::isQueued() {
	bool bQueued = false;
	if (_pSound != nullptr) {
		bQueued = _pSound->isQueued();
	}

	return bQueued;
}

void CBagSoundObject::setPlaying(bool bVal) {
	if (((_wFlags & SOUND_MIDI) && CBagMasterWin::getMidi()) || (((_wFlags & SOUND_WAVE) || (_wFlags & SOUND_MIX)) && CBagMasterWin::getDigitalAudio())) {

		if (bVal) {
			if (_pSound && _pMidiSound != _pSound) {

				_pSound->setQSlot(getState());
				_pSound->play();

				// If we are supposed to wait until this audio finishes
				if (_bWait) {
					// Show busy cursor
					CBagMasterWin::setActiveCursor(6);

					EventLoop limiter(EventLoop::FORCE_REPAINT);
					while (_pSound->isPlaying()) {
						CBofSound::audioTask();

						if (limiter.frame()) {
							_pSound->stop();
							break;
						}
					}
				}

				if (_wFlags & SOUND_MIDI)
					_pMidiSound = _pSound;
			}
		} else if (_pSound) {
			_pSound->stop();
			if (_wFlags & SOUND_MIDI)
				_pMidiSound = nullptr;
		}
	}
}

void CBagSoundObject::setNumOfLoops(int n) {
	_nLoops = n; // Only have ability to set at creation of BofSound
}

int CBagSoundObject::getProperty(const CBofString &sProp) {
	if (!sProp.find("VOLUME")) {
		return getVolume();

	}

	if (!sProp.find("QUEUED")) {
		return isQueued();

	}

	if (!sProp.find("PLAYING")) {
		return isPlaying();

	}

	if (!sProp.find("LOOP")) {
		return _nLoops;
	}

	return CBagObject::getProperty(sProp);
}

void CBagSoundObject::setProperty(const CBofString &sProp, int nVal) {
	if (!sProp.find("VOLUME")) {
		setVolume(nVal);

	} else if (!sProp.find("PLAYING")) {

		if (nVal == 1)
			setPlaying();
		else
			setPlaying(false);

	} else if (!sProp.find("LOOP")) {
		setNumOfLoops(nVal);
	} else {
		CBagObject::setProperty(sProp, nVal);
	}
}

void CBagSoundObject::setSync(bool b) {
	_wFlags = SOUND_WAVE;
	if (!b)
		_wFlags |= SOUND_ASYNCH;
}

} // namespace SpaceBar
} // namespace Bagel
