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

#ifndef HODJNPODJ_GARFUNKLE_NOTE_H
#define HODJNPODJ_GARFUNKLE_NOTE_H

#include "bagel/hodjnpodj/hnplibs/dibapi.h"

namespace Bagel {
namespace HodjNPodj {
namespace Garkfunkle {

class CNote : public CObject {
	DECLARE_DYNCREATE(CNote)

// Constructors
public:
	CNote();    // use "new" operator to create notes, then SetValue

// Destructors
public:
	~CNote();

// Implementation
public:
	void    SetValue(int nValue);
	int GetValue() const {
		return m_nValue;
	}
	void LinkNote();
	void UnLinkNote();
	CNote *GetNextNote() const {
		return m_pNext;
	}
	CNote *GetPrevNote() const {
		return m_pPrev;
	}

	static CNote *GetNoteHead() {
		return m_pNoteHead;
	}
	static void FlushNoteList();

private:
	int         m_nValue = 0;
	CNote       *m_pNext = nullptr;		// pointer to next note in chain
	CNote       *m_pPrev = nullptr;		// pointer to previous note in chain

	static CNote    *m_pNoteHead;		// pointer to linked chain of notes
	static CNote    *m_pNoteTail;		// pointer to tail of list of notes
};

} // namespace Garfunkle
} // namespace HodjNPodj
} // namespace Bagel

#endif
