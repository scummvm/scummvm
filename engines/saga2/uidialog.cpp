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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/config-manager.h"
#include "audio/mixer.h"

#include "saga2/saga2.h"
#include "saga2/intrface.h"
#include "saga2/grequest.h"
#include "saga2/gtextbox.h"
#include "saga2/saveload.h"
#include "saga2/script.h"

#include "saga2/uidialog.h"
#include "saga2/document.h"
#include "saga2/tilemode.h"
#include "saga2/display.h"
#include "saga2/uitext.h"
#include "saga2/vpal.h"
#include "saga2/palette.h"

#include "saga2/fontlib.h"

namespace Saga2 {

// dialog functions
APPFUNC(cmdDialogQuit);
APPFUNCV(cmdFileSave);
APPFUNCV(cmdFileLoad);
APPFUNC(cmdSaveDialogUp);
APPFUNC(cmdSaveDialogDown);
APPFUNC(cmdTextResponse);
APPFUNC(cmdOptionsSaveGame);
APPFUNC(cmdOptionsLoadGame);
APPFUNC(cmdOptionsNewGame);
APPFUNC(cmdQuitGame);
APPFUNC(cmdCredits);
APPFUNC(cmdAutoAggression);
APPFUNC(cmdAutoWeapon);
APPFUNC(cmdNight);
APPFUNC(cmdSpeechText);

// volume control functions
APPFUNC(cmdSetMIDIVolume);
APPFUNC(cmdSetDIGVolume);
APPFUNC(cmdSetSpeechVolume);
APPFUNC(cmdSetSoundVolume);
APPFUNCV(cmdSaveVolumeSettings);

#define SmallFont Helv11Font

/* ===================================================================== *
   External declarations
 * ===================================================================== */
extern BackWindow       *mainWindow;
extern bool fullInitialized;

/* ===================================================================== *
   Dialog Controls
 * ===================================================================== */

// control pointers
gTextBox        *textBox;
CTextWindow     *editWin;


/* ===================================================================== *
   User interface dialog metrics
 * ===================================================================== */

enum fileProcessTypes {
	typeSave = 0,
	typeLoad
};

enum saveLoadImageResIDs {
	SLTopPanelResID = 0,
	SLMidPanelResID,
	SLBotPanelResID
};


enum optionsImageResIDs {
	optTopPanelResID = 0,
	optMidPanelResID,
	optBotPanelResID
};

enum messageImageResIDs {
	mesPanelResID = 0
};


// panels
static const StaticRect SLTopPanel = {
	kSLDBoxX,
	kSLDBoxY,
	kSLTPWidth,
	kSLTPHeight
};

static const StaticRect SLMidPanel = {
	kSLDBoxX,
	kSLDBoxY + kSLTPHeight,
	kSLMDWidth,
	kSLMDHeight
};

static const StaticRect SLBotPanel = {
	kSLDBoxX,
	kSLDBoxY + kSLTPHeight + kSLMDHeight,
	kSLBTWidth,
	kSLBTHeight
};


// buttons
static const StaticRect SLQuitBtnRect = {
	211,
	kSLTPHeight + kSLMDHeight + 11,
	122,
	30
};

static const StaticRect SLBtnRect = {
	31,
	kSLTPHeight + kSLMDHeight + 11,
	122,
	30
};

static const StaticRect SLUpArrowBtnRect = {
	327,
	46,
	32,
	36
};

static const StaticRect SLDnArrowBtnRect = {
	327,
	121,
	32,
	36
};

// texts

static const StaticRect SLTitleRect = {
	0,
	0,
	kSLDBoxXSzNS,
	47
};


// save load window rect

static const StaticRect saveLoadWindowRect = {
	kSLDBoxX,
	kSLDBoxY,
	kSLDBoxXSize,
	kSLDBoxYSize
};


// indirections

static const StaticRect *saveLoadButtonRects[kNumSaveLoadBtns] = {
	&SLQuitBtnRect,
	&SLBtnRect,
	&SLUpArrowBtnRect,
	&SLDnArrowBtnRect
};

static const StaticRect *saveLoadTextRects[kNumSaveLoadTexts] = {
	&SLTitleRect
};



// save/load dialog window decorations

static StaticWindow saveWindowDecorations[kNumSaveLoadPanels] = {
	{SLTopPanel, nullptr, SLTopPanelResID},
	{SLMidPanel, nullptr, SLMidPanelResID},
	{SLBotPanel, nullptr, SLBotPanelResID}
};



// panels

static const StaticRect optTopPanel = {
	kOptBoxX,
	kOptBoxY,
	kOptTPWidth,
	kOptTPHeight
};

static const StaticRect optMidPanel = {
	kOptBoxX,
	kOptBoxY + kOptTPHeight,
	kOptMDWidth,
	kOptMDHeight
};

static const StaticRect optBotPanel = {
	kOptBoxX,
	kOptBoxY + kOptTPHeight + kOptMDHeight,
	kOptBTWidth,
	kOptBTHeight
};


static const StaticRect optResumeRect = {
	kOptBoxXSzNS - (kPushButtonWidth + 14),
	kButtonYOffset + kButtonSpace,
	kPushButtonWidth,
	kPushButtonHeight
};

static const StaticRect optSaveRect = {
	kOptBoxXSzNS - (kPushButtonWidth + 14),
	kButtonYOffset + ((kPushButtonHeight * 1) + kButtonSpace * 2),
	kPushButtonWidth,
	kPushButtonHeight
};



static const StaticRect optRestoreRect = {
	kOptBoxXSzNS - (kPushButtonWidth + 14),
	kButtonYOffset + ((kPushButtonHeight * 2) + kButtonSpace * 3),
	kPushButtonWidth,
	kPushButtonHeight
};

static const StaticRect optQuitRect = {
	kOptBoxXSzNS - (kPushButtonWidth + 14),
	kButtonYOffset + ((kPushButtonHeight * 3) + kButtonSpace * 4),
	kPushButtonWidth,
	kPushButtonHeight
};

static const StaticRect optCreditsRect = {
	kOptBoxXSzNS - (kPushButtonWidth + 14),
	kButtonYOffset + ((kPushButtonHeight * 4) + kButtonSpace * 5),
	kPushButtonWidth,
	kPushButtonHeight
};

static const StaticRect optAggressRect = {
	14,
	98 + kOptTPHeight,
	18,
	17
};

static const StaticRect optWeaponRect = {
	14,
	121 + kOptTPHeight,
	18,
	17
};

static const StaticRect optSpeechRect = {
	14,
	121 + 23 + kOptTPHeight,
	18,
	17
};

static const StaticRect optNightRect = {
	14 + 200,
	98 + kOptTPHeight,
	18,
	17
};

static const StaticRect optTopSliderRect = {
	15,
	15 + kOptTPHeight - 2,
	kSliderWidth,
	kImageHeight
};

static const StaticRect optMidSliderRect = {
	15,
	(int16)(optTopSliderRect.y + 32 - 2),
	kSliderWidth,
	kImageHeight
};

static const StaticRect optBotSliderRect = {
	15,
	(int16)(optMidSliderRect.y + 32 - 2),
	kSliderWidth,
	kImageHeight
};

static const StaticRect optTopFaceRect = {
	optTopSliderRect.x,
	optTopSliderRect.y,
	28,
	kImageHeight
};

static const StaticRect optMidFaceRect = {
	optMidSliderRect.x,
	optMidSliderRect.y,
	28,
	kImageHeight
};

static const StaticRect optBotFaceRect = {
	optBotSliderRect.x,
	optBotSliderRect.y,
	28,
	kImageHeight
};


// texts

static const StaticRect optTitleText = {
	0,
	0,
	kOptBoxXSzNS,
	kOptTPHeight
};

static const StaticRect optTopSlideText = {
	16 + kSliderWidth,
	(int16)(optTopSliderRect.y + 1),
	kTextPixelLen,
	20
};

static const StaticRect optMidSlideText = {
	16 + kSliderWidth,
	(int16)(optMidSliderRect.y + 1),
	kTextPixelLen,
	17
};

static const StaticRect optBotSlideText = {
	16 + kSliderWidth,
	(int16)(optBotSliderRect.y + 1),
	kTextPixelLen,
	17
};

static const StaticRect optTopCheckText = {
	(int16)(optAggressRect.x + optAggressRect.width + 3),
	optAggressRect.y,
	kTextPixelLen - kSmallTextOffset,
	17
};

static const StaticRect optMidCheckText = {
	(int16)(optWeaponRect.x + optWeaponRect.width + 3),
	optWeaponRect.y,
	kTextPixelLen - kSmallTextOffset,
	17
};

static const StaticRect optBotCheckText = {
	(int16)(optSpeechRect.x  + optSpeechRect.width + 3),
	optSpeechRect.y,
	kTextPixelLen - kSmallTextOffset,
	17
};

static const StaticRect optTop2CheckText = {
	(int16)(optNightRect.x + optNightRect.width + 3),
	optNightRect.y,
	kTextPixelLen - kSmallTextOffset,
	17
};

// options window rect

static const StaticRect optionsWindowRect = {
	kOptBoxX,
	kOptBoxY,
	kOptBoxXSize,
	kOptBoxYSize
};


// indirections

static const StaticRect *optionsButtonRects[] = {
	&optResumeRect,
	&optSaveRect,
	&optRestoreRect,
	&optQuitRect,
	&optCreditsRect,
	&optAggressRect,
	&optWeaponRect,
	&optSpeechRect,
	&optNightRect
};

static const StaticRect *optionsTextRects[] = {
	&optTitleText,
	&optTopSlideText,
	&optMidSlideText,
	&optBotSlideText,
	&optTopCheckText,
	&optMidCheckText,
	&optBotCheckText,
	&optTop2CheckText
};


// options dialog window decorations

static StaticWindow optionsDecorations[kNumOptionsPanels] = {
	{optTopPanel, nullptr, optTopPanelResID},
	{optMidPanel, nullptr, optMidPanelResID},
	{optBotPanel, nullptr, optBotPanelResID}
};


// panels
static const StaticRect messagePanel = {
	kMesBoxX,
	kMesBoxY,
	kMesBoxXSize,
	kMesBoxYSize
};


// buttons
static const StaticRect mesCancelBtnRect = {
	kMesBoxXSzNS - (kPushButtonWidth + kMesBtnOffset),
	kMesBoxY - kMesBtnOffset,
	kPushButtonWidth,
	kPushButtonHeight
};

static const StaticRect mesOkBtnRect = {
	kMesBtnOffset,
	kMesBoxY - kMesBtnOffset,
	kPushButtonWidth,
	kPushButtonHeight
};


static const StaticRect mesBtn3Rect = {
	kMesBoxXSzNS / 2 - kPushButtonWidth / 2,
	kMesBoxY - kMesBtnOffset,
	kPushButtonWidth,
	kPushButtonHeight
};




// texts
static const StaticRect mesTitleRect = {
	0,
	0,
	kMesBoxXSzNS,
	47
};

static const StaticRect messageRect = {
	0,
	0,
	kMesBoxXSzNS,
	kMesBoxYSize
};



static const StaticRect *messageTextRects[kNumMessageTexts] = {
	&mesTitleRect,
	&messageRect
};


static const StaticRect *messageButtonRects[kNumMessageBtns] = {
	&mesOkBtnRect,
	&mesCancelBtnRect,
	&mesBtn3Rect
};



// options window rect
static const StaticRect messageWindowRect = {
	kMesBoxX,
	kMesBoxY,
	kMesBoxXSize,
	kMesBoxYSize
};




// message dialog window decorations
static StaticWindow messageDecorations[kNumMessagePanels] = {
	{messagePanel, nullptr, mesPanelResID}
};


// pointer to the auto aggression button
GfxOwnerSelCompButton *autoAggressBtn,
                    *autoWeaponBtn,
                    *nightBtn,
                    *speechTextBtn;



static int deferredLoadID = 0;
static bool deferredLoadFlag = false;
static bool deferredSaveFlag = false;
static char deferredSaveName[64];

inline bool isUserAction(gEvent ev) {
	return (ev.eventType == gEventNewValue) || (ev.eventType == gEventKeyDown);
}

/* ===================================================================== *
   Save game file name handling funtions
 * ===================================================================== */


char **initFileFields() {
	uint16              i;
	SaveFileHeader      header;                 //  The save file header.

	char **strings = new (char *[numEditLines]);

	for (i = 0; i < numEditLines; i++) {
		strings[i] = new char[editLen + 1];

		if (getSaveName(i, header)) {
			Common::strlcpy(strings[i], header.saveName.c_str(), editLen);
		} else {
			Common::strlcpy(strings[i], FILE_DIALOG_NONAME, editLen);
			strings[i][0] |= 0x80;
		}

		// make sure this thing is caped
		strings[i][editLen] = '\0';
	}

	return strings;
}

int numValid(char **names) {
	int v = 0;
	for (int i = 0; i < numEditLines; i++) {
		if ((names[i][0] & 0x80) == 0) v++;
	}
	return v;
}

void destroyFileFields(char **strings) {
	uint16  i;

	for (i = 0; i < numEditLines; i++) {
		if (strings[i])
			delete[] strings[i];
		strings[i] = nullptr;
	}

	delete[] strings;
}

bool getSaveName(int8 saveNo, SaveFileHeader &header) {
	Common::String fname = g_vm->getSavegameFile(saveNo);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fname);

