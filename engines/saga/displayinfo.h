/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2007 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SAGA_DISPLAYINFO_H
#define SAGA_DISPLAYINFO_H

namespace Saga {

struct PanelButton {
	PanelButtonType type;
	int xOffset;
	int yOffset;
	int width;
	int height;
	int id;
	uint16 ascii;
	int state;
	int upSpriteNumber;
	int downSpriteNumber;
	int overSpriteNumber;
};

struct GameDisplayInfo {
	int logicalWidth;
	int logicalHeight;

	int pathStartY;
	int sceneHeight;

	int statusXOffset;
	int statusYOffset;
	int statusWidth;
	int statusHeight;
	int statusTextY;
	int statusTextColor;
	int statusBGColor;

	int saveReminderXOffset;
	int saveReminderYOffset;
	int saveReminderWidth;
	int saveReminderHeight;
	int saveReminderFirstSpriteNumber;
	int saveReminderSecondSpriteNumber;

	int leftPortraitXOffset;
	int leftPortraitYOffset;
	int rightPortraitXOffset;
	int rightPortraitYOffset;

	int inventoryUpButtonIndex;
	int inventoryDownButtonIndex;
	int inventoryRows;
	int inventoryColumns;

	int mainPanelXOffset;
	int mainPanelYOffset;
	int mainPanelButtonsCount;
	PanelButton *mainPanelButtons;

	int converseMaxTextWidth;
	int converseTextHeight;
	int converseTextLines;
	int converseUpButtonIndex;
	int converseDownButtonIndex;

	int conversePanelXOffset;
	int conversePanelYOffset;
	int conversePanelButtonsCount;
	PanelButton *conversePanelButtons;

	int optionSaveFilePanelIndex;
	int optionSaveFileSliderIndex;
	uint32 optionSaveFileVisible;

	int optionPanelXOffset;
	int optionPanelYOffset;
	int optionPanelButtonsCount;
	PanelButton *optionPanelButtons;

	int quitPanelXOffset;
	int quitPanelYOffset;
	int quitPanelWidth;
	int quitPanelHeight;
	int quitPanelButtonsCount;
	PanelButton *quitPanelButtons;

	int loadPanelXOffset;
	int loadPanelYOffset;
	int loadPanelWidth;
	int loadPanelHeight;
	int loadPanelButtonsCount;
	PanelButton *loadPanelButtons;

	int saveEditIndex;
	int savePanelXOffset;
	int savePanelYOffset;
	int savePanelWidth;
	int savePanelHeight;
	int savePanelButtonsCount;
	PanelButton *savePanelButtons;

	int protectEditIndex;
	int protectPanelXOffset;
	int protectPanelYOffset;
	int protectPanelWidth;
	int protectPanelHeight;
	int protectPanelButtonsCount;
	PanelButton *protectPanelButtons;
};

#define ITE_CONVERSE_MAX_TEXT_WIDTH (256 - 60)
#define ITE_CONVERSE_TEXT_HEIGHT        10
#define ITE_CONVERSE_TEXT_LINES     4

// ITE section
static PanelButton ITE_MainPanelButtons[] = {
	{kPanelButtonVerb,		52,4,	57,10,	kVerbITEWalkTo,'w',0,	0,1,0},
	{kPanelButtonVerb,		52,15,	57,10,	kVerbITELookAt,'l',0,	2,3,0},
	{kPanelButtonVerb,		52,26,	57,10,	kVerbITEPickUp,'p',0,	4,5,0},
	{kPanelButtonVerb,		52,37,	57,10,	kVerbITETalkTo,'t',0,	0,1,0},
	{kPanelButtonVerb,		110,4,	56,10,	kVerbITEOpen,'o',0,	6,7,0},
	{kPanelButtonVerb,		110,15,	56,10,	kVerbITEClose,'c',0,	8,9,0},
	{kPanelButtonVerb,		110,26,	56,10,	kVerbITEUse,'u',0,		10,11,0},
	{kPanelButtonVerb,		110,37,	56,10,	kVerbITEGive,'g',0,	12,13,0},
	{kPanelButtonArrow,		306,6,	8,5,	-1,'U',0,			0,4,2},
	{kPanelButtonArrow,		306,41,	8,5,	1,'D',0,			1,5,3},

	{kPanelButtonInventory,	181 + 32*0,6,	27,18,	0,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*1,6,	27,18,	1,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*2,6,	27,18,	2,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*3,6,	27,18,	3,'-',0,	0,0,0},

	{kPanelButtonInventory,	181 + 32*0,27,	27,18,	4,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*1,27,	27,18,	5,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*2,27,	27,18,	6,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*3,27,	27,18,	7,'-',0,	0,0,0}
};

static PanelButton ITE_ConversePanelButtons[] = {
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 0, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	0,'1',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 1, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	1,'2',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 2, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	2,'3',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 3, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	3,'4',0,	0,0,0},
	{kPanelButtonArrow,			257,6,	9,6,	-1,'u',0,	0,4,2},
	{kPanelButtonArrow,			257,41,	9,6,	1,'d',0,	1,5,3},
};

