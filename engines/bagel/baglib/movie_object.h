
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
	// CBofSize          m_xSize;
	DISP_TYPE m_xDisplayType;
	byte m_bFlyThru;
	int16 m_nAsynchFlags;
	bool m_bIncrement : 1;      // Increment timer for this movie?
	bool m_bOnBlack : 1;        // Play movie on a black background.
	CBagSoundObject *m_pSndObj; // associated sound object

public:
	CBagMovieObject();
	virtual ~CBagMovieObject();

	// Associated sound object access members
	void SetAssociatedSound(CBagSoundObject *p) {
		m_pSndObj = p;
	}
	CBagSoundObject *GetAssociatedSound() const {
		return m_pSndObj;
	}

	// Return true if the Object had members that are properly initialized/de-initialized
	// ERROR_CODE           Attach();
	// ERROR_CODE           Detach();
	PARSE_CODES SetInfo(bof_ifstream &istr);

	virtual bool RunObject();

	// Return true if this asynch zelda movie can play right now
	bool AsynchPDAMovieCanPlay();

	// Special routines for handling asynch zelda movies
	void SetDontQueue() {
		m_nAsynchFlags |= ASYNCH_DONTQUEUE;
	}
	void SetDontOverride() {
		m_nAsynchFlags |= ASYNCH_DONTOVERRIDE;
	}
	void SetPlayImmediate() {
		m_nAsynchFlags |= ASYNCH_PLAYIMMEDIATE;
	}
	void SetIncrement(bool b = true) {
		m_bIncrement = b;
	}
	void SetOnBlack(bool b = true) {
		m_bOnBlack = b;
	}

	bool IsDontQueue() const {
		return (m_nAsynchFlags & ASYNCH_DONTQUEUE) != 0;
	}
	bool IsDontOverride() const {
		return (m_nAsynchFlags & ASYNCH_DONTOVERRIDE) != 0;
	}
	bool IsPlayImmediate() const {
		return (m_nAsynchFlags & ASYNCH_PLAYIMMEDIATE) != 0;
	}
	bool IsIncrement() const {
		return m_bIncrement;
	}
	bool IsOnBlack() const {
		return m_bOnBlack;
	}
};

} // namespace Bagel

#endif