	if (!in) {
		debugC(1, kDebugSaveload, "Unable to load save %d (%s)", saveNo, fname.c_str());
		return false;
	}

	header.read(in);

	delete in;

	return true;
}


/* ===================================================================== *
   Dialog boxes
 * ===================================================================== */

int16 FileDialog(int16 fileProcess) {
	//const   int strLen              = editLen;
	char    **fieldStrings;
	uint16  stringIndex;
	bool    displayOnly;
	void    **arrowUpIm = nullptr, **arrowDnIm = nullptr, **pushBtnIm = nullptr;

	AppFunc *fileCommands[2]  = { cmdFileSave, cmdFileLoad };

	// text for dialog
	const char    *saveTextStrings[kNumSaveLoadTexts]    = { SAVE_DIALOG_NAME };
	const char    *saveBtnStrings[kNumSaveLoadBtns]      = { SAVE_DIALOG_BUTTON1, SAVE_DIALOG_BUTTON2 };

	const char    *loadTextStrings[kNumSaveLoadTexts]    = { LOAD_DIALOG_NAME };
	const char    *loadBtnStrings[kNumSaveLoadBtns]      = { LOAD_DIALOG_BUTTON1, LOAD_DIALOG_BUTTON2 };

	const char    **textStrings[] = { saveTextStrings, loadTextStrings };
	const char    **btnStrings[]  = { saveBtnStrings, loadBtnStrings };


	// make the text coloring object
	textPallete     pal(33 + 9, 36 + 9, 41 + 9, 34 + 9, 40 + 9, 43 + 9);

	if (fileProcess == typeSave) {
		stringIndex = 0;
		displayOnly = false;
	} else {
		stringIndex = 1;
		displayOnly = true;
	}

	// resource info
	const   int16   dialogPushResNum    = 4;
	const   int16   upArrowResNum       = 0;
	const   int16   dnArrowResNum       = 2;

	// requester info struct
	requestInfo     rInfo;

	rInfo.result    = -1;
	rInfo.running   = true;

	// point to the modal window
	ModalWindow     *win;

	// resource handle
	hResContext     *decRes;

	// get the file fields names
	fieldStrings = initFileFields();


#ifndef ALLOW_BAD_LOADS
	if (displayOnly && numValid(fieldStrings) == 0) {
		destroyFileFields(fieldStrings);
		if (userDialog("Error", "No saved games to load!\n Would you like to start over?", "_Yes", "_No", nullptr) != 1) {
			deferredLoadFlag = true;
			deferredLoadID = 999;
			return typeLoad;
		}
		return 0;
	}
#endif
	// init the resource context handle
	decRes = resFile->newContext(dialogGroupID, "dialog resources");


	// get the graphics associated with the buttons
	pushBtnIm = loadButtonRes(decRes, dialogPushResNum, numBtnImages);
	arrowUpIm = loadButtonRes(decRes, upArrowResNum, numBtnImages);
	arrowDnIm = loadButtonRes(decRes, dnArrowResNum, numBtnImages);


	// create the window
	win = new ModalWindow(saveLoadWindowRect, 0, nullptr);

	// make the quit button
	new GfxCompButton(*win, *saveLoadButtonRects[0], pushBtnIm, numBtnImages, btnStrings[stringIndex][0], pal, 0, cmdDialogQuit);
	//t->accelKey=0x1B;

	// make the Save/Load button
	new GfxCompButton(*win, *saveLoadButtonRects[1], pushBtnIm, numBtnImages, btnStrings[stringIndex][1], pal, fileProcess, fileCommands[fileProcess]);
	//t->accelKey=0x0D;
	// make the up arrow
	new GfxCompButton(*win, *saveLoadButtonRects[2], arrowUpIm, numBtnImages, 0, cmdSaveDialogUp);
	//t->accelKey=33+0x80;
	// make the down arrow
	new GfxCompButton(*win, *saveLoadButtonRects[3], arrowDnIm, numBtnImages, 0, cmdSaveDialogDown);
	//t->accelKey=34+0x80;
	// attach the title
	new CPlaqText(*win, *saveLoadTextRects[0], textStrings[stringIndex][0], &Plate18Font, 0, pal, 0, nullptr);



	// attach the text box editing field object
	textBox          = new gTextBox(*win, editBaseRect, &Onyx10Font,
	        textHeight, textPen, textBackground, textHilite, textBackHilite, cursorColor,
	        nullptr, "Error out", fieldStrings, editLen, 0, (uint16) - 1, displayOnly, nullptr,
	        fileCommands[fileProcess], cmdDialogQuit);


	win->setDecorations(saveWindowDecorations,
	                    ARRAYSIZE(saveWindowDecorations),
	                    decRes, 'S', 'L', 'D');

	win->userData = &rInfo;
	win->open();

	if (GameMode::newmodeFlag)
		GameMode::update();

	win->invalidate();
	textBox->choose();

	EventLoop(rInfo.running, true);

	// remove the window all attatched controls
	delete win;
	win = nullptr;

	// unload all image arrays
	unloadImageRes(arrowUpIm, numBtnImages);
	unloadImageRes(arrowDnIm, numBtnImages);
	unloadImageRes(pushBtnIm, numBtnImages);


	// remove the resource handle
	if (decRes) resFile->disposeContext(decRes);
	decRes = nullptr;

	// destroy the file fields
	destroyFileFields(fieldStrings);

	// replace the damaged area
	mainWindow->invalidate(&saveLoadWindowRect);

	// return the result code
	return rInfo.result;
}


