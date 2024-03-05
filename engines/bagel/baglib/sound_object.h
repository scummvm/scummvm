
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

#ifndef BAGEL_BAGLIB_SOUND_OBJECT_H
#define BAGEL_BAGLIB_SOUND_OBJECT_H

#include "bagel/baglib/object.h"
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/sound.h"

namespace Bagel {

/**
 * CBagSoundObject is an object that can be place within the slide window.
 */
class CBagSoundObject : public CBagObject {
private:
	static CBofSound *m_pMidiSound; // There is only one allowed at a time
	CBofSound *m_pSound;

	WORD m_wFlags;
	INT m_nLoops;
	UBYTE m_nVol;

protected:
	UBYTE m_bWait;

public:
	CBagSoundObject();
	virtual ~CBagSoundObject();
	static void initStatics();

	VOID KillSound();
	ERROR_CODE NewSound(CBofWindow *pWin);

	// Return TRUE if the Object had members that are properly initialized/de-initialized
	ERROR_CODE Attach() {
		return Attach((CBofWindow *)CBagel::GetBagApp()->GetMasterWnd() /*CBofApp::GetApp()->GetMainWindow()*/);
	}
	ERROR_CODE Attach(CBofWindow *pWnd);
	BOOL IsAttached() {
		return m_pSound != nullptr;
	}
	ERROR_CODE Detach();

	CBofSound *GetLastMidi() {
		return m_pMidiSound;
	}

	CBofSound *GetSound() {
		return m_pSound;
	}

	VOID SetWave() {
		m_wFlags = SOUND_WAVE;
	}
	VOID SetMidi() {
		m_wFlags = (SOUND_MIDI | SOUND_LOOP);
	}

	// Gives ability to sound over certain sounds
	VOID SetSoundOver() {
		m_wFlags |= SOUND_OVEROK;
	}

	VOID SetSync(BOOL b = TRUE);
	VOID SetASync(BOOL b = TRUE) {
		SetSync(!b);
	}
	BOOL IsSync() {
		return m_wFlags & SOUND_ASYNCH;
	}

	VOID SetMix() {
		m_wFlags = SOUND_MIX;
	}
	VOID SetQueue(BOOL b = TRUE);

	virtual BOOL RunObject();

	VOID SetVolume(int nVol);
	INT GetVolume();

	VOID SetNumOfLoops(INT n);

	INT GetProperty(const CBofString &sProp);
	VOID SetProperty(const CBofString &sProp, int nVal);

	/**
	 * Takes in info and then removes the relative information and returns
	 * the info without the relevant info.
	 */
	PARSE_CODES SetInfo(bof_ifstream &istr);

	//  Added properties to sound object
	BOOL IsPlaying();
	BOOL IsQueued();

	VOID SetPlaying(BOOL bVal = TRUE);
};

} // namespace Bagel

#endif
