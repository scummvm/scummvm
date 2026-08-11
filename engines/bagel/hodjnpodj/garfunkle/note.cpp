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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/garfunkle/note.h"

namespace Bagel {
namespace HodjNPodj {
namespace Garkfunkle {

CNote   *CNote::m_pNoteHead = nullptr;            // pointer to list of linked notes
CNote   *CNote::m_pNoteTail = nullptr;            // pointer to tail of list of linked notes

IMPLEMENT_DYNCREATE(CNote, CObject)

/*************************************************************************
 *
 * CNote()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for note class.
 *
 ************************************************************************/

CNote::CNote() {
	m_nValue = 1;
	m_pNext = nullptr;                                     // it is not yet in the sprite chain and
	m_pPrev = nullptr;                                     // ... thus has no links to other sprites
}


/*************************************************************************
 *
 * ~CNote()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for note class.
 *
 ************************************************************************/

CNote::~CNote() {
}


/*************************************************************************
 *
 * SetValue()
 *
 * Parameters:
 *  int nValue      value which identifies the button associated with the note
 *
 * Return Value:    none
 *
 * Description:     Identify the button to be played at this point in the sequence
 *
 ************************************************************************/

void CNote::SetValue(int nValue) {
	m_nValue = nValue;                  // identify the button to be played
}

/*************************************************************************
 *
 * LinkNote()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Link this note into the list by placing it at the
 *                  the tail of the list
 *
 ************************************************************************/

void CNote::LinkNote() {
	m_pNext = nullptr;                 // link note onto tail of list
	m_pPrev = m_pNoteTail;          //... by pointing it back at the current
	//... tail, making it the tail, and
	if (m_pNoteTail)                //... pointing it at nullptr (the list terminator)
		(*m_pNoteTail).m_pNext = this;
	else
		m_pNoteHead = this;
	m_pNoteTail = this;
}

/*************************************************************************
 *
 * UnLinkNote()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Remove this sprite from the sprite chain and point its
 *                  neighbors at each other to fill the gap
 *
 ************************************************************************/

void CNote::UnLinkNote() {
	if (m_pPrev)                            // disconnect us from the note chain
		(*m_pPrev).m_pNext = m_pNext;       // ... by pointing the one before us, and
	else                                    // ... the one after us, at each other
		m_pNoteHead = m_pNext;              // special case the instances where the
	// ... note to be removed is the first
	if (m_pNext)                            // ...  or the last in the list, update
		(*m_pNext).m_pPrev = m_pPrev;       // ... the head of chain pointer
	else
		m_pNoteTail = m_pPrev;

	m_pNext = m_pPrev = nullptr;
}

/*************************************************************************
 *
 * FlushNoteList()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Remove all notes from the chain and delete them via
 *                  the standard destructor
 *
 ************************************************************************/

void CNote::FlushNoteList() {
	CNote   *pNote;

	while ((pNote = CNote::GetNoteHead())) {
		(*pNote).UnLinkNote();
		delete pNote;
	}
}

} // namespace Garfunkle
} // namespace HodjNPodj
} // namespace Bagel