/* ===================================================================== *
   Options dialog box
 * ===================================================================== */
void updateMainDisplay();
void drawMainDisplay();
void fadeUp();
void fadeDown();
void clearTileAreaPort();
void displayUpdate();

int16 OptionsDialog(bool disableSaveResume) {
	// Save back buffer before opening the dialog
	g_vm->_renderer->saveBackBuffer(kBeforeOpeningMenu);

	// text for dialog
	const char  *btnStrings[kNumOptionsBtns] = {
		OPTN_DIALOG_BUTTON1,
		OPTN_DIALOG_BUTTON2,
		OPTN_DIALOG_BUTTON3,
		OPTN_DIALOG_BUTTON4,
		OPTN_DIALOG_BUTTON5
	};


	const char *textStrings[kNumOptionsTexts] = {
		OPTN_DIALOG_NAME,
		OPTN_DIALOG_SLIDE1,
		OPTN_DIALOG_SLIDE2,
		OPTN_DIALOG_SLIDE3,
		OPTN_DIALOG_CHECK1,
		OPTN_DIALOG_CHECK2,
		OPTN_DIALOG_CHECK3,
		OPTN_DIALOG_CHECK4
	};

	// make the text coloring object
	textPallete pal(33 + 9, 36 + 9, 41 + 9, 34 + 9, 40 + 9, 43 + 9);

	// requester info struct
	requestInfo     rInfo;

	rInfo.result    = -1;
	rInfo.running   = true;
	deferredLoadID = 0;
	deferredLoadFlag = false;
	deferredSaveFlag = false;
	deferredSaveName[0] = '\0';

	// point to the modal window
	ModalWindow     *win;


	// resource handle
	hResContext     *decRes;

	// resource info
	const   int16   dialogPushResNum    = 4;
	const   int16   checkResNum         = 6;
	const   int16   slideFaceResNum     = 8;
	const   int16   numSlideFace        = 4;

	// compressed image arrays
	void    **dialogPushImag;
	void    **checkImag;
	void    **slideFaceImag;
	if (!fullInitialized) return -1;

	// init the resource context handle
	decRes = resFile->newContext(dialogGroupID, "dialog resources");

	// get the graphics associated with the buttons
	dialogPushImag   = loadButtonRes(decRes, dialogPushResNum, numBtnImages);
	checkImag        = loadButtonRes(decRes, checkResNum, numBtnImages);
	slideFaceImag    = loadButtonRes(decRes, slideFaceResNum, numSlideFace);

	// create the window
	win = new ModalWindow(optionsWindowRect, 0, nullptr);
	GfxCompButton *t;

	// buttons
	if (!disableSaveResume) {
		t = new GfxCompButton(*win, *optionsButtonRects[0],
		                               dialogPushImag, numBtnImages, btnStrings[0], pal, 0, cmdDialogQuit);
		t->accelKey = 0x1B;

		t = new GfxCompButton(*win, *optionsButtonRects[1],
		                               dialogPushImag, numBtnImages, btnStrings[1], pal, 0, cmdOptionsSaveGame);    // make the quit button
		t->accelKey = 'S';
	} else {
		t = new GfxCompButton(*win, *optionsButtonRects[1],
		                               dialogPushImag, numBtnImages, OPTN_DIALOG_BUTTON6, pal, 0, cmdOptionsNewGame);
		t->accelKey = 'N';
	}

	t = new GfxCompButton(*win, *optionsButtonRects[2],
	                               dialogPushImag, numBtnImages, btnStrings[2], pal, 0, cmdOptionsLoadGame);    // make the quit button
	t->accelKey = 'L';

	t = new GfxCompButton(*win, *optionsButtonRects[3],
	                               dialogPushImag, numBtnImages, btnStrings[3], pal, 0, cmdQuitGame);
	t->accelKey = 'Q';

	t = new GfxCompButton(*win, *optionsButtonRects[4],
	                               dialogPushImag, numBtnImages, btnStrings[4], pal, 0, cmdCredits);
	t->accelKey = 'C';

	autoAggressBtn = new GfxOwnerSelCompButton(*win, *optionsButtonRects[5],
	        checkImag, numBtnImages, 0, cmdAutoAggression);
	autoAggressBtn->select(isAutoAggressionSet());

	autoWeaponBtn = new GfxOwnerSelCompButton(*win, *optionsButtonRects[6],
	        checkImag, numBtnImages, 0, cmdAutoWeapon);
	autoWeaponBtn->select(isAutoWeaponSet());

	speechTextBtn = new GfxOwnerSelCompButton(*win, *optionsButtonRects[7],
	        checkImag, numBtnImages, 0, cmdSpeechText);
	speechTextBtn->select(g_vm->_speechText);

	nightBtn = new GfxOwnerSelCompButton(*win, *optionsButtonRects[8],
	        checkImag, numBtnImages, 0, cmdNight);
	nightBtn->select(g_vm->_showNight);

	new GfxSlider(*win, optTopSliderRect, optTopFaceRect, 0,
	                       Audio::Mixer::kMaxMixerVolume, slideFaceImag, numSlideFace, ConfMan.getInt("sfx_volume"),
	                       0, cmdSetSoundVolume);

	new GfxSlider(*win, optMidSliderRect, optMidFaceRect, 0,
	                       Audio::Mixer::kMaxMixerVolume, slideFaceImag, numSlideFace, ConfMan.getInt("speech_volume"),
	                       0, cmdSetSpeechVolume);

	new GfxSlider(*win, optBotSliderRect, optBotFaceRect, 0,
	                       Audio::Mixer::kMaxMixerVolume, slideFaceImag, numSlideFace, ConfMan.getInt("music_volume"),
	                       0, cmdSetMIDIVolume);

	new CPlaqText(*win, *optionsTextRects[0],
	                         textStrings[0], &Plate18Font, 0, pal, 0, nullptr);

	for (int i = 1; i < kNumOptionsTexts; i++) {
		new CPlaqText(*win, *optionsTextRects[i], textStrings[i], &SmallFont, textPosLeft, pal, 0, nullptr);
	}

	win->setDecorations(optionsDecorations,
	                    ARRAYSIZE(optionsDecorations),
	                    decRes, 'O', 'P', 'T');


	win->userData = &rInfo;
	win->open();

	EventLoop(rInfo.running, true);

	g_vm->saveConfig();

	// remove the window all attatched controls
	delete win;
	win = nullptr;

	// unload all image arrays
	unloadImageRes(slideFaceImag,   numSlideFace);
	unloadImageRes(checkImag,       numBtnImages);
	unloadImageRes(dialogPushImag, numBtnImages);

	// remove the resource handle
	if (decRes) resFile->disposeContext(decRes);
	decRes = nullptr;

	// replace the damaged area

	if (deferredLoadFlag) {
		reDrawScreen();

		disableUserControls();
		cleanupGameState();

		fadeDown();

		if (deferredLoadID == 999)
			loadRestartGame();
		else {
			loadSavedGameState(deferredLoadID);
		}
		if (GameMode::newmodeFlag)
			GameMode::update();
		updateActiveRegions();
		//displayUpdate();
		enableUserControls();
		updateMainDisplay();
		drawMainDisplay();
		enablePaletteChanges();
		updateAllUserControls();
		fadeUp();
		reDrawScreen();
	} else {
		if (deferredSaveFlag) {
#ifdef IMMEDIATE_SAVE
			g_vm->saveGameState(deferredLoadID, deferredSaveName, false);
#endif
		}
		mainWindow->invalidate(&optionsWindowRect);
	}

	// Pop back buffer
	g_vm->_renderer->popSavedBackBuffer(kBeforeOpeningMenu);

	// return the result code
	return rInfo.result;
}

