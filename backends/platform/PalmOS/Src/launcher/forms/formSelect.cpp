/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2007 The ScummVM project
 * Copyright (C) 2002-2007 Chris Apers - PalmOS Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include <PalmOS.h>
#include "StarterRsc.h"
#include "formUtil.h"
#include "games.h"
#include "start.h"
#include "common/util.h"

static Char **items = NULL;
Int16 selectedEngine = -1;

// Supported games
static const char *engine_agos[] = {
	"Demon in my Pocket",
	"Elvira - Mistress of the Dark",
	"Elvira II - The Jaws of Cerberus",
	"Jumble",
	"NoPatience",
	"Simon the Sorcerer I",
	"Simon the Sorcerer II",
	"Swampy Adventures",
	"The Feeble Files",
	"Waxworks"
};

static const char *engine_sky[] = {
	"Floppy, CD and Demo"
};

static const char *engine_sword1[] = {
	"The Shadow of the Templars (PC/Mac)",
	"The Shadow of the Templars (Demo)"
};

static const char *engine_sword2[] = {
	"The Smoking Mirror",
	"The Smoking Mirror (Demo)"
};

static const char *engine_cine[] = {
	"Future Wars",
	"Operation Stealth"
};

static const char *engine_queen[] = {
	"Flight of the Amazon Queen"
};

static const char *engine_lure[] = {
	"Lure of the Tempress"
};

static const char *engine_gob[] = {
	"Bargon Attack",
	"Gobliiins",
	"Gobliins 2",
	"Goblins Quest 3",
	"The Bizarre Adventures of Woodruff and the Schnibble",
	"Ween: The Prophecy",
};

static const char *engine_kyra[] = {
	"The Legend of Kyrandia",
	"The Legend of Kyrandia: The Hand of Fate",
	"The Legend of Kyrandia: Malcolm's Revenge"
};

static const char *engine_parallaction[] = {
	"Nippon Safes Inc."
};

static const char *engine_saga[] = {
	"I Have No Mouth And I Must Scream",
	"Inherit the earth"
};

static const char *engine_scumm[] = {
	"Day of the Tentacle",
	"Indiana Jones and the Fate of Atlantis",
	"Indiana Jones and the Last Crusade",
	"Loom",
	"Maniac Mansion",
	"Monkey Island 2: LeChuck's Revenge",
	"Passport to Adventure",
	"Sam & Max Hit the Road",
	"The Secret of Monkey Island"
	"Zak McKracken and the Alien Mindbenders"
};

static const char *engine_agi[] = {
	"AGI Tetris",
	"Caitlyn's Destiny",
	"Donald Duck's Playground",
	"Fanmade AGI game",
	"Gold Rush!",
	"King's Quest I: Quest for the Crown",
	"King's Quest II: Romancing the Throne",
	"King's Quest III: To Heir Is Human",
	"King's Quest IV: The Perils of Rosella",
	"Leisure Suit Larry in the Land of the Lounge Lizards",
	"Mixed-Up Mother Goose",
	"Manhunter 1: New York",
	"Manhunter 2: San Francisco",
	"Police Quest I: In Pursuit of the Death Angel",
	"Serguei's Destiny 1",
	"Serguei's Destiny 2",
	"Space Quest 0: Replicated",
	"Space Quest I: The Sarien Encounter",
	"Space Quest II: Vohaul's Revenge",
	"Space Quest X: The Lost Chapter",
	"The Black Cauldron",
	"Xmas Card"
};

static const char *engine_touche[] = {
	"Touche: The Adventures of the Fifth Musketeer"
};

static const char *engine_cruise[] = {
	"Cruise for a Corpse"
};

static const struct {
	int size;
	const char **listP;
} supported[] = {
	{ ARRAYSIZE(engine_agos), engine_agos },
	{ ARRAYSIZE(engine_sky), engine_sky },
	{ ARRAYSIZE(engine_sword1), engine_sword1 },
	{ ARRAYSIZE(engine_sword2), engine_sword2 },
	{ ARRAYSIZE(engine_cine), engine_cine },
	{ ARRAYSIZE(engine_queen), engine_queen },
	{ ARRAYSIZE(engine_lure), engine_lure },
	{ ARRAYSIZE(engine_gob), engine_gob },
	{ ARRAYSIZE(engine_kyra), engine_kyra },
	{ ARRAYSIZE(engine_parallaction), engine_parallaction },
	{ ARRAYSIZE(engine_saga), engine_saga },
	{ ARRAYSIZE(engine_scumm), engine_scumm },
	{ ARRAYSIZE(engine_agi), engine_agi },
	{ ARRAYSIZE(engine_touche), engine_touche },
	{ ARRAYSIZE(engine_cruise), engine_cruise }
};

static void SelectorSetList(Int16 sel) {
	ListType *listP;
	FormPtr frmP = FrmGetActiveForm();

	Boolean toBeDrawn = (items != NULL);
	if (items)
		MemPtrFree(items);

	listP = (ListType *)FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, EngineSupportedList));
	items = (Char **)MemPtrNew(supported[sel].size * sizeof(Char *));

	for (int i = 0; i < supported[sel].size; i++)
		items[i] = (Char *)supported[sel].listP[i];

	LstSetListChoices (listP, items, supported[sel].size);
	LstSetTopItem(listP, 0);
	LstSetSelection(listP, -1);

	if (toBeDrawn) {
		WinScreenLock(winLockCopy);
		LstDrawList(listP);
		WinScreenUnlock();
	}
}

static void SelectorFormInit() {
	ListType *listP;
	FormPtr frmP = FrmGetActiveForm();

	listP = (ListType *)FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, EngineListList));
	itemsText = (Char **)MemPtrNew(ENGINE_COUNT * sizeof(Char *));
	
	for (int i = 0; i < ENGINE_COUNT; i++)
		itemsText[i] = (Char *)engines[i].nameP;
		
	LstSetListChoices (listP, itemsText, ENGINE_COUNT);
	LstSetSelection(listP, 0);

	SelectorSetList(0);

	FrmDrawForm(frmP);
}

static void SelectorFormFree(bool quit) {
	items = NULL;

	if (!quit) {
		ListType *listP = (ListType *)GetObjectPtr(EngineListList);
		Int16 sel = LstGetSelection(listP);

		FrmReturnToMain();
		StartScummVM(sel);

	} else {
		FrmReturnToMain();

		if (bDirectMode) {
			// force exit if nothing selected
			EventType event;
			event.eType = keyDownEvent;
			event.data.keyDown.chr = vchrLaunch;
			event.data.keyDown.modifiers = commandKeyMask;
			EvtAddUniqueEventToQueue(&event, 0, true);
		}
	}
}

Boolean SelectorFormHandleEvent(EventPtr eventP) {
	FormPtr frmP = FrmGetActiveForm();
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			SelectorFormInit();
			handled = true;
			break;

		case frmCloseEvent:
			SelectorFormFree(true);
			handled = true;
			break;

		case lstSelectEvent:
			if (eventP->data.lstSelect.listID == EngineSupportedList)
				LstSetSelection(eventP->data.lstSelect.pList, -1);
			else 
				SelectorSetList(eventP->data.lstSelect.selection);
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case EngineOkButton:
					SelectorFormFree(false);
					break;

				case EngineCancelButton:
					SelectorFormFree(true);
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