static PanelButton ITE_OptionPanelButtons[] = {
	{kPanelButtonOptionSlider,	284,19, 13,75,	0,'-',0,	0,0,0}, //slider-scroller
	{kPanelButtonOption,	113,18, 45,17,	kTextReadingSpeed,'r',0,	0,0,0}, //read speed
	{kPanelButtonOption,	113,37, 45,17,	kTextMusic,'m',0,	0,0,0}, //music
	{kPanelButtonOption,	113,56, 45,17,	kTextSound,'n',0,	0,0,0}, //sound-noise
	{kPanelButtonOption,	13,79, 135,17,	kTextQuitGame,'q',0,	0,0,0}, //quit
	{kPanelButtonOption,	13,98, 135,17,	kTextContinuePlaying,'c',0,	0,0,0}, //continue
	{kPanelButtonOption,	164,98, 57,17,	kTextLoad,'l',0,	0,0,0}, //load
	{kPanelButtonOption,	241,98, 57,17,	kTextSave,'s',0,	0,0,0},	//save
	{kPanelButtonOptionSaveFiles,	166,20, 112,74,	0,'-',0,	0,0,0},	//savefiles

	{kPanelButtonOptionText,106,4, 0,0,	kTextGameOptions,'-',0,	0,0,0},	// text: game options
	{kPanelButtonOptionText,11,22, 0,0,	kTextReadingSpeed,'-',0, 0,0,0},	// text: read speed
	{kPanelButtonOptionText,28,22, 0,0,	kTextShowDialog,'-',0, 0,0,0},	// text: read speed
	{kPanelButtonOptionText,73,41, 0,0,	kTextMusic,'-',0, 0,0,0},	// text: music
	{kPanelButtonOptionText,69,60, 0,0,	kTextSound,'-',0, 0,0,0},	// text: noise
};

static PanelButton ITE_QuitPanelButtons[] = {
	{kPanelButtonQuit, 11,17, 60,16, kTextQuit,'q',0, 0,0,0},
	{kPanelButtonQuit, 121,17, 60,16, kTextCancel,'c',0, 0,0,0},
	{kPanelButtonQuitText, -1,5, 0,0, kTextQuitTheGameQuestion,'-',0, 0,0,0},
};

static PanelButton ITE_LoadPanelButtons[] = {
	{kPanelButtonLoad, 101,19, 60,16, kTextOK,'o',0, 0,0,0},
	{kPanelButtonLoadText, -1,5, 0,0, kTextLoadSuccessful,'-',0, 0,0,0},
};

static PanelButton ITE_SavePanelButtons[] = {
	{kPanelButtonSave, 11,37, 60,16, kTextSave,'s',0, 0,0,0},
	{kPanelButtonSave, 101,37, 60,16, kTextCancel,'c',0, 0,0,0},
	{kPanelButtonSaveEdit, 26,17, 119,17, 0,'-',0, 0,0,0},
	{kPanelButtonSaveText, -1,5, 0,0, kTextEnterSaveGameName,'-',0, 0,0,0},
};

static PanelButton ITE_ProtectPanelButtons[] = {
	{kPanelButtonProtectEdit, 26,17, 119,17, 0,'-',0, 0,0,0},
	{kPanelButtonProtectText, -1,5, 0,0, kTextEnterProtectAnswer,'-',0, 0,0,0},
};