/* ===================================================================== *
   message dialog box
 * ===================================================================== */

char stripAccel(char *t, const char *s) {
	char accel = '\0';
	char    *underscore;

	if (t == nullptr || s == nullptr) return accel;
	strcpy(t, s);

	if ((underscore = strchr(t, '_')) != nullptr) {
		accel = toupper(underscore[1]);
		strcpy(underscore, s + (underscore - t) + 1);
	}
	return accel;
}




#ifdef DYNALOAD_USERDIALOG
// requester info struct
requestInfo     udrInfo;

// point to the modal window
ModalWindow     *udWin;

// resource handle
hResContext     *udDecRes;

// compressed image array
void    **udDialogPushImag;

bool    udInit = false;

bool initUserDialog() {

	const   int16   dialogPushResNum    = 4;
	// init the resource context handle
	udDecRes = resFile->newContext(dialogGroupID, "dialog resources");


	// get the graphics associated with the buttons
	udDialogPushImag = loadButtonRes(udDecRes, dialogPushResNum, numBtnImages);

	// create the window
	udWin = new ModalWindow(messageWindowRect, 0 nullptr);

	udWin->setDecorations(messageDecorations,
	                      ARRAYSIZE(messageDecorations),
	                      udDecRes, 'M', 'E', 'S');

	udWin->userData = &udrInfo;

	if (udDecRes) resFile->disposeContext(udDecRes);
	udDecRes = nullptr;

	udInit = true;

	return true;
}

