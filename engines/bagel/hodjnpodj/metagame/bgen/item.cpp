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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/bgen/note.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

IMPLEMENT_DYNCREATE(CItem, CObject)


#ifndef FRAME_EXE
// descriptive text for each item
const char *CItem::m_pItemText[MG_OBJ_COUNT] = {
	"a Radio-Controlled Flying Anvil",
	"a Beret of Invisibilty",
	"a Broomstick",
	"Scented Candles",
	"a Magic Chestnut",
	"a Chisel of Ennui",
	"a Gold Crown",
	"a Copy of \"Robinson Crusoe\"",
	"Exploding Eggs",
	"an Eggbeater",
	"an Eyepatch",
	"a Fishhook",
	"a Heavy-Duty Flashlight",
	"a Grappling Hook",
	"a Killer Hacksaw",
	"a Red Herring",
	"Hodj's Log",
	"a Pot of Honey",
	"an Enchanted Kumquat",
	"a Mirror of Deflection",
	"Mish",
	"Mosh",
	"a Kayak Paddle",
	"a Bundt Pan",
	"Podj's Log",
	"an Extra-Long Jump Rope",
	"a Poison Scarab",
	"an Enchanted Pencil Sharpener",
	"a Scallop Shell",
	"a Giant Shoelace",
	"a Magic Staff",
	"a Killer Cotton Swab",
	"a Flyswatter of Death",
	"a General Admission Ticket"
};

// pluralized descriptive text for each item
const char *CItem::m_pItemPluralText[MG_OBJ_COUNT] = {
	"Radio-Controlled Flying Anvils",
	"Berets of Invisibilty",
	"Broomsticks",
	"Scented Candles",
	"Magic Chestnuts",
	"Chisels of Ennui",
	"Gold Crowns",
	"Copies of \"Robinson Crusoe\"",
	"Exploding Eggs",
	"Eggbeaters",
	"Eyepatches",
	"Fishhooks",
	"Heavy-Duty Flashlights",
	"Grappling Hooks",
	"Killer Hacksaws",
	"Red Herrings",
	"Hodj's Logs",
	"Pots of Honey",
	"Enchanted Kumquats",
	"Mirrors of Deflection",
	"Mish",
	"Mosh",
	"Kayak Paddles",
	"Bundt Pans",
	"Podj's Logs",
	"Extra-Long Jump Ropes",
	"Poison Scarabs",
	"Enchanted Pencil Sharpeners",
	"Scallop Shells",
	"Giant Shoelaces",
	"Magic Staves",
	"Killer Cotton Swabs",
	"Flyswatters of Death",
	"General Admission Tickets"
};

// place to construct item names
char    CItem::m_chTextBuffer[ITEM_BUFFER_LENGTH + 1];

// disk path specifications for item bitmaps
const char *CItem::m_pItemBitmapPath[MG_OBJ_COUNT] = {
	".\\art\\anvil.bmp",
	".\\art\\beret.bmp",
	".\\art\\broom.bmp",
	".\\art\\candle.bmp",
	".\\art\\chestnut.bmp",
	".\\art\\chisel.bmp",
	".\\art\\crown.bmp",
	".\\art\\crusoe.bmp",
	".\\art\\egg.bmp",
	".\\art\\beater.bmp",
	".\\art\\eyepatch.bmp",
	".\\art\\fishhook.bmp",
	".\\art\\flash.bmp",
	".\\art\\grapnel.bmp",
	".\\art\\hacksaw.bmp",
	".\\art\\herring.bmp",
	".\\art\\hnotes.bmp",
	".\\art\\honey.bmp",
	".\\art\\kumquat.bmp",
	".\\art\\mirror.bmp",
	".\\art\\mish.bmp",
	".\\art\\mosh.bmp",
	".\\art\\paddle.bmp",
	".\\art\\bundt.bmp",
	".\\art\\pnotes.bmp",
	".\\art\\jumprope.bmp",
	".\\art\\scarab.bmp",
	".\\art\\sharpner.bmp",
	".\\art\\scallop.bmp",
	".\\art\\shoelace.bmp",
	".\\art\\staff.bmp",
	".\\art\\swab.bmp",
	".\\art\\swatter.bmp",
	".\\art\\ticket.bmp"
};

// disk path specifications for item sounds
const char *CItem::m_pItemSoundPath[MG_OBJ_COUNT] = {
	".\\sound\\obj25.wav",
	".\\sound\\obj21.wav",
	".\\sound\\obj15.wav",
	".\\sound\\obj06.wav",
	".\\sound\\obj03.wav",
	".\\sound\\obj27.wav",
	".\\sound\\obj01.wav",
	".\\sound\\obj17.wav",
	".\\sound\\obj24.wav",
	".\\sound\\obj20.wav",
	".\\sound\\obj12.wav",
	".\\sound\\obj22.wav",
	".\\sound\\obj26.wav",
	".\\sound\\obj08.wav",
	".\\sound\\obj18.wav",
	nullptr,
	".\\sound\\obj30.wav",
	".\\sound\\obj13.wav",
	".\\sound\\obj05.wav",
	".\\sound\\obj04.wav",
	".\\sound\\obj32.wav",
	".\\sound\\obj33.wav",
	".\\sound\\obj10.wav",
	".\\sound\\obj19.wav",
	".\\sound\\obj31.wav",
	".\\sound\\obj29.wav",
	".\\sound\\obj11.wav",
	".\\sound\\obj02.wav",
	".\\sound\\obj14.wav",
	".\\sound\\obj23.wav",
	".\\sound\\obj16.wav",
	".\\sound\\obj28.wav",
	".\\sound\\obj09.wav",
	".\\sound\\obj07.wav"
};

