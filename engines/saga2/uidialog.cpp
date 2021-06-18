/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/intrface.h"
#include "saga2/grequest.h"
#include "saga2/gtextbox.h"
#include "saga2/loadsave.h"
#include "saga2/script.h"
#include "saga2/audio.h"

#include "saga2/queues.h"
#include "saga2/audiosmp.h"
#include "saga2/audqueue.h"
#include "saga2/audiosys.h"

#include "saga2/uidialog.h"
#include "saga2/document.h"
#include "saga2/tilemode.h"
#include "saga2/display.h"
#include "saga2/uitext.h"
#include "saga2/vpal.h"
#include "saga2/palette.h"

#include "saga2/config.h"
#include "saga2/fontlib.h"
#include "saga2/savefile.h"

namespace Saga2 {

#define PROGRAM_ABOUT   PROGRAM_FULL_NAME "\r\n\r\n"\
	" Version " VERSION_STAMP "\r\n"


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
extern audioInterface   *audio;
extern configuration    globalConfig;
extern bool fullInitialized;

/* ===================================================================== *
   External functions
 * ===================================================================== */

void getSaveFileName(int16 saveNo, char *fileName);


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

// Save/Load dialog metrics

const   int numSaveLoadPanels   = 3;
const   int numSaveLoadBtns     = 4;
const   int numSaveLoadTexts    = 1;

const   int16       SLDBoxXSize = 374,
                    SLDBoxXSzNS = 366,
                    SLDBoxYSize = 223,
                    SLDBoxX     = (640 - SLDBoxXSize) / 2,
                    SLDBoxY     = (480 - SLDBoxYSize) / 3;

const   int16       SLTPHeight = 38;
const   int16       SLMDHeight = 122;
const   int16       SLBTHeight = 63;
const   int16       SLTPWidth  = 374;
const   int16       SLMDWidth  = 374;
const   int16       SLBTWidth  = 374;



// panels
Rect16  SLTopPanel(SLDBoxX,
                   SLDBoxY,
                   SLTPWidth,
                   SLTPHeight);

Rect16  SLMidPanel(SLDBoxX,
                   SLDBoxY + SLTPHeight,
                   SLMDWidth,
                   SLMDHeight);

Rect16  SLBotPanel(SLDBoxX,
                   SLDBoxY + SLTPHeight + SLMDHeight,
                   SLBTWidth,
                   SLBTHeight);


// buttons
Rect16  SLQuitBtnRect(211,
                      SLTPHeight + SLMDHeight + 11,
                      122,
                      30);

Rect16  SLBtnRect(31,
                  SLTPHeight + SLMDHeight + 11,
                  122,
                  30);

Rect16  SLUpArrowBtnRect(327,
                         46,
                         32,
                         36);

Rect16  SLDnArrowBtnRect(327,
                         121,
                         32,
                         36);

// texts

Rect16  SLTitleRect(0,
                    0,
                    SLDBoxXSzNS,
                    47);


// save load window rect

Rect16  saveLoadWindowRect =    Rect16(SLDBoxX,
                                       SLDBoxY,
                                       SLDBoxXSize,
                                       SLDBoxYSize);


// indirections

Rect16  saveLoadPanelRects[numSaveLoadPanels] = { { SLTopPanel },
	{ SLMidPanel },
	{ SLBotPanel }
};

Rect16  saveLoadButtonRects[numSaveLoadBtns]  = { { SLQuitBtnRect },
	{ SLBtnRect     },
	{ SLUpArrowBtnRect },
	{ SLDnArrowBtnRect }
};

Rect16  saveLoadTextRects[numSaveLoadTexts]   = { { SLTitleRect } };



// save/load dialog window decorations

WindowDecoration saveWindowDecorations[numSaveLoadPanels] =

{	{ WindowDecoration(saveLoadPanelRects[0], SLTopPanelResID) },
	{ WindowDecoration(saveLoadPanelRects[1], SLMidPanelResID) },
	{ WindowDecoration(saveLoadPanelRects[2], SLBotPanelResID) }
};


// Options dialog metrics

const   int numOptionsPanels    = 3;
const   int numOptionsBtns      = 9;
const   int numOptionsTexts     = 8;


const   int16       optBoxXSize = 487,
                    optBoxXSzNS = 479,
                    optBoxYSize = 230,
                    optBoxX     = (640 - optBoxXSize) / 2,
                    optBoxY     = (480 - optBoxYSize) / 3;

const   int16       optTPHeight = 39;
const   int16       optMDHeight = 90;
const   int16       optBTHeight = 101;
const   int16       optTPWidth  = 487;
const   int16       optMDWidth  = 487;
const   int16       optBTWidth  = 487;



// panels

Rect16  optTopPanel(optBoxX,
                    optBoxY,
                    optTPWidth,
                    optTPHeight);

Rect16  optMidPanel(optBoxX,
                    optBoxY + optTPHeight,
                    optMDWidth,
                    optMDHeight);

Rect16  optBotPanel(optBoxX,
                    optBoxY + optTPHeight + optMDHeight,
                    optBTWidth,
                    optBTHeight);


// buttons
const   int buttonSpace         =   3;
const   int buttonYOffset       =   optTPHeight + 7;
const   int pushButtonWidth     =   121;
const   int pushButtonHeight    =   30;

const   int sliderWidth         =   168;
//const   int sliderHeight        =   15;
const   int imageHeight         =   17;

const   int textPixelLen        =   175;
const   int smallTextOffset     =    80;


Rect16  optResumeRect(optBoxXSzNS - (pushButtonWidth + 14),
                      buttonYOffset + buttonSpace,
                      pushButtonWidth,
                      pushButtonHeight);

Rect16  optSaveRect(optBoxXSzNS - (pushButtonWidth + 14),
                    buttonYOffset + ((pushButtonHeight * 1) + buttonSpace * 2),
                    pushButtonWidth,
                    pushButtonHeight);

Rect16  optRestoreRect(optBoxXSzNS - (pushButtonWidth + 14),
                       buttonYOffset + ((pushButtonHeight * 2) + buttonSpace * 3),
                       pushButtonWidth,
                       pushButtonHeight);

Rect16  optQuitRect(optBoxXSzNS - (pushButtonWidth + 14),
                    buttonYOffset + ((pushButtonHeight * 3) + buttonSpace * 4),
                    pushButtonWidth,
                    pushButtonHeight);

Rect16  optCreditsRect(optBoxXSzNS - (pushButtonWidth + 14),
                       buttonYOffset + ((pushButtonHeight * 4) + buttonSpace * 5),
                       pushButtonWidth,
                       pushButtonHeight);

Rect16  optAggressRect(14,
                       98 + optTPHeight,
                       18,
                       17);

Rect16  optWeaponRect(14,
                      121 + optTPHeight,
                      18,
                      17);

Rect16  optSpeechRect(14,
                      121 + 23 + optTPHeight,
                      18,
                      17);

Rect16  optNightRect(14 + 200,
                     98 + optTPHeight,
                     18,
                     17);

Rect16  optTopSliderRect(15,
                         15 + optTPHeight - 2,
                         sliderWidth,
                         imageHeight);

Rect16  optMidSliderRect(15,
                         optTopSliderRect.y + 32 - 2,
                         sliderWidth,
                         imageHeight);

Rect16  optBotSliderRect(15,
                         optMidSliderRect.y + 32 - 2,
                         sliderWidth,
                         imageHeight);

Rect16  optTopFaceRect(optTopSliderRect.x,
                       optTopSliderRect.y,
                       28,
                       imageHeight);

Rect16  optMidFaceRect(optMidSliderRect.x,
                       optMidSliderRect.y,
                       28,
                       imageHeight);

Rect16  optBotFaceRect(optBotSliderRect.x,
                       optBotSliderRect.y,
                       28,
                       imageHeight);


// texts

Rect16  optTitleText(0,
                     0,
                     optBoxXSzNS,
                     optTPHeight);

Rect16  optTopSlideText(16 + sliderWidth,
                        optTopSliderRect.y + 1,
                        textPixelLen,
                        20);

Rect16  optMidSlideText(16 + sliderWidth,
                        optMidSliderRect.y + 1,
                        textPixelLen,
                        17);

Rect16  optBotSlideText(16 + sliderWidth,
                        optBotSliderRect.y + 1,
                        textPixelLen,
                        17);

Rect16  optTopCheckText(optAggressRect.x + optAggressRect.width + 3,
                        optAggressRect.y,
                        textPixelLen - smallTextOffset,
                        17);

Rect16  optMidCheckText(optWeaponRect.x + optWeaponRect.width + 3,
                        optWeaponRect.y,
                        textPixelLen - smallTextOffset,
                        17);

Rect16  optBotCheckText(optSpeechRect.x  + optSpeechRect.width + 3,
                        optSpeechRect.y,
                        textPixelLen - smallTextOffset,
                        17);

Rect16  optTop2CheckText(optNightRect.x + optNightRect.width + 3,
                         optNightRect.y,
                         textPixelLen - smallTextOffset,
                         17);

// options window rect

Rect16  optionsWindowRect =     Rect16(optBoxX,
                                       optBoxY,
                                       optBoxXSize,
                                       optBoxYSize);


// indirections

Rect16  optionsPanelRects[numOptionsPanels]       = { { optTopPanel },
	{ optMidPanel },
	{ optBotPanel }
};

Rect16  optionsButtonRects[]                        = { { optResumeRect },
	{ optSaveRect },
	{ optRestoreRect },
	{ optQuitRect },
	{ optCreditsRect },
	{ optAggressRect },
	{ optWeaponRect },
	{ optSpeechRect },
	{ optNightRect }
};

Rect16  optionsTextRects[]                          = { { optTitleText },
	{ optTopSlideText },
	{ optMidSlideText },
	{ optBotSlideText },
	{ optTopCheckText },
	{ optMidCheckText },
	{ optBotCheckText },
	{ optTop2CheckText }
};


// options dialog window decorations

WindowDecoration optionsDecorations[numOptionsPanels] =

{	{ WindowDecoration(optionsPanelRects[0], optTopPanelResID) },
	{ WindowDecoration(optionsPanelRects[1], optMidPanelResID) },
	{ WindowDecoration(optionsPanelRects[2], optBotPanelResID) }
};


// Message Dialog Metrics

const   int numMessagePanels    = 1;
const   int numMessageBtns      = 3;
const   int numMessageTexts     = 2;
const   int mesBtnOffset        = 14;

const   int16       mesBoxXSize = 374,
                    mesBoxXSzNS = 368,
                    mesBoxYSize = 146,
                    mesBoxX     = (640 - mesBoxXSize) / 2,
                    mesBoxY     = (480 - mesBoxYSize) / 3;


// panels
Rect16  messagePanel(mesBoxX,
                     mesBoxY,
                     mesBoxXSize,
                     mesBoxYSize);



// buttons
Rect16  mesCancelBtnRect(mesBoxXSzNS - (pushButtonWidth + mesBtnOffset),
                         mesBoxY - mesBtnOffset,
                         pushButtonWidth,
                         pushButtonHeight);

Rect16  mesOkBtnRect(mesBtnOffset,
                     mesBoxY - mesBtnOffset,
                     pushButtonWidth,
                     pushButtonHeight);


Rect16  mesBtn3Rect(mesBoxXSzNS / 2 - pushButtonWidth / 2,
                    mesBoxY - mesBtnOffset,
                    pushButtonWidth,
                    pushButtonHeight);




// texts
Rect16  mesTitleRect(0,
                     0,
                     mesBoxXSzNS,
                     47);

Rect16  messageRect(0,
                    0,
                    mesBoxXSzNS,
                    mesBoxYSize);



Rect16  messageTextRects[numMessageTexts]     = { { mesTitleRect },
	{ messageRect }
};


Rect16  messageButtonRects[numMessageBtns]    = { { mesOkBtnRect },
	{ mesCancelBtnRect },
	{ mesBtn3Rect     }
};



// options window rect
Rect16  messageWindowRect = Rect16(mesBoxX,
                                   mesBoxY,
                                   mesBoxXSize,
                                   mesBoxYSize);




// message dialog window decorations
WindowDecoration messageDecorations[numMessagePanels] =

{ { WindowDecoration(messagePanel, mesPanelResID) } };


// pointer to the auto aggression button
gOwnerSelCompButton *autoAggressBtn,
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


char **initFileFields(void) {
	uint16              i;
	SaveFileHeader      header;                 //  The save file header.

	char **strings = new (char *[numEditLines]);

	for (i = 0; i < numEditLines; i++) {
		strings[i] = new char[editLen + 1];

		if (getSaveName(i, header)) {
			strncpy(strings[i], header.saveName, editLen);
		} else {
			strncpy(strings[i], FILE_DIALOG_NONAME, editLen);
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
	FILE            *fileHandle;            //  A standard C file handle
	char            fileName[fileNameSize + 1];

	//  Construct the file name based on the save number
	getSaveFileName(saveNo, fileName);

	//  Open the file or throw an exception
	if ((fileHandle = fopen(fileName, "rb")) == nullptr) {
		return false;
	}

	//  Read the save file header
	if (fread(&header, sizeof(header), 1, fileHandle) != 1) {
		return false;
	}

	// close the used file handle
	if (fileHandle != nullptr) fclose(fileHandle);

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
	const char    *saveTextStrings[numSaveLoadTexts]    = { SAVE_DIALOG_NAME };
	const char    *saveBtnStrings[numSaveLoadBtns]      = { SAVE_DIALOG_BUTTON1, SAVE_DIALOG_BUTTON2 };

	const char    *loadTextStrings[numSaveLoadTexts]    = { LOAD_DIALOG_NAME };
	const char    *loadBtnStrings[numSaveLoadBtns]      = { LOAD_DIALOG_BUTTON1, LOAD_DIALOG_BUTTON2 };

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
	checkAlloc(pushBtnIm        = loadButtonRes(decRes, dialogPushResNum, numBtnImages));
	checkAlloc(arrowUpIm        = loadButtonRes(decRes, upArrowResNum, numBtnImages));
	checkAlloc(arrowDnIm        = loadButtonRes(decRes, dnArrowResNum, numBtnImages));


	// create the window
	checkAlloc(win = new ModalWindow(saveLoadWindowRect,
	                                 0,
	                                 nullptr));

	gCompButton *t;
	// make the quit button
	checkAlloc(t = new gCompButton(*win, saveLoadButtonRects[0],
	                               pushBtnIm, numBtnImages, btnStrings[stringIndex][0], pal, 0, cmdDialogQuit));
	//t->accelKey=0x1B;

	// make the Save/Load button
	checkAlloc(t = new gCompButton(*win, saveLoadButtonRects[1],
	                               pushBtnIm, numBtnImages, btnStrings[stringIndex][1], pal, fileProcess, fileCommands[fileProcess]));
	//t->accelKey=0x0D;
	// make the up arrow
	checkAlloc(t = new gCompButton(*win, saveLoadButtonRects[2],
	                               arrowUpIm, numBtnImages, 0, cmdSaveDialogUp));
	//t->accelKey=33+0x80;
	// make the down arrow
	checkAlloc(t = new gCompButton(*win, saveLoadButtonRects[3],
	                               arrowDnIm, numBtnImages, 0, cmdSaveDialogDown));
	//t->accelKey=34+0x80;
	// attach the title
	checkAlloc(new CPlaqText(*win, saveLoadTextRects[0],
	                         textStrings[stringIndex][0], &Plate18Font, 0, pal, 0, nullptr));



	// attach the text box editing field object
	checkAlloc(textBox          = new gTextBox(*win, editBaseRect, &Onyx10Font,
	        textHeight, textPen, textBackground, textHilite, textBackHilite, cursorColor,
	        nullptr, "Error out", fieldStrings, editLen, 0, (uint16) - 1, displayOnly, nullptr,
	        fileCommands[fileProcess], cmdDialogQuit));


	win->setDecorations(saveWindowDecorations,
	                    elementsof(saveWindowDecorations),
	                    decRes, 'S', 'L', 'D');

	win->userData = &rInfo;
	win->open();

	if (GameMode::newmodeFlag)
		GameMode::update();

	win->invalidate();
	//win->draw();
	//G_BASE.setActive(textBox);

	EventLoop(rInfo.running, true);


	// remove the window all attatched controls
	if (win) delete win;
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
void reDrawScreen(void) ;
void updateMainDisplay(void);
void updateActiveRegions();
void drawMainDisplay(void);
void fadeUp();
void fadeDown();
void clearTileAreaPort(void);
void displayUpdate(void);
void disableUserControls(void);
void enableUserControls(void);
void updateAllUserControls(void);

int16 OptionsDialog(bool disableSaveResume) {
	// text for dialog
	const char  *btnStrings[numOptionsBtns] = {
		OPTN_DIALOG_BUTTON1,
		OPTN_DIALOG_BUTTON2,
		OPTN_DIALOG_BUTTON3,
		OPTN_DIALOG_BUTTON4,
		OPTN_DIALOG_BUTTON5
	};


	const char *textStrings[numOptionsTexts] = {
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
	checkAlloc(dialogPushImag   = loadButtonRes(decRes, dialogPushResNum, numBtnImages));
	checkAlloc(checkImag        = loadButtonRes(decRes, checkResNum, numBtnImages));
	checkAlloc(slideFaceImag    = loadButtonRes(decRes, slideFaceResNum, numSlideFace));

	// create the window
	checkAlloc(win = new ModalWindow(optionsWindowRect,
	                                 0,
	                                 nullptr));
	gCompButton *t;

	// buttons
	if (!disableSaveResume) {
		checkAlloc(t = new gCompButton(*win, optionsButtonRects[0],
		                               dialogPushImag, numBtnImages, btnStrings[0], pal, 0, cmdDialogQuit));
		t->accelKey = 0x1B;

		checkAlloc(t = new gCompButton(*win, optionsButtonRects[1],
		                               dialogPushImag, numBtnImages, btnStrings[1], pal, 0, cmdOptionsSaveGame));    // make the quit button
		t->accelKey = 'S';
	} else {
		checkAlloc(t = new gCompButton(*win, optionsButtonRects[1],
		                               dialogPushImag, numBtnImages, OPTN_DIALOG_BUTTON6, pal, 0, cmdOptionsNewGame));
		t->accelKey = 'N';
	}

	checkAlloc(t = new gCompButton(*win, optionsButtonRects[2],
	                               dialogPushImag, numBtnImages, btnStrings[2], pal, 0, cmdOptionsLoadGame));    // make the quit button
	t->accelKey = 'L';

	checkAlloc(t = new gCompButton(*win, optionsButtonRects[3],
	                               dialogPushImag, numBtnImages, btnStrings[3], pal, 0, cmdQuitGame));
	t->accelKey = 'Q';

	checkAlloc(t = new gCompButton(*win, optionsButtonRects[4],
	                               dialogPushImag, numBtnImages, btnStrings[4], pal, 0, cmdCredits));
	t->accelKey = 'C';

	checkAlloc(autoAggressBtn = new gOwnerSelCompButton(*win, optionsButtonRects[5],
	        checkImag, numBtnImages, 0, cmdAutoAggression));
	autoAggressBtn->select(isAutoAggressionSet());

	checkAlloc(autoWeaponBtn = new gOwnerSelCompButton(*win, optionsButtonRects[6],
	        checkImag, numBtnImages, 0, cmdAutoWeapon));
	autoWeaponBtn->select(isAutoWeaponSet());

	checkAlloc(speechTextBtn = new gOwnerSelCompButton(*win, optionsButtonRects[7],
	        checkImag, numBtnImages, 0, cmdSpeechText));
	speechTextBtn->select(globalConfig.speechText);

	checkAlloc(nightBtn = new gOwnerSelCompButton(*win, optionsButtonRects[8],
	        checkImag, numBtnImages, 0, cmdNight));
	nightBtn->select(globalConfig.showNight);

	checkAlloc(new gSlider(*win, optTopSliderRect, optTopFaceRect, 0,
	                       127, slideFaceImag, numSlideFace, globalConfig.soundVolume,
	                       0, cmdSetSoundVolume));

	checkAlloc(new gSlider(*win, optMidSliderRect, optMidFaceRect, 0,
	                       127, slideFaceImag, numSlideFace, globalConfig.voiceVolume,
	                       0, cmdSetSpeechVolume));

	checkAlloc(new gSlider(*win, optBotSliderRect, optBotFaceRect, 0,
	                       127, slideFaceImag, numSlideFace, globalConfig.musicVolume,
	                       0, cmdSetMIDIVolume));

	checkAlloc(new CPlaqText(*win, optionsTextRects[0],
	                         textStrings[0], &Plate18Font, 0, pal, 0, nullptr));

	for (int i = 1; i < numOptionsTexts; i++) {
		checkAlloc(new CPlaqText(*win, optionsTextRects[i],
		                         textStrings[i], &SmallFont, textPosLeft, pal, 0, nullptr));
	}

	win->setDecorations(optionsDecorations,
	                    elementsof(optionsDecorations),
	                    decRes, 'O', 'P', 'T');


	win->userData = &rInfo;
	win->open();

	EventLoop(rInfo.running, true);

	writeConfig();

	// remove the window all attatched controls
	if (win) delete win;
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
			saveGameState(deferredLoadID, deferredSaveName);
#endif
		}
		mainWindow->invalidate(&optionsWindowRect);
	}

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

bool initUserDialog(void) {

	const   int16   dialogPushResNum    = 4;
	// init the resource context handle
	udDecRes = resFile->newContext(dialogGroupID, "dialog resources");


	// get the graphics associated with the buttons
	checkAlloc(udDialogPushImag = loadButtonRes(udDecRes, dialogPushResNum, numBtnImages));

	// create the window
	checkAlloc(udWin = new ModalWindow(messageWindowRect,
	                                   0,
	                                   nullptr));

	udWin->setDecorations(messageDecorations,
	                      elementsof(messageDecorations),
	                      udDecRes, 'M', 'E', 'S');

	udWin->userData = &udrInfo;

	if (udDecRes) resFile->disposeContext(udDecRes);
	udDecRes = nullptr;

	udInit = true;

	return true;
}

bool userDialogAvailable(void) {
	return udInit;
}

void cleanupUserDialog(void) {
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

	gCompButton *t;

	// button one
	if (numBtns >= 1) {
		checkAlloc(t = new gCompButton(*udWin, messageButtonRects[0],
		                               udDialogPushImag, numBtnImages, btnMsg1, pal, 10, cmdDialogQuit));
		t->accel = k1;
	}

	// button two
	if (numBtns >= 2) {
		checkAlloc(t = new gCompButton(*udWin, messageButtonRects[1],
		                               udDialogPushImag, numBtnImages, btnMsg2, pal, 11, cmdDialogQuit));
		t->accel = k2;
	}

	// button three
	if (numBtns >= 3) {
		checkAlloc(t = new gCompButton(*udWin, messageButtonRects[2],
		                               udDialogPushImag, numBtnImages, btnMsg3, pal, 12, cmdDialogQuit));
		t->accel = k3;
	}

	// title for the box
	checkAlloc(new CPlaqText(*udWin, messageTextRects[0],
	                         title, &Plate18Font, nullptr, pal, 0, nullptr));

	// message for box
	checkAlloc(new CPlacardPanel(*udWin, messageTextRects[1],
	                             msg, &Onyx10Font, nullptr, pal, 0, nullptr));


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

bool initUserDialog(void) {
	return true;
}
bool userDialogAvailable(void) {
	return true;
}
void cleanupUserDialog(void) {}

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
	checkAlloc(dialogPushImag   = loadButtonRes(decRes, dialogPushResNum, numBtnImages));

	// create the window
	checkAlloc(win = new ModalWindow(messageWindowRect,
	                                 0,
	                                 nullptr));

	gCompButton *t;

	// button one
	if (numBtns >= 1) {
		checkAlloc(t = new gCompButton(*win, messageButtonRects[0],
		                               dialogPushImag, numBtnImages, btnMsg1, pal, 10, cmdDialogQuit));
		t->accelKey = k1;
	}

	// button two
	if (numBtns >= 2) {
		checkAlloc(t = new gCompButton(*win, messageButtonRects[1],
		                               dialogPushImag, numBtnImages, btnMsg2, pal, 11, cmdDialogQuit));
		t->accelKey = k2;
	}

	// button three
	if (numBtns >= 3) {
		checkAlloc(t = new gCompButton(*win, messageButtonRects[2],
		                               dialogPushImag, numBtnImages, btnMsg3, pal, 12, cmdDialogQuit));
		t->accelKey = k3;
	}

	// title for the box
	checkAlloc(new CPlaqText(*win, messageTextRects[0],
	                         title, &Plate18Font, 0, pal, 0, nullptr));

	// message for box
	checkAlloc(new CPlacardPanel(*win, messageTextRects[1],
	                             msg, &Onyx10Font, 0, pal, 0, nullptr));

	win->setDecorations(messageDecorations,
	                    elementsof(messageDecorations),
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
	if (!extent.overlap(r)) return;

	// do background drawing first...
	ModalWindow::drawClipped(port, offset, r);

	int16   i;
	Point16 origin;
	Rect16  rect;

	SAVE_GPORT_STATE(port);

	origin.x = extent.x - offset.x;
	origin.y = extent.y - offset.y;

	rect.x = origin.x;
	rect.y = origin.y;
	rect.width  = extent.width;
	rect.height = extent.height;

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
	if (!extent.overlap(r)) return;

	// do background drawing first...
	int16   i;
	Point16 origin;
	Rect16  rect;

	SAVE_GPORT_STATE(port);

	origin.x = extent.x - offset.x;
	origin.y = extent.y - offset.y;

	rect.x = origin.x;
	rect.y = origin.y;
	rect.width  = extent.width;
	rect.height = extent.height;

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
	WindowDecoration plaqDecWood[1] =
	{ { WindowDecoration(plaqRectWood, 0) } };

	WindowDecoration plaqDecStone[1] =
	{ { WindowDecoration(plaqRectStone, 1) } };

	WindowDecoration plaqDecBrass[2] =
	{ { WindowDecoration(plaqRectBrass, 2) } };

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
		checkAlloc(win = new CPlacardWindow(plaqRectWood, 0, nullptr, text, pal, &Plate18Font));

		// setup the background imagery
		win->setDecorations(plaqDecWood,
		                    elementsof(plaqDecWood),
		                    resContext, 'P', 'L', 'Q');

		break;

	case STONE_TYPE:

		// set stone text inlay color
		pal.set(16, 12, 18, 11, 23, 0x78);

		// create the window
		checkAlloc(win = new CPlacardWindow(plaqRectStone, 0, nullptr, text, pal, &Plate18Font));

		// setup the background imagery
		win->setDecorations(plaqDecStone,
		                    elementsof(plaqDecStone),
		                    resContext, 'P', 'L', 'Q');

		break;

	case BRASS_TYPE:

		// set brass text inlay color
		pal.set(89, 93, 95, 11, 23, 0x76);

		// create the window
		checkAlloc(win = new CPlacardWindow(plaqRectBrass, 0, nullptr, text, pal, &Plate18Font));

		// setup the background imagery
		win->setDecorations(plaqDecBrass,
		                    elementsof(plaqDecBrass),
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

//void updateShowNightButton( bool setting )
//{
//	if ( nightBtn != nullptr )
//		nightBtn->select( setting );
//}

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
		saveGameState(saveIndex, textBox->getLine(saveIndex));
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
		globalConfig.showNight = !globalConfig.showNight;
		nightBtn->select(globalConfig.showNight);
	}
}

APPFUNC(cmdSpeechText) {
	if (isUserAction(ev)) {
		globalConfig.speechText = !globalConfig.speechText;
		speechTextBtn->select(globalConfig.speechText);
	}
}

void volumeChanged(void);

// Set music volume

APPFUNC(cmdSetMIDIVolume) {
	int16 v = quantizedVolume(ev.value);
	globalConfig.musicVolume = v;
	audio->setVolume(volMusic, volumeSetTo, globalConfig.musicVolume);
	volumeChanged();
}

// Set DIG volume for DINO

APPFUNC(cmdSetDIGVolume) {
	int16 v = quantizedVolume(ev.value);
	globalConfig.soundVolume = v;
	globalConfig.voiceVolume = v;
	//audio->setVolume(volVoice,volumeSetTo,globalConfig.voiceVolume);
	audio->setVolume(volSoundMaster, volumeSetTo, globalConfig.soundVolume);
	//setVideoVolume(globalConfig.soundVolume);
	volumeChanged();
}

// Set DIG speech volume for FTA

APPFUNC(cmdSetSpeechVolume) {
	int16 v = quantizedVolume(ev.value);
	globalConfig.voiceVolume = v;
	audio->setVolume(volVoice, volumeSetTo, globalConfig.voiceVolume);
	volumeChanged();
}

// Set DIG sound volume for FTA

APPFUNC(cmdSetSoundVolume) {
	int16 v = quantizedVolume(ev.value);
	globalConfig.soundVolume = v;
	audio->setVolume(volSandL, volumeSetTo, globalConfig.soundVolume);
	//setVideoVolume(globalConfig.soundVolume);
	volumeChanged();
}

// Save volume settings
//   This should be called when exiting the dialog to save the changes

APPFUNCV(cmdSaveVolumeSettings) {
	writeConfig();
}

} // end of namespace Saga2