bool userDialogAvailable() {
	return udInit;
}

void cleanupUserDialog() {
	udInit = false;
	// remove the window all attatched controls
	if (udWin) delete   udWin;
	udWin = nullptr;

	// unload all image arrays
	unloadImageRes(udDialogPushImag, numBtnImages);

}

int16 userDialog(const char *title, const char *msg, const char *bMsg1,
                 const char *bMsg2,
                 const char *bMsg3) {
	const   maxBtns = 3;
	uint8   numBtns = 0;

	char k1, k2, k3;

	char btnMsg1[32];
	char btnMsg2[32];
	char btnMsg3[32];

	if (bMsg1 && strlen(bMsg1)) numBtns++;
	if (bMsg2 && strlen(bMsg2)) numBtns++;
	if (bMsg3 && strlen(bMsg3)) numBtns++;

	k1 = stripAccel(btnMsg1, bMsg1);
	k2 = stripAccel(btnMsg2, bMsg2);
	k3 = stripAccel(btnMsg3, bMsg3);


	// make the text coloring object
	textPallete     pal(33 + 9, 36 + 9, 41 + 9, 34 + 9, 40 + 9, 43 + 9);

	if (udWin == nullptr) return -1;

	udrInfo.result  = -1;
	udrInfo.running = true;

	GfxCompButton *t;

	// button one
	if (numBtns >= 1) {
		t = new GfxCompButton(*udWin, messageButtonRects[0],
		                               udDialogPushImag, numBtnImages, btnMsg1, pal, 10, cmdDialogQuit);
		t->accel = k1;
	}

	// button two
	if (numBtns >= 2) {
		t = new GfxCompButton(*udWin, messageButtonRects[1],
		                               udDialogPushImag, numBtnImages, btnMsg2, pal, 11, cmdDialogQuit);
		t->accel = k2;
	}

	// button three
	if (numBtns >= 3) {
		t = new GfxCompButton(*udWin, messageButtonRects[2],
		                               udDialogPushImag, numBtnImages, btnMsg3, pal, 12, cmdDialogQuit);
		t->accel = k3;
	}

	// title for the box
	new CPlaqText(*udWin, messageTextRects[0], title, &Plate18Font, nullptr, pal, 0, nullptr);

	// message for box
	new CPlacardPanel(*udWin, messageTextRects[1], msg, &Onyx10Font, nullptr, pal, 0, nullptr);


	udWin->open();

	EventLoop(udrInfo.running, true);

	udWin->close();

	udWin->removeControls();

	// replace the damaged area
	mainWindow->invalidate(messageWindowRect);

	// return the result code
	return (udrInfo.result % 10);
}