/*
static PanelButton ITE_ProtectionPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};*/

static const GameDisplayInfo ITE_DisplayInfo = {
	320, 200,		// logical width&height

	35,				// scene path y offset
	137,			// scene height

	0,				// status x offset
	137,			// status y offset
	320,			// status width
	11,				// status height
	2,				// status text y offset
	186,			// status text color
	15,				// status BG color
	308,137,		// save reminder pos
	12,12,			// save reminder w & h
	6,7,			// save reminder sprite numbers

	5, 4,			// left portrait x, y offset
	274, 4,			// right portrait x, y offset

	8, 9,			// inventory Up & Down button indexies
	2, 4,			// inventory rows, columns

	0, 148,			// main panel offsets
	ARRAYSIZE(ITE_MainPanelButtons),
	ITE_MainPanelButtons,

	ITE_CONVERSE_MAX_TEXT_WIDTH,
	ITE_CONVERSE_TEXT_HEIGHT,
	ITE_CONVERSE_TEXT_LINES,
	4, 5,			// converse Up & Down button indexies
	0, 148,			// converse panel offsets
	ARRAYSIZE(ITE_ConversePanelButtons),
	ITE_ConversePanelButtons,

	8, 0,			// save file index
	8,				// optionSaveFileVisible
	8, 8,			// option panel offsets
	ARRAYSIZE(ITE_OptionPanelButtons),
	ITE_OptionPanelButtons,

	64,54,			// quit panel offsets
	192,38,			// quit panel width & height
	ARRAYSIZE(ITE_QuitPanelButtons),
	ITE_QuitPanelButtons,

	74, 53,			// load panel offsets
	172, 40,		// load panel width & height
	ARRAYSIZE(ITE_LoadPanelButtons),
	ITE_LoadPanelButtons,

	2,				// save edit index
	74, 44,			// save panel offsets
	172, 58,		// save panel width & height
	ARRAYSIZE(ITE_SavePanelButtons),
	ITE_SavePanelButtons,

	0,				// protect edit index
	74, 44,			// protect panel offsets
	172, 58,		// protect panel width & height
	ARRAYSIZE(ITE_ProtectPanelButtons),
	ITE_ProtectPanelButtons
};


//TODO: ihnm
#define IHNM_CONVERSE_MAX_TEXT_WIDTH (256 - 60)
#define IHNM_CONVERSE_TEXT_HEIGHT       10
#define IHNM_CONVERSE_TEXT_LINES        10

static PanelButton IHNM_MainPanelButtons[] = {
	// TODO: The +2's are needed here to fix the verbs, investigate why
	// The computation of textid in Interface::drawVerbPanelText has also been changed accordingly
	{kPanelButtonVerb,		106,12,		114,30,	kVerbIHNMWalk + 2,'w',0,	0,1,0},
	{kPanelButtonVerb,		106,44,		114,30,	kVerbIHNMLookAt + 2,'l',0,	2,3,0},
	{kPanelButtonVerb,		106,76,		114,30, kVerbIHNMTake + 2,'k',0,	4,5,0},
	{kPanelButtonVerb,		106,108,	114,30, kVerbIHNMUse + 2,'u',0,		6,7,0},
	{kPanelButtonVerb,		223,12,		114,30, kVerbIHNMTalkTo + 2,'t',0,	8,9,0},
	{kPanelButtonVerb,		223,44,		114,30, kVerbIHNMSwallow + 2,'s',0,	10,11,0},
	{kPanelButtonVerb,		223,76,		114,30, kVerbIHNMGive + 2,'g',0,	12,13,0},
	{kPanelButtonVerb,		223,108,	114,30, kVerbIHNMPush + 2,'p',0,	14,15,0},
	{kPanelButtonArrow,		606,22,		20,25,	-1,'[',0,			0,0,0}, //TODO: arrow Sprite Numbers
	{kPanelButtonArrow,		606,108,	20,25,	1,']',0,			0,0,0},

	{kPanelButtonInventory,	357 + 64*0,18,	54,54,	0,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*1,18,	54,54,	1,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*2,18,	54,54,	2,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*3,18,	54,54,	3,'-',0,	0,0,0},

	{kPanelButtonInventory,	357 + 64*0,80,	54,54,	4,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*1,80,	54,54,	5,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*2,80,	54,54,	6,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*3,80,	54,54,	7,'-',0,	0,0,0}
};

