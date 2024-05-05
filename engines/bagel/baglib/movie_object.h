
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

#ifndef BAGEL_BAGLIB_MOVIE_OBJECT_H
#define BAGEL_BAGLIB_MOVIE_OBJECT_H

#include "bagel/baglib/object.h"
#include "bagel/baglib/sound_object.h"

namespace Bagel {

#define ASYNCH_DONTQUEUE 0x0001
#define ASYNCH_PLAYIMMEDIATE 0x0002
#define ASYNCH_DONTOVERRIDE 0x0004

/**
 * CBagMovieObject is an object that can be place within the slide window.
 */
class CBagMovieObject : public CBagObject {
public:
	enum class DISP_TYPE { MOVIE, EXAMINE, PDAMSG, ASYNCH_PDAMSG };

private:
	DISP_TYPE _xDisplayType;
	byte _bFlyThru;
	int16 _nAsynchFlags;
	bool _bIncrement : 1;      // Increment timer for this movie?
	bool _bOnBlack : 1;        // Play movie on a black background.
	CBagSoundObject *_pSndObj; // associated sound object

public:
	CBagMovieObject();
	virtual ~CBagMovieObject();

	// Associated sound object access members
	void setAssociatedSound(CBagSoundObject *p) {
		_pSndObj = p;
	}
	CBagSoundObject *getAssociatedSound() const {
		return _pSndObj;
	}

	ParseCodes setInfo(CBagIfstream &istr);

	virtual bool runObject();

	// Return true if this asynch zelda movie can play right now
	bool asynchPDAMovieCanPlay();

	// Special routines for handling asynch zelda movies
	void setDontQueue() {
		_nAsynchFlags |= ASYNCH_DONTQUEUE;
	}
	void setDontOverride() {
		_nAsynchFlags |= ASYNCH_DONTOVERRIDE;
	}
	void setPlayImmediate() {
		_nAsynchFlags |= ASYNCH_PLAYIMMEDIATE;
	}
	void setIncrement(bool b = true) {
		_bIncrement = b;
	}
	void setOnBlack(bool b = true) {
		_bOnBlack = b;
	}

	bool isDontQueue() const {
		return (_nAsynchFlags & ASYNCH_DONTQUEUE) != 0;
	}
	bool isDontOverride() const {
		return (_nAsynchFlags & ASYNCH_DONTOVERRIDE) != 0;
	}
	bool isPlayImmediate() const {
		return (_nAsynchFlags & ASYNCH_PLAYIMMEDIATE) != 0;
	}
	bool isIncrement() const {
		return _bIncrement;
	}
	bool isOnBlack() const {
		return _bOnBlack;
	}
};

} // namespace Bagel

#endif