#else

/* ===================================================================== *
   message dialog box
 * ===================================================================== */

bool initUserDialog() {
	return true;
}
bool userDialogAvailable() {
	return true;
}
void cleanupUserDialog() {}

int16 userDialog(const char *title, const char *msg, const char *bMsg1,
                 const char *bMsg2,
                 const char *bMsg3) {
	//const   int maxBtns = 3;
	uint8   numBtns = 0;

	char k1, k2, k3;

	char btnMsg1[32];
	char btnMsg2[32];
	char btnMsg3[32];

	if (bMsg1 && strlen(bMsg1)) numBtns++;
	if (bMsg2 && strlen(bMsg2)) numBtns++;
	if (bMsg3 && strlen(bMsg3)) numBtns++;

	k1 = stripAccel(btnMsg1, bMsg1);
	k2 = stripAccel(btnMsg2, bMsg2);
	k3 = stripAccel(btnMsg3, bMsg3);


	// make the text coloring object
	textPallete     pal(33 + 9, 36 + 9, 41 + 9, 34 + 9, 40 + 9, 43 + 9);

	// resource info
	const   int16   dialogPushResNum    = 4;

	// requester info struct
	requestInfo     rInfo;

	// point to the modal window
	ModalWindow     *win;


	// resource handle
	hResContext     *decRes;

	// compressed image array
	void    **dialogPushImag;

	rInfo.result    = -1;
	rInfo.running   = true;

	if (!fullInitialized)
		return -1;

	// init the resource context handle
	decRes = resFile->newContext(dialogGroupID, "dialog resources");


	// get the graphics associated with the buttons
	dialogPushImag   = loadButtonRes(decRes, dialogPushResNum, numBtnImages);

	// create the window
	win = new ModalWindow(messageWindowRect, 0, nullptr);

	GfxCompButton *t;

	// button one
	if (numBtns >= 1) {
		t = new GfxCompButton(*win, *messageButtonRects[0],
		                               dialogPushImag, numBtnImages, btnMsg1, pal, 10, cmdDialogQuit);
		t->accelKey = k1;
	}

	// button two
	if (numBtns >= 2) {
		t = new GfxCompButton(*win, *messageButtonRects[1],
		                               dialogPushImag, numBtnImages, btnMsg2, pal, 11, cmdDialogQuit);
		t->accelKey = k2;
	}

	// button three
	if (numBtns >= 3) {
		t = new GfxCompButton(*win, *messageButtonRects[2],
		                               dialogPushImag, numBtnImages, btnMsg3, pal, 12, cmdDialogQuit);
		t->accelKey = k3;
	}

	// title for the box
	new CPlaqText(*win, *messageTextRects[0], title, &Plate18Font, 0, pal, 0, nullptr);

	// message for box
	new CPlacardPanel(*win, *messageTextRects[1], msg, &Onyx10Font, 0, pal, 0, nullptr);

	win->setDecorations(messageDecorations,
	                    ARRAYSIZE(messageDecorations),
	                    decRes, 'M', 'E', 'S');


	win->userData = &rInfo;
	win->open();


	EventLoop(rInfo.running, true);


	// remove the window all attatched controls
	delete  win;

	// unload all image arrays
	unloadImageRes(dialogPushImag, numBtnImages);

	// remove the resource handle
	if (decRes) resFile->disposeContext(decRes);
	decRes = nullptr;

	// replace the damaged area
	mainWindow->invalidate(&messageWindowRect);

	// return the result code
	return rInfo.result % 10;
}

#endif

/* ===================================================================== *
   Placard thingus
 * ===================================================================== */

CPlacardWindow::CPlacardWindow(
    const Rect16 &r,
    uint16 ident,
    AppFunc *cmd,
    char *windowText,
    textPallete &pal,
    gFont *font) :
	ModalWindow(r, ident, cmd) {
	textPal = pal;
	textFont = font;

	positionText(windowText, Rect16(0, 0, r.width, r.height));
}

void CPlacardWindow::positionText(
    char *windowText,
    const Rect16 &textArea) {
	if (windowText) {
		int16   i,
		        yPos,
		        maxY;

		int16   fontHeight = textFont->height;

		// make a copy of the window text string
		sprintf(titleBuf, "%s", windowText);

		//  break up the title text string
		titleCount = SplitString(titleBuf, titleStrings, maxLines, '\n');

		yPos = textArea.y +
		       ((textArea.height - titleCount * fontHeight) >> 1);
		yPos = MAX(yPos, textArea.y);

		maxY = textArea.y + textArea.height - fontHeight;

		for (i = 0; i < titleCount; i++, yPos += fontHeight) {
			if (yPos < maxY) {
				titlePos[i].y = yPos;
				titlePos[i].x =
				    textArea.x +
				    ((textArea.width -
				      TextWidth(textFont, titleStrings[i], -1, 0))
				     >> 1);
			} else titleCount = i;
		}
	} else titleCount = 0;
}

int16 CPlacardWindow:: SplitString(
    char            *text,
    char            *textStart[],
    int16           maxStrings,
    char            delimiter) {
	int16           count;

	for (count = 0; count < maxStrings;) {
		textStart[count++] = text;
		if ((text = strchr(text, delimiter)) == nullptr) break;
		*text++ = '\0';
	}
	return count;
}


// just exit if the user hit the screen.
bool CPlacardWindow::pointerHit(gPanelMessage &) {
	gWindow         *win;
	requestInfo     *ri;

	win = getWindow();      // get the window pointer
	ri = win ? (requestInfo *)win->userData : nullptr;

	if (ri) {
		ri->running = 0;
		ri->result  = id;
	}

	//activate( gEventMouseDown );
	return true;
}