static PanelButton IHNM_ConversePanelButtons[] = {
	{kPanelButtonConverseText,	117,18 + IHNM_CONVERSE_TEXT_HEIGHT * 0, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	0,'1',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 1, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	1,'2',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 2, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	2,'3',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 3, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	3,'4',0,	0,0,0},
	//.....
	{kPanelButtonArrow,			606,22,		20,25,	-1,'[',0,	0,0,0}, //TODO: arrow Sprite Numbers
	{kPanelButtonArrow,			606,108,	20,25,	1,']',0,	0,0,0}
};

static PanelButton IHNM_OptionPanelButtons[] = {
	//TODO: Add the rest of the buttons
	//TODO: Those coordinates might not be pixel perfect, check with the original interpreter
	{kPanelButtonOption,	20,150,	200,25,	kTextQuitGame,'q',0,	0,0,0}, //quit
	{kPanelButtonOption,	20,175,	200,25,	kTextContinuePlaying,'c',0,	0,0,0}, //continue
	//.....
};

static PanelButton IHNM_QuitPanelButtons[] = {
	//FIXME: Show the correct quit dialog background 
	//TODO: Those coordinates might not be pixel perfect, check with the original interpreter
	{kPanelButtonQuit,		25,80,	80,25,	kTextQuit,'q',0, 0,0,0},
	{kPanelButtonQuit,		155,80,	80,25,	kTextCancel,'c',0, 0,0,0},
	{kPanelButtonQuitText, -1,5,	0,0,	kTextQuitTheGameQuestion,'-',0, 0,0,0},
};

static PanelButton IHNM_LoadPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static PanelButton IHNM_SavePanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};


static const GameDisplayInfo IHNM_DisplayInfo = { //TODO: fill it all
	640, 480,	// logical width&height

	0,			// scene path y offset
	304,		// scene height

	0,			// status x offset
	304,		// status y offset
	616,		// status width
	24,			// status height
	8,			// status text y offset
	253,		// status text color
	250,		// status BG color
	616, 303,	// save reminder pos
	24, 24,		// save reminder w&h
	0,1,		// save reminder sprite numbers

	11, 12,		// left portrait x, y offset
	-1, -1,		// right portrait x, y offset

	-1, -1,		// inventory Up & Down button indexies
	2, 4,		// inventory rows, columns

	0, 328,		// main panel offsets
	ARRAYSIZE(IHNM_MainPanelButtons),
	IHNM_MainPanelButtons,

	-1, -1,		// converse Up & Down button indexies

	IHNM_CONVERSE_MAX_TEXT_WIDTH,
	IHNM_CONVERSE_TEXT_HEIGHT,
	IHNM_CONVERSE_TEXT_LINES,
	0, 328,		// converse panel offsets
	ARRAYSIZE(IHNM_ConversePanelButtons),
	IHNM_ConversePanelButtons,

	-1, -1,		// save file index
	0,			// optionSaveFileVisible
	100, 75,	// option panel offsets
	ARRAYSIZE(IHNM_OptionPanelButtons),
	IHNM_OptionPanelButtons,

	190,180,		// quit panel offsets
	260,115,		// quit panel width & height
	ARRAYSIZE(IHNM_QuitPanelButtons),
	IHNM_QuitPanelButtons,

	0, 0,			// load panel offsets
	0, 0,			// load panel width & height
	ARRAYSIZE(IHNM_LoadPanelButtons),
	IHNM_LoadPanelButtons,

	-1,				// save edit index
	0, 0,			// save panel offsets
	0, 0,			// save panel width & height
	ARRAYSIZE(IHNM_SavePanelButtons),
	IHNM_SavePanelButtons,

	// No protection panel in IHNM
	-1,				// protect edit index
	0, 0,			// protect panel offsets
	0, 0,			// protect panel width & height
	ARRAYSIZE(IHNM_SavePanelButtons),
	IHNM_SavePanelButtons
};

} // End of namespace Saga

#endif
