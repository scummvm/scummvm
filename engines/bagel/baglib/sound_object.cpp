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
#include "bagel/baglib/sound_object.h"
#include "bagel/bagel.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {

// There is only one allowed at a time
CBofSound *CBagSoundObject::m_pMidiSound;

void CBagSoundObject::initialize() {
	m_pMidiSound = nullptr;
}

CBagSoundObject::CBagSoundObject() {
	m_xObjType = SOUNDOBJ;
	m_pSound = nullptr;

	// Assume MIX if not specified
	m_wFlags = SOUND_MIX; //(SOUND_WAVE | SOUND_ASYNCH);

	m_nVol = VOLUME_INDEX_DEFAULT;
	setState(0);
	m_bWait = false;

	m_nLoops = 1;

	SetVisible(false);
	SetOverCursor(3);
}

CBagSoundObject::~CBagSoundObject() {
	detach();
}

ErrorCode CBagSoundObject::attach(CBofWindow *pWnd) {
	NewSound(pWnd);

	return CBagObject::attach();
}

ErrorCode CBagSoundObject::NewSound(CBofWindow *pWin) {
	// assume no error
	ErrorCode errCode = ERR_NONE;

	KillSound();

	if ((m_pSound = new CBofSound(pWin, getFileName(), m_wFlags, m_nLoops)) != nullptr) {
		m_pSound->setVolume(m_nVol);
		m_pSound->setQSlot(getState());

	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}

void CBagSoundObject::KillSound() {
	if (m_pSound != nullptr) {
		delete m_pSound;
		m_pSound = nullptr;
	}
}

ErrorCode CBagSoundObject::detach() {
	KillSound();
	return CBagObject::detach();
}

bool CBagSoundObject::runObject() {
	if (((m_wFlags & SOUND_MIDI) && CBagMasterWin::getMidi()) || (((m_wFlags & SOUND_WAVE) || (m_wFlags & SOUND_MIX)) && CBagMasterWin::getDigitalAudio())) {

		if (m_pSound && m_pMidiSound != m_pSound) {

			m_pSound->setQSlot(getState());
			m_pSound->play();

			// If waiting until this sound finishes
			if (m_bWait) {
				// Show busy cursor
				CBagMasterWin::setActiveCursor(6);

				EventLoop limiter(EventLoop::FORCE_REPAINT);
				while (m_pSound->isPlaying()) {
					CBofSound::audioTask();

					if (limiter.frame()) {
						m_pSound->stop();
						break;
					}
				}
			}

			if (m_wFlags & SOUND_MIDI)
				m_pMidiSound = m_pSound;
		} else { /* if no sound */
			if (!(m_wFlags & SOUND_MIDI)) {

				int nExt = getFileName().getLength() - 4; // ".EXT"

				if (nExt <= 0) {
					logError("Sound does not have a file name or proper extension.  Please write better scripts.");
					return false;
				}

				CBofString sBaseStr = getFileName().Left(nExt) + ".TXT";

				Common::File f;
				if (fileExists(sBaseStr) && f.open(sBaseStr.GetBuffer())) {
					Common::String line = f.readLine();

					bofMessageBox(line.c_str(), "Using .TXT for missing .WAV!");
					f.close();
					return true;
				} else {
					logError(buildString("Sound TEXT file could not be read: %s.  Why? because we like you ...", getFileName().GetBuffer()));
					return false;
				}
			}
		}
	}

	return CBagObject::runObject();
}

PARSE_CODES CBagSoundObject::setInfo(CBagIfstream &istr) {
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

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("VOLUME")) {
				istr.eatWhite();
				int n;
				GetIntFromStream(istr, n);
				setVolume(n);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
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
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {

				istr.eatWhite();
				GetAlphaNumFromStream(istr, sStr);

				if (!sStr.Find("WAVE")) {
					// m_xSndType  = WAVE;
					SetWave();
					nObjectUpdated = true;

				} else if (!sStr.Find("MIDI")) {
					// m_xSndType  = MIDI;
					SetMidi();
					nObjectUpdated = true;

				} else if (!sStr.Find("SYNC")) {
					SetSync();
					nObjectUpdated = true;

				} else if (!sStr.Find("ASYNC")) {
					SetASync();
					nObjectUpdated = true;

					// Mix and Wait
					//
				} else if (!sStr.Find("WAITMIX")) {

					SetMix();
					m_bWait = true;
					nObjectUpdated = true;

					// Queue and Wait
					//
				} else if (!sStr.Find("WAITQUEUE")) {

					SetQueue();
					m_bWait = true;
					nObjectUpdated = true;

				} else if (!sStr.Find("QUEUE")) {

					SetQueue();
					nObjectUpdated = true;

				} else if (!sStr.Find("MIX")) {
					SetMix();
					nObjectUpdated = true;

				} else {
					PutbackStringOnStream(istr, sStr);
					PutbackStringOnStream(istr, "AS ");
				}

			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// LOOP PROPERTY FOR SOUNDS
		case 'L': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // performance improvement

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("LOOP")) {
				istr.eatWhite();
				GetIntFromStream(istr, m_nLoops);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// Oversound attribute for sound object
		case 'S': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // performance improvement

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SOUNDOVEROK")) {
				SetSoundOver();
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		//  No match return from funtion
		//
		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
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

void CBagSoundObject::SetQueue(bool b) {
	if (b) {
		m_wFlags = SOUND_MIX | SOUND_QUEUE;

	} else {
		m_wFlags &= ~SOUND_QUEUE;
	}
}

int CBagSoundObject::getVolume() {
	return m_nVol;
}

void CBagSoundObject::setVolume(int nVol) {
	m_nVol = (byte)nVol;
	if (isAttached()) {

		if (m_pSound != nullptr) {
			m_pSound->setVolume(m_nVol);
		}
	}
}

bool CBagSoundObject::isPlaying() {
	bool bPlaying = false;
	if (m_pSound != nullptr) {
		bPlaying = m_pSound->playing();
	}

	return bPlaying;
}

bool CBagSoundObject::isQueued() {
	bool bQueued = false;
	if (m_pSound != nullptr) {
		bQueued = m_pSound->isQueued();
	}

	return bQueued;
}

void CBagSoundObject::SetPlaying(bool bVal) {
	if (((m_wFlags & SOUND_MIDI) && CBagMasterWin::getMidi()) || (((m_wFlags & SOUND_WAVE) || (m_wFlags & SOUND_MIX)) && CBagMasterWin::getDigitalAudio())) {

		if (bVal) {

			if (m_pSound && m_pMidiSound != m_pSound) {

				m_pSound->setQSlot(getState());
				m_pSound->play();

				// If we are supposed to wait until this audio finishes
				if (m_bWait) {
					// Show busy cursor
					CBagMasterWin::setActiveCursor(6);

					EventLoop limiter(EventLoop::FORCE_REPAINT);
					while (m_pSound->isPlaying()) {
						CBofSound::audioTask();

						if (limiter.frame()) {
							m_pSound->stop();
							break;
						}
					}
				}

				if (m_wFlags & SOUND_MIDI)
					m_pMidiSound = m_pSound;
			}
		} else {

			if (m_pSound) {
				m_pSound->stop();
				if (m_wFlags & SOUND_MIDI)
					m_pMidiSound = nullptr;
			}
		}
	}
}

void CBagSoundObject::SetNumOfLoops(int n) {
	m_nLoops = n; // Only have ability to set at creation of BofSound
}

int CBagSoundObject::getProperty(const CBofString &sProp) {
	if (!sProp.Find("VOLUME")) {
		return getVolume();

	}

	if (!sProp.Find("QUEUED")) {
		return isQueued();

	}

	if (!sProp.Find("PLAYING")) {
		return isPlaying();

	}

	if (!sProp.Find("LOOP")) {
		return m_nLoops;
	}

	return CBagObject::getProperty(sProp);
}

void CBagSoundObject::setProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("VOLUME")) {
		setVolume(nVal);

	} else if (!sProp.Find("PLAYING")) {

		if (nVal == 1)
			SetPlaying();
		else
			SetPlaying(false);

	} else if (!sProp.Find("LOOP")) {
		SetNumOfLoops(nVal);
	} else {
		CBagObject::setProperty(sProp, nVal);
	}
}

void CBagSoundObject::SetSync(bool b) {
	m_wFlags = SOUND_WAVE;
	if (!b)
		m_wFlags |= SOUND_ASYNCH;
}

} // namespace Bagel