void CPlacardWindow::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &r) {
	if (!_extent.overlap(r)) return;

	// do background drawing first...
	ModalWindow::drawClipped(port, offset, r);

	int16   i;
	Point16 origin;
	Rect16  rect;

	SAVE_GPORT_STATE(port);

	origin.x = _extent.x - offset.x;
	origin.y = _extent.y - offset.y;

	rect.x = origin.x;
	rect.y = origin.y;
	rect.width  = _extent.width;
	rect.height = _extent.height;

	for (i = 0; i < titleCount; i++) {
		Point16 textPos     = origin + titlePos[i];

		writePlaqTextPos(port,
		                 textPos,
		                 textFont,
		                 0,
		                 textPal,
		                 false,
		                 titleStrings[i]);
	}
}

CPlacardPanel::CPlacardPanel(gPanelList &gpl, const Rect16 &r, const char *t, gFont *f,
                             int16 i, textPallete &p, int16 i2, AppFunc *cmd) :
	CPlaqText(gpl, r, t, f, i, p, i2, cmd) {

	positionText(t, Rect16(0, 0, r.width, r.height));
}



void CPlacardPanel::positionText(const char *windowText, const Rect16 &textArea) {
	if (windowText) {
		int16   i,
		        yPos,
		        maxY;

		int16   fontHeight = buttonFont->height;

		// make a copy of the window text string
		sprintf(titleBuf, "%s", windowText);

		//  break up the title text string
		titleCount = SplitString(titleBuf, titleStrings, maxLines, '\n');

		yPos = textArea.y +
		       ((textArea.height - titleCount * fontHeight) >> 1);
		yPos = MAX(yPos, textArea.y);

		maxY = textArea.y + textArea.height - fontHeight;

		for (i = 0; i < titleCount; i++, yPos += fontHeight) {
			if (yPos < maxY) {
				titlePos[i].y = yPos;
				titlePos[i].x =
				    textArea.x +
				    ((textArea.width -
				      TextWidth(buttonFont, titleStrings[i], -1, 0))
				     >> 1);
			} else titleCount = i;
		}
	} else titleCount = 0;
}

int16 CPlacardPanel:: SplitString(
    char            *text,
    char            *textStart[],
    int16           maxStrings,
    char            delimiter) {
	int16           count;

	for (count = 0; count < maxStrings;) {
		textStart[count++] = text;
		if ((text = strchr(text, delimiter)) == nullptr) break;
		*text++ = '\0';
	}
	return count;
}

void CPlacardPanel::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &r) {
	if (!_extent.overlap(r)) return;

	// do background drawing first...
	int16   i;
	Point16 origin;
	Rect16  rect;

	SAVE_GPORT_STATE(port);

	origin.x = _extent.x - offset.x;
	origin.y = _extent.y - offset.y;

	rect.x = origin.x;
	rect.y = origin.y;
	rect.width  = _extent.width;
	rect.height = _extent.height;

	for (i = 0; i < titleCount; i++) {
		Point16 textPos     = origin + titlePos[i];

		writePlaqTextPos(port,
		                 textPos,
		                 buttonFont,
		                 0,
		                 textFacePal,
		                 false,
		                 titleStrings[i]);
	}
}


void placardWindow(int8 type, char *text) {

	Rect16 plaqRectWood = Rect16((640 - 238) / 2,
	                             (480 - 145) / 3,
	                             238,
	                             145);

	Rect16 plaqRectStone = Rect16((640 - 236) / 2,
	                              (480 - 143) / 3,
	                              236,
	                              143);

	Rect16 plaqRectBrass = Rect16((640 - 274) / 2,
	                              (480 - 145) / 3,
	                              274,
	                              145);

	// decoration information
	WindowDecoration plaqDecWood[1] = {
		WindowDecoration(plaqRectWood, 0)
	};

	WindowDecoration plaqDecStone[1] = {
		WindowDecoration(plaqRectStone, 1)
	};

	WindowDecoration plaqDecBrass[2] = {
		WindowDecoration(plaqRectBrass, 2),
		WindowDecoration(plaqRectBrass, 2)
	};

	// used to hold the coloration of the text for a give type
	textPallete     pal;

	// requester info struct
	requestInfo     rInfo;

	rInfo.result    = -1;
	rInfo.running   = true;

	// point to the modal window
	CPlacardWindow      *win;

	// resource handle
	hResContext     *resContext;

	// init the resource context handle
	resContext = resFile->newContext(MKTAG('I', 'M', 'A', 'G'), "Placard resources");


	// do type related assignments
	switch (type) {
	case WOOD_TYPE:

		// set wood text inlay color
		pal.set(62, 64, 67, 11, 23, 17);

		// create the window
		win = new CPlacardWindow(plaqRectWood, 0, nullptr, text, pal, &Plate18Font);

		// setup the background imagery
		win->setDecorations(plaqDecWood,
		                    ARRAYSIZE(plaqDecWood),
		                    resContext, 'P', 'L', 'Q');

		break;

	case STONE_TYPE:

		// set stone text inlay color
		pal.set(16, 12, 18, 11, 23, 0x78);

		// create the window
		win = new CPlacardWindow(plaqRectStone, 0, nullptr, text, pal, &Plate18Font);

		// setup the background imagery
		win->setDecorations(plaqDecStone,
		                    ARRAYSIZE(plaqDecStone),
		                    resContext, 'P', 'L', 'Q');

		break;

	case BRASS_TYPE:

		// set brass text inlay color
		pal.set(89, 93, 95, 11, 23, 0x76);

		// create the window
		win = new CPlacardWindow(plaqRectBrass, 0, nullptr, text, pal, &Plate18Font);

		// setup the background imagery
		win->setDecorations(plaqDecBrass,
		                    ARRAYSIZE(plaqDecBrass),
		                    resContext, 'P', 'L', 'Q');

		break;

	default:
		error("Unhandled placard type %d", type);
		break;
	}


	win->userData = &rInfo;
	win->open();


	EventLoop(rInfo.running, true);


	// remove the window all attatched controls
	delete  win;

	// remove the resource handle
	if (resContext) resFile->disposeContext(resContext);

	// replace the damaged area
	mainWindow->invalidate(&plaqRectBrass);   // brass just happens to be the largest rect....

	// return the result code
	//return rInfo.result;
}


void updateAutoAggressionButton(bool setting) {
	if (autoAggressBtn != nullptr)
		autoAggressBtn->select(setting);
}

