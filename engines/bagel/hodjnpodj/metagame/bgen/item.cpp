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

#include "common/str.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {


#ifndef FRAME_EXE
// Descriptive text for each item
const char *CItem::m_pItemText[MG_OBJ_COUNT] = {
	"a Radio-Controlled Flying Anvil",
	"a Beret of Invisibility",
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

// Pluralized descriptive text for each item
const char *CItem::m_pItemPluralText[MG_OBJ_COUNT] = {
	"Radio-Controlled Flying Anvils",
	"Berets of Invisibility",
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

// Place to construct item names
char CItem::m_chTextBuffer[ITEM_BUFFER_LENGTH + 1];

// Disk path specifications for item bitmaps
const char *CItem::m_pItemBitmapPath[MG_OBJ_COUNT] = {
	"meta/art/anvil.bmp",
	"meta/art/beret.bmp",
	"meta/art/broom.bmp",
	"meta/art/candle.bmp",
	"meta/art/chestnut.bmp",
	"meta/art/chisel.bmp",
	"meta/art/crown.bmp",
	"meta/art/crusoe.bmp",
	"meta/art/egg.bmp",
	"meta/art/beater.bmp",
	"meta/art/eyepatch.bmp",
	"meta/art/fishhook.bmp",
	"meta/art/flash.bmp",
	"meta/art/grapnel.bmp",
	"meta/art/hacksaw.bmp",
	"meta/art/herring.bmp",
	"meta/art/hnotes.bmp",
	"meta/art/honey.bmp",
	"meta/art/kumquat.bmp",
	"meta/art/mirror.bmp",
	"meta/art/mish.bmp",
	"meta/art/mosh.bmp",
	"meta/art/paddle.bmp",
	"meta/art/bundt.bmp",
	"meta/art/pnotes.bmp",
	"meta/art/jumprope.bmp",
	"meta/art/scarab.bmp",
	"meta/art/sharpner.bmp",
	"meta/art/scallop.bmp",
	"meta/art/shoelace.bmp",
	"meta/art/staff.bmp",
	"meta/art/swab.bmp",
	"meta/art/swatter.bmp",
	"meta/art/ticket.bmp"
};

// Disk path specifications for item sounds
const char *CItem::m_pItemSoundPath[MG_OBJ_COUNT] = {
	"meta/sound/obj25.wav",
	"meta/sound/obj21.wav",
	"meta/sound/obj15.wav",
	"meta/sound/obj06.wav",
	"meta/sound/obj03.wav",
	"meta/sound/obj27.wav",
	"meta/sound/obj01.wav",
	"meta/sound/obj17.wav",
	"meta/sound/obj24.wav",
	"meta/sound/obj20.wav",
	"meta/sound/obj12.wav",
	"meta/sound/obj22.wav",
	"meta/sound/obj26.wav",
	"meta/sound/obj08.wav",
	"meta/sound/obj18.wav",
	nullptr,
	"meta/sound/obj30.wav",
	"meta/sound/obj13.wav",
	"meta/sound/obj05.wav",
	"meta/sound/obj04.wav",
	"meta/sound/obj32.wav",
	"meta/sound/obj33.wav",
	"meta/sound/obj10.wav",
	"meta/sound/obj19.wav",
	"meta/sound/obj31.wav",
	"meta/sound/obj29.wav",
	"meta/sound/obj11.wav",
	"meta/sound/obj02.wav",
	"meta/sound/obj14.wav",
	"meta/sound/obj23.wav",
	"meta/sound/obj16.wav",
	"meta/sound/obj28.wav",
	"meta/sound/obj09.wav",
	"meta/sound/obj07.wav"
};

// value for each item if sold
const int CItem::m_nValue[MG_OBJ_COUNT] = {
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

CItem::CItem(int nID) {
	m_nID = nID;
	m_nQuantity = 1;
	m_nActionCode = ITEM_ACTION_NONE;
	m_pNotes = nullptr;
	m_pNext = nullptr;
	m_pPrev = nullptr;
}

CItem::~CItem() {
	CNote *pNote;

	while (m_pNotes != nullptr) {						// Remove all associated notes
		pNote = m_pNotes;
		m_pNotes = (*pNote).m_pNext;
		delete pNote;
	}
}

#ifndef FRAME_EXE
const char *CItem::GetDescription(int nID, long nQuantity) {
	if ((nID < MG_OBJ_BASE) ||
		(nID >= MG_OBJ_BASE + MG_OBJ_COUNT))			// Return nullptr if invalid identifier
		m_chTextBuffer[0] = '\0';
	else {
		if (nQuantity > 1)                              // Create blurb for multiple
			Common::sprintf_s(m_chTextBuffer, "%ld %s", nQuantity, m_pItemPluralText[nID - MG_OBJ_BASE]);
		else
			if ((nID == MG_OBJ_CROWN) &&				// Special case having no crowns
				(nQuantity == 0))
				Common::sprintf_s(m_chTextBuffer, "%s %s", "You have no", m_pItemPluralText[nID - MG_OBJ_BASE]);
			else
				Common::strcpy_s(m_chTextBuffer, m_pItemText[nID - MG_OBJ_BASE]);    // get the text for the item
	}

	return(m_chTextBuffer);
}
#endif

/*************************************************************************
 *
 * AddNote()
 *
 * Parameters:
 *	int nID;		note identifier
 *	int nPerson		identifier of note giver
 *	int nPlace		identifier of note location
 *
 * Return Value:
 *	bool			success / failure
 *
 * Description:		add a note entry to the notebook object.
 *
 ************************************************************************/

bool CItem::AddNote(int nID, int nClue, int nRepeat, int nPerson, int nPlace) {
	bool	bSuccess = false;
	CNote *pNote;

	if ((GetID() != MG_OBJ_HODJ_NOTEBOOK) &&			// Punt if not using notebook
		(GetID() != MG_OBJ_PODJ_NOTEBOOK))
		return false;

	pNote = new CNote(nID, nClue, nRepeat, nPerson, nPlace);	// Create the note object
	if (pNote != nullptr) {							// ... and add it to the list
		(*pNote).m_pNext = m_pNotes;				// Make head of list follow us
		if (m_pNotes != nullptr)                    // Have list point back at us
			(*m_pNotes).m_pPrev = pNote;
		m_pNotes = pNote;							// Make us be new head of list
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
 *	bool			success / failure
 *
 * Description:		add a note entry to the notebook object.
 *
 ************************************************************************/

bool CItem::AddNote(CNote *pNote) {
	bool bSuccess = false;

	if (pNote != nullptr) {							// ... and add it to the list
		(*pNote).m_pNext = m_pNotes;				// Make head of list follow us
		if (m_pNotes != nullptr)                    // Have list point back at us
			(*m_pNotes).m_pPrev = pNote;
		m_pNotes = pNote;							// Make us be new head of list
		bSuccess = true;
	}

	return bSuccess;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
