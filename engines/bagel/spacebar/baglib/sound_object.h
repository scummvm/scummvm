
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

#include "bagel/spacebar/baglib/object.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace SpaceBar {

/**
 * CBagSoundObject is an object that can be place within the slide window.
 */
class CBagSoundObject : public CBagObject {
private:
	static CBofSound *_pMidiSound; // There is only one allowed at a time
	CBofSound *_pSound;

	uint16 _wFlags;
	int _nLoops;
	byte _nVol;

protected:
	byte _bWait;

public:
	CBagSoundObject();
	virtual ~CBagSoundObject();
	static void initialize();

	void killSound();
	void newSound(CBofWindow *pWin);

	// Return true if the Object had members that are properly initialized/de-initialized
	ErrorCode attach() override {
		return attach((CBofWindow *)CBagel::getBagApp()->getMasterWnd());
	}
	ErrorCode attach(CBofWindow *pWnd);
	bool isAttached() override {
		return _pSound != nullptr;
	}
	ErrorCode detach() override;

	CBofSound *getLastMidi() {
		return _pMidiSound;
	}

	CBofSound *getSound() {
		return _pSound;
	}

	void setWave() {
		_wFlags = SOUND_WAVE;
	}
	void setMidi() {
		_wFlags = (SOUND_MIDI | SOUND_LOOP);
	}

	// Gives ability to sound over certain sounds
	void setSoundOver() {
		_wFlags |= SOUND_OVEROK;
	}

	void setSync(bool b = true);
	void setASync(bool b = true) {
		setSync(!b);
	}
	bool isSync() {
		return _wFlags & SOUND_ASYNCH;
	}

	void setMix() {
		_wFlags = SOUND_MIX;
	}
	void setQueue(bool b = true);

	bool runObject() override;

	void setVolume(int nVol);
	int getVolume();

	void setNumOfLoops(int n);

	int getProperty(const CBofString &sProp) override;
	void setProperty(const CBofString &sProp, int nVal) override;

	/**
	 * Takes in info and then removes the relative information and returns
	 * the info without the relevant info.
	 */
	ParseCodes setInfo(CBagIfstream &istr) override;

	//  Added properties to sound object
	bool isPlaying();
	bool isQueued();

	void setPlaying(bool bVal = true);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