// value for each item if sold
int CItem::m_nValue[MG_OBJ_COUNT] = {
	49,
	50,
	32,
	45,
	46,
	39,
	0,
	39,
	42,
	35,
	27,
	29,
	33,
	31,
	48,
	0,
	0,
	28,
	38,
	45,
	0,
	0,
	34,
	30,
	0,
	33,
	52,
	48,
	26,
	36,
	44,
	26,
	40,
	30
};
#endif

/*************************************************************************
 *
 * CItem()
 *
 * Parameters:
 *  int nID         item identifier
 *
 * Return Value:    none
 *
 * Description:     Constructor for item class.  Initialize all fields
 *                  to reflect a single instance of the item.
 *
 ************************************************************************/

CItem::CItem(int nID) {
	m_nID = nID;
	m_nQuantity = 1;
	m_nActionCode = ITEM_ACTION_NONE;
	m_pNotes = nullptr;
	m_pNext = nullptr;
	m_pPrev = nullptr;
}


/*************************************************************************
 *
 * ~CItem()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for item class.  Item item objects
 *                  are automatically purged from the equipment list.
 *
 ************************************************************************/

CItem::~CItem() {
	CNote   *pNote;

	while (m_pNotes != nullptr) {                      // remove all associated notes
		pNote = m_pNotes;
		m_pNotes = (*pNote).m_pNext;
		delete pNote;
	}
}


/*************************************************************************
 *
 * GetDescription()
 *
 * Parameters:
 *  int nID;        item identifier
 *
 * Return Value:
 *  char *          pointer to item's descriptive text
 *
 * Description:     return a pointer to the descriptive text for an item.
 *
 ************************************************************************/
#ifndef FRAME_EXE
char *CItem::GetDescription(int nID, long nQuantity) {
	if ((nID < MG_OBJ_BASE) ||
	        (nID >= MG_OBJ_BASE + MG_OBJ_COUNT))            // return nullptr if invalid identifier
		m_chTextBuffer[0] = '\0';
	else {
		if (nQuantity > 1)                              // create blurb for multiple
			Common::sprintf_s(m_chTextBuffer, "%ld %s", nQuantity, m_pItemPluralText[nID - MG_OBJ_BASE]);
		else if ((nID == MG_OBJ_CROWN) &&                   // special case having no crowns
		         (nQuantity == 0))
			Common::sprintf_s(m_chTextBuffer, "%s %s", "You have no", m_pItemPluralText[nID - MG_OBJ_BASE]);
		else
			Common::strcpy_s(m_chTextBuffer, m_pItemText[nID - MG_OBJ_BASE]);   // get the text for the item
	}

	return (m_chTextBuffer);
}
#endif

/*************************************************************************
 *
 * AddNote()
 *
 * Parameters:
 *  int nID;        note identifier
 *  int nPerson     identifier of note giver
 *  int nPlace      identifier of note location
 *
 * Return Value:
 *  bool            success / failure
 *
 * Description:     add a note entry to the notebook object.
 *
 ************************************************************************/

bool CItem::AddNote(int nID, int nClue, int nRepeat, int nPerson, int nPlace) {
	bool    bSuccess = false;
	CNote   *pNote;

	if ((GetID() != MG_OBJ_HODJ_NOTEBOOK) &&            // punt if not using notebook
	        (GetID() != MG_OBJ_PODJ_NOTEBOOK))
		return false;

	pNote = new CNote(nID, nClue, nRepeat, nPerson, nPlace);    // create the note object
	if (pNote != nullptr) {                            // ... and add it to the list
		(*pNote).m_pNext = m_pNotes;                // make head of list follow us
		if (m_pNotes != nullptr)                       // have list point back at us
			(*m_pNotes).m_pPrev = pNote;
		m_pNotes = pNote;                           // make us be new head of list
		bSuccess = true;
	}

	return bSuccess;
}


/*************************************************************************
 *
 * AddNote()
 *
 * Parameters:
 *  CNote *pNote    note pointer
 *
 * Return Value:
 *  bool            success / failure
 *
 * Description:     add a note entry to the notebook object.
 *
 ************************************************************************/

bool CItem::AddNote(CNote *pNote) {
	bool    bSuccess = false;

	if (pNote != nullptr) {                            // ... and add it to the list
		(*pNote).m_pNext = m_pNotes;                // make head of list follow us
		if (m_pNotes != nullptr)                       // have list point back at us
			(*m_pNotes).m_pPrev = pNote;
		m_pNotes = pNote;                           // make us be new head of list
		bSuccess = true;
	}

	return bSuccess;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

/////////////////////////////////////////////////////////////////////////////