void updateAutoWeaponButton(bool setting) {
	if (autoWeaponBtn != nullptr)
		autoWeaponBtn->select(setting);
}

// dialog appfuncs
APPFUNC(cmdDialogQuit) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && isUserAction(ev) && ev.value) {
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->userData : nullptr;

		if (ri) {
			ri->running = 0;
			ri->result = ev.panel->id;
		}
	}
}

APPFUNC(cmdFileSave) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && isUserAction(ev) && ev.value) {
		// now close the window
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->userData : nullptr;

		if (ri) {
			ri->running = 0;
			ri->result  = typeSave; //ev.panel->id;
		}

		textBox->keepChanges();

		// get index of the game
		int8    saveIndex = textBox->getIndex();

#ifndef IMMEDIATE_SAVE
		// save game
		g_vm->saveGameState(saveIndex, textBox->getLine(saveIndex), false);
#else
		deferredLoadID = saveIndex;
		deferredSaveFlag = true;
		strcpy(deferredSaveName, textBox->getLine(saveIndex));
#endif
	}
}

APPFUNC(cmdFileLoad) {
	gWindow         *win;
	requestInfo     *ri;
	SaveFileHeader  header;
	int         saveNo;

	if (ev.panel && isUserAction(ev) && ev.value) {
		// get the file index
		saveNo = textBox->getIndex();

		if (getSaveName(saveNo, header)) {

			// close window
			win = ev.panel->getWindow();        // get the window pointer
			ri = win ? (requestInfo *)win->userData : nullptr;

			if (ri) {
				ri->running = 0;
				ri->result = typeLoad; //ev.panel->id;
			}

			deferredLoadID = saveNo;
			deferredLoadFlag = true;
		}
	}
}

APPFUNC(cmdSaveDialogUp) {
	if (ev.panel && isUserAction(ev) && ev.value) {
		if (textBox) {
			textBox->scrollUp();
		}
	}
}

APPFUNC(cmdSaveDialogDown) {
	if (ev.panel && isUserAction(ev) && ev.value) {
		if (textBox) {
			textBox->scrollDown();
		}
	}
}

APPFUNCV(cmdTextResponse) {
}

APPFUNC(cmdOptionsSaveGame) {
	if (ev.panel && isUserAction(ev) && ev.value) {
		FileDialog(typeSave);
	}
}

APPFUNC(cmdOptionsNewGame) {
	if (ev.panel && isUserAction(ev) && ev.value) {
		gWindow         *win;
		requestInfo     *ri;
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->userData : nullptr;

		if (ri) {
			ri->running = 0;
			ri->result = ev.panel->id;
			deferredLoadID = 999;
			deferredLoadFlag = true;
		}
	}
}

APPFUNC(cmdOptionsLoadGame) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && isUserAction(ev) && ev.value) {

		// if the fileDialog actually did loading
		if (FileDialog(typeLoad) == typeLoad) {
			win = ev.panel->getWindow();        // get the window pointer
			ri = win ? (requestInfo *)win->userData : nullptr;

			if (ri) {
				ri->running = 0;
				ri->result = ev.panel->id;
			}
		}
	}
}

APPFUNC(cmdQuitGame) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && isUserAction(ev) && ev.value) {
		win = ev.panel->getWindow();        // get the window pointer
		ri  = win ? (requestInfo *)win->userData : nullptr;

		if (ri
		        &&  userDialog(
		            VFYX_DIALOG_NAME,
		            VFYX_DIALOG_CAPTION,
		            VFYX_DIALOG_BUTTON1,
		            VFYX_DIALOG_BUTTON2, nullptr) == 0) {
			endGame();

			ri->running = false;
			ri->result = ev.panel->id;
		}
	}

}

inline int16 quantizedVolume(uint16 trueVolume) {
	int16 quantized = trueVolume & 0xFFF8;
	quantized += (quantized / 16);

	quantized = CLIP(quantized, (int16)0, (int16)255);

	return quantized;
}



APPFUNC(cmdCredits) {
	if (ev.panel && isUserAction(ev) && ev.value) {
		//reDrawScreen();
		openBook(resImports->reserved[0]);
	}
}

APPFUNC(cmdAutoAggression) {
	if (isUserAction(ev)) {
		toggleAutoAggression();
	}
}

APPFUNC(cmdAutoWeapon) {
	if (isUserAction(ev)) {
		toggleAutoWeapon();
	}
}

APPFUNC(cmdNight) {
	if (isUserAction(ev)) {
		g_vm->_showNight = !g_vm->_showNight;
		nightBtn->select(g_vm->_showNight);
	}
}

APPFUNC(cmdSpeechText) {
	if (isUserAction(ev)) {
		g_vm->_speechText = !g_vm->_speechText;
		speechTextBtn->select(g_vm->_speechText);

		ConfMan.setBool("subtitles", g_vm->_speechText);
	}
}

void volumeChanged();

// Set music volume

APPFUNC(cmdSetMIDIVolume) {
	int16 v = quantizedVolume(ev.value);
	ConfMan.setInt("music_volume", v);
	g_vm->syncSoundSettings();
	volumeChanged();
}

// Set DIG volume for DINO

APPFUNC(cmdSetDIGVolume) {
	int16 v = quantizedVolume(ev.value);
	ConfMan.setInt("music_volume", v);
	g_vm->syncSoundSettings();
	volumeChanged();
}

// Set DIG speech volume for FTA

APPFUNC(cmdSetSpeechVolume) {
	int16 v = quantizedVolume(ev.value);
	ConfMan.setInt("speech_volume", v);
	g_vm->syncSoundSettings();
	volumeChanged();
}

// Set DIG sound volume for FTA

APPFUNC(cmdSetSoundVolume) {
	int16 v = quantizedVolume(ev.value);
	ConfMan.setInt("sfx_volume", v);
	g_vm->syncSoundSettings();
	volumeChanged();
}

// Save volume settings
//   This should be called when exiting the dialog to save the changes

APPFUNCV(cmdSaveVolumeSettings) {
	g_vm->saveConfig();
}

} // end of namespace Saga2
