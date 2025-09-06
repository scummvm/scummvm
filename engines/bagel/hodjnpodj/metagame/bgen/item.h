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

#ifndef BAGEL_METAGAME_BGEN_ITEM_H
#define BAGEL_METAGAME_BGEN_ITEM_H

#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/hodjnpodj/metagame/bgen/note.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define ITEM_BUFFER_LENGTH      132

// action codes
#define ITEM_ACTION_NONE        0
#define ITEM_ACTION_NOTEBOOK    1
#define ITEM_ACTION_SOUND       2


class CItem : public CObject {
	DECLARE_DYNCREATE(CItem)

// Constructors
public:
	CItem(int nID = -1);

// Destructors
public:
	~CItem();

// Implementation
public:
	int GetID() {
		return (m_nID);
	}

	long GetQuantity() {
		return (m_nQuantity);
	}
	void SetQuantity(long nQuantity) {
		m_nQuantity = nQuantity;
	}

	#ifndef FRAME_EXE
	char *GetDescription() {
		return (GetDescription(m_nID, m_nQuantity));
	}
	#endif
	int GetActionCode() {
		return (m_nActionCode);
	}
	void SetActionCode(int nActionCode) {
		m_nActionCode = nActionCode;
	}

	#ifndef FRAME_EXE
	const char *GetArtSpec() {
		return (m_pItemBitmapPath[m_nID - MG_OBJ_BASE]);
	}
	const char *GetSoundSpec() {
		return (m_pItemSoundPath[m_nID - MG_OBJ_BASE]);
	}

	int GetValue() {
		return (m_nValue[m_nID - MG_OBJ_BASE]);
	}
	#endif

	bool AddNote(int nID, int nClue, int nRepeat, int nPerson, int nPlace);
	bool AddNote(CNote *pNote);
	CNote *GetFirstNote() {
		return (m_pNotes);
	}

	CItem  *GetNext() {
		return (m_pNext);
	}
	CItem *GetPrev() {
		return (m_pPrev);
	}


	static char *GetDescription(int nID, long nQuantity);


public:
	int     m_nID;                  // item identifier code
	long    m_nQuantity;            // number of units of the item
	int     m_nActionCode;          // action code for mouse click
	CNote   *m_pNotes;              // pointer to notes for noteboook item
	CItem   *m_pNext;               // pointer to next item in the list
	CItem   *m_pPrev;               // pointer to previous item in the list

private:

	#ifndef FRAME_EXE
	static const char *m_pItemText[MG_OBJ_COUNT];   // descriptive text for each item
	static const char *m_pItemPluralText[MG_OBJ_COUNT]; // pluralized descriptive text for each item
	static const char *m_pItemBitmapPath[MG_OBJ_COUNT]; // disk files specification for each item
	static const char *m_pItemSoundPath[MG_OBJ_COUNT];  // disk files specification for each item
	static int  m_nValue[MG_OBJ_COUNT];             // value of each item
	#endif
	static char m_chTextBuffer[ITEM_BUFFER_LENGTH + 1];     // place to construct item names
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
