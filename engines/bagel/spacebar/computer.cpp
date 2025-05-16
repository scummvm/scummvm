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

#include "bagel/spacebar/computer.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/boflib/sound.h"
#include "bagel/spacebar/baglib/inv.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/spacebar/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define DRINK_FILE       "$SBARDIR\\BAR\\CLOSEUP\\BDCA\\DRINKS.TXT"
#define INGRD_FILE       "$SBARDIR\\BAR\\CLOSEUP\\BDCA\\ING.TXT"
#define BDCA_DIR         "$SBARDIR\\BAR\\CLOSEUP\\BDCA\\"
#define ORDER_AUDIO      "BDNDPSC1.WAV"
#define REFUSE_AUDIO     "BDTOORC1.WAV"
#define ON_AUDIO         "ON.WAV"

#define szBroke "You have insufficient funds for that purchase."
#define szRefuse "The computer politely informs you that the House has a policy against two-fisted drinking."

struct ST_BUTTONS {
	const char *_pszName;
	const char *_pszUp;
	const char *_pszDown;
	const char *_pszFocus;
	const char *_pszDisabled;
	int _nLeft;
	int _nTop;
	int _nWidth;
	int _nHeight;
	int _nID;
};

static const ST_BUTTONS g_stButtons[NUM_COMPBUTT] = {
	{ "Off", "bdnb1.bmp", "bdnb1.bmp", "bdnb1.bmp", "bdnb1.bmp", 144, 323, 92, 48, OFFBUT },
	{ "On", "bdnb2.bmp", "bdnb2.bmp", "bdnb2.bmp", "bdnb2.bmp",  144, 323, 92, 48, ONBUT },
	{ "Drinks", "bdnr1.bmp", "bdnr2.bmp", "bdnr1.bmp", "bdnr1.bmp", 238, 318, 112, 60, DRINKS },
	{ "Ingred", "bdnq1.bmp", "bdnq2.bmp", "bdnq1.bmp", "bdnq1.bmp",  238, 318, 112, 60, INGRED },
	{ "ListD", "bdnc1.bmp", "bdnc2.bmp", "bdnc2.bmp", "bdnc1.bmp", 354, 318, 112, 60, LISTD },
	{ "ListI", "bdnc4.bmp", "bdnc5.bmp", "bdnc4.bmp", "bdnc3.bmp", 354, 318, 112, 60, LISTI },
	{ "Order", "bdnd1.bmp", "bdnd2.bmp", "bdnd1.bmp", "bdnd1.bmp", 238, 378, 112, 60, ORDER },
	{ "Page Up", "bdns1.bmp", "bdns2.bmp", "bdns1.bmp", "bdns1.bmp", 188, 392, 44, 35, PGUP },
	{ "Page Down", "bdnt1.bmp", "bdnt2.bmp", "bdnt1.bmp", "bdnt1.bmp", 356, 392, 44, 35, PGDOWN },
	{ "Back", "backup.bmp", "backdn.bmp", "backup.bmp", "backup.bmp", 354, 318, 112, 60, BCBACK },
	{ "Help", "helpup.bmp", "helpdown.bmp", "helpup.bmp", "helpup.bmp", 31, 445, 200, 30, BCHELP },
	{ "Quit", "quitup.bmp", "quitdown.bmp", "quitup.bmp", "quitup.bmp", 407, 445, 200, 30, BCQUIT }
};

// Local functions
static const char *BuildBarcDir(const char *pszFile);

SBarComputer::SBarComputer() : CBagStorageDevWnd(),
	_compDisplay(165, 36, 430, 220), _compTextWindow(0, 0, 639, 21) {
	_pDrinkBuff = nullptr;
	_pIngBuff = nullptr;
	_pDrinkBox = nullptr;
	_pIngBox = nullptr;
	_pTBox = nullptr;
	_pDrinkList = nullptr;
	_pIngList = nullptr;
	//  _nSelection = -1;
	_nDrinkSelect = -1;
	_nIngSelect = -1;
	_eMode = OFFMODE;
	_ePrevMode = DRINKMODE;

	for (int i = 0; i < NUM_COMPBUTT; i++) {
		_pButtons[i] = nullptr;
	}

	// Call this thing a closeup so that time won't go by when
	// entering the closeup
	setCloseup(true);
}
SBarComputer::~SBarComputer() {
	delete _pDrinkBox;
	delete _pIngBox;
}

void SBarComputer::onMainLoop() {
	if (_bFirstPaint) {
		_bFirstPaint = false;
		attachActiveObjects();
	}
}

void SBarComputer::eraseBackdrop() {
	invalidateRect(&_compDisplay);

	updateWindow();
}

void  SBarComputer::onPaint(CBofRect *pRect) {
	if (getBackdrop()) {
		assert(getWorkBmp() != nullptr);

		// Erase everything from the background
		getWorkBmp()->paint(getBackdrop(), pRect, pRect);
		// Paint all the objects to the background
		paintStorageDevice(nullptr, getBackdrop(), pRect);
	}

	if (_pTBox != nullptr)
		_pTBox->display();

	// Paint the backdrop
	if (getBackdrop())
		paintBackdrop();
}

ErrorCode SBarComputer::attach() {
	logInfo("Attaching SBarComputer...");

	ErrorCode errorCode = CBagStorageDevWnd::attach();
	if (errorCode == ERR_NONE) {
		g_waitOKFl = false;

		_pDrinkList = new CBofList<SBarCompItem>;
		_pIngList = new CBofList<SBarCompItem>;

		errorCode = readDrnkFile();
		if (errorCode == ERR_NONE)
			errorCode = readIngFile();

		// Create the list box that display the lists
		createDrinksListBox();
		createIngListBox();

		// Must have a valid backdrop by now
		assert(_pBackdrop != nullptr);
		CBofPalette *pPal = _pBackdrop->getPalette();

		// Build all our buttons
		for (int i = 0; i < NUM_COMPBUTT; i++) {
			_pButtons[i] = new CBofBmpButton;

			CBofBitmap *pUp = loadBitmap(BuildBarcDir(g_stButtons[i]._pszUp), pPal);
			CBofBitmap *pDown = loadBitmap(BuildBarcDir(g_stButtons[i]._pszDown), pPal);
			CBofBitmap *pFocus = loadBitmap(BuildBarcDir(g_stButtons[i]._pszFocus), pPal);
			CBofBitmap *pDis = loadBitmap(BuildBarcDir(g_stButtons[i]._pszDisabled), pPal);

			_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
			_pButtons[i]->create(g_stButtons[i]._pszName, g_stButtons[i]._nLeft, g_stButtons[i]._nTop, g_stButtons[i]._nWidth, g_stButtons[i]._nHeight, this, g_stButtons[i]._nID);
			_pButtons[i]->hide();
		}

		show();
		_pButtons[OFFBUT]->show();
		_pButtons[BCHELP]->show();
		_pButtons[BCQUIT]->show();

		if (_pDrinkBox != nullptr)
			_pDrinkBox->setSelectedItem(-1, false);

		if (_pIngBox != nullptr)
			_pIngBox->setSelectedItem(-1, false);

		updateWindow();
	}

	CBagCursor::showSystemCursor();

	return errorCode;
}

ErrorCode SBarComputer::detach() {
	CBagCursor::hideSystemCursor();

	delete _pDrinkBuff;
	_pDrinkBuff = nullptr;

	delete _pIngBuff;
	_pIngBuff = nullptr;

	deleteListBox();
	deleteTextBox();

	delete _pDrinkList;
	_pDrinkList = nullptr;

	delete _pIngList;
	_pIngList = nullptr;

	// Destroy all buttons
	for (int i = 0; i < NUM_COMPBUTT; i++) {
		delete _pButtons[i];
		_pButtons[i] = nullptr;
	}

	_nDrinkSelect = -1;
	_nIngSelect = -1;

	_eMode = OFFMODE;

	CBagStorageDevWnd::detach();

	// Going into mr drinkmaster makes 1 turn go by
	g_VarManager->incrementTimers();

	logInfo("Detached SBarComputer");

	return ERR_NONE;
}

ErrorCode SBarComputer::readDrnkFile() {
	CBofString DrinkString(DRINK_FILE);
	fixPathName(DrinkString);

	// Open the text files
	CBofFile fpDrinkFile(DrinkString);
	if (fpDrinkFile.errorOccurred())
		return fpDrinkFile.getErrorCode();

	// Check that buffers are null
	delete _pDrinkBuff;
	_pDrinkBuff = nullptr;

	// Allocate the buffers
	_pDrinkBuff = (char *)bofAlloc(fpDrinkFile.getLength() + 1);

	// Read the text file into buffers
	fpDrinkFile.read(_pDrinkBuff, fpDrinkFile.getLength());

	// Get pointers indexing into Drink buffers
	char *pPosInBuff = _pDrinkBuff;
	while (pPosInBuff < _pDrinkBuff + fpDrinkFile.getLength()) {
		SBarCompItem *pCompItem = new SBarCompItem();
		pCompItem->_pList = nullptr;
		pCompItem->_pDrink = nullptr;

		// Get the item pointer pointing to item
		pCompItem->_pItem = pPosInBuff;

		// Search for @ - field delimiter
		while (*pPosInBuff != '@')
			pPosInBuff++;
		*pPosInBuff = '\0'; // Replace with \0
		pPosInBuff++; // Increment past it

		// Get the list item pointing to list
		pCompItem->_pList = pPosInBuff;

		// Search for @ - field delimiter
		while (*pPosInBuff != '@')
			pPosInBuff++;
		*pPosInBuff = '\0'; // Replace with \0
		pPosInBuff++; // Increment past it

		// Get the drink item pointing to list
		pCompItem->_pDrink = pPosInBuff;

		// Search for $ - record delimiter
		while (*pPosInBuff != '$')
			pPosInBuff++;
		*pPosInBuff = '\0'; // Replace with \0
		pPosInBuff++;   // Increment past it

		// Search record delimiter from beginning of next
		while ((pPosInBuff < _pDrinkBuff + fpDrinkFile.getLength()) && (*pPosInBuff != '$'))
			pPosInBuff++;

		pPosInBuff++;   // Increment past it

		_pDrinkList->addToTail(*pCompItem);
		delete pCompItem;
	}

	fpDrinkFile.close();
	return ERR_NONE;
}

ErrorCode SBarComputer::readIngFile() {
	CBofString IngString(INGRD_FILE);
	fixPathName(IngString);

	// Open the text files
	CBofFile fpIngFile(IngString);
	if (fpIngFile.errorOccurred())
		return fpIngFile.getErrorCode();

	// Check that buffers are null
	delete _pIngBuff;
	_pIngBuff = nullptr;

	// Allocate the buffers
	_pIngBuff = (char *)bofAlloc(fpIngFile.getLength() + 1);

	// Read the text file into buffers
	fpIngFile.read(_pIngBuff, fpIngFile.getLength());

	// Get pointers indexing into Ingredient buffers
	char *pPosInBuff = _pIngBuff;
	while (pPosInBuff < _pIngBuff + fpIngFile.getLength()) {
		SBarCompItem *pCompItem = new SBarCompItem();
		pCompItem->_pList = nullptr;
		pCompItem->_pDrink = nullptr;

		// get the item pointer pointing to item
		pCompItem->_pItem = pPosInBuff;

		// search for @ - field delimiter
		while (*pPosInBuff != '@')
			pPosInBuff++;
		*pPosInBuff = '\0'; // replace with /0
		pPosInBuff++; // Increment past it

		// Get the list item pointing to list
		pCompItem->_pList = pPosInBuff;

		// Search for $ - record delimiter
		while (*pPosInBuff != '$')
			pPosInBuff++;
		*pPosInBuff = '\0'; // Replace with /0
		pPosInBuff++;   // Increment past it

		// Search record delimiter from beginning of next
		while ((pPosInBuff < _pIngBuff + fpIngFile.getLength()) && (*pPosInBuff != '$'))
			pPosInBuff++;

		pPosInBuff++;   // Increment past it

		_pIngList->addToTail(*pCompItem);
		delete pCompItem;
	}

	fpIngFile.close();
	return ERR_NONE;
}

void SBarComputer::createTextBox(CBofString &newText) {
	if (_pTBox == nullptr) {
		_pTBox = new CBofTextBox(getBackdrop(), &_compDisplay, newText);
		_pTBox->setTextAttribs(12, TEXT_NORMAL, RGB(0, 0, 0));
	} else {
		eraseBackdrop();
		_pTBox->erase();
		_pTBox->flushBackground();
		_pTBox->setText(newText);
	}

	if (_pButtons[ONBUT]) {
		_pButtons[ONBUT]->show();
		_pButtons[ONBUT]->invalidateRect(nullptr);
	}
}

void SBarComputer::deleteTextBox() {
	delete _pTBox;
	_pTBox = nullptr;
}

void SBarComputer::deleteListBox() {
	delete _pDrinkBox;
	_pDrinkBox = nullptr;

	delete _pIngBox;
	_pIngBox = nullptr;
}

ErrorCode SBarComputer::createDrinksListBox() {
	ErrorCode errorCode = ERR_NONE;

	if (_pDrinkBox != nullptr)
		return errorCode;

	// We need to create one
	_pDrinkBox = new CBofListBox();
	errorCode = _pDrinkBox->create("ListBox", &_compDisplay, this);

	if (errorCode != ERR_NONE) {
		return errorCode;
	}

	_pDrinkBox->hide();
	_pDrinkBox->setPointSize(12);
	_pDrinkBox->setItemHeight(20);

	CBofPalette *pPal = _pBackdrop->getPalette();
	byte PalIdx = pPal->getNearestIndex(RGB(255, 0, 0));

	_pDrinkBox->setHighlightColor(pPal->getColor(PalIdx));

	// Populate listbox
	int numItems = _pDrinkList->getCount();
	for (int i = 0; i < numItems; ++i) {
		SBarCompItem CompItem = _pDrinkList->getNodeItem(i);
		_pDrinkBox->addToTail(CBofString(CompItem._pItem), false);
	}

	return errorCode;
}

ErrorCode SBarComputer::createIngListBox() {
	ErrorCode errorCode = ERR_NONE;

	if (_pIngBox != nullptr)
		return errorCode;

	// We need to create one
	_pIngBox = new CBofListBox();
	errorCode = _pIngBox->create("ListBox", &_compDisplay, this);

	if (errorCode != ERR_NONE) {
		return errorCode;
	}

	_pIngBox->hide();
	_pIngBox->setPointSize(12);
	_pIngBox->setItemHeight(20);

	CBofPalette *pPal = _pBackdrop->getPalette();
	byte PalIdx = pPal->getNearestIndex(RGB(255, 0, 0));

	_pIngBox->setHighlightColor(pPal->getColor(PalIdx));


	// Populate listbox
	int numItems = _pIngList->getCount();
	for (int i = 0; i < numItems; ++i) {
		SBarCompItem CompItem = _pIngList->getNodeItem(i);
		_pIngBox->addToTail(CBofString(CompItem._pItem), false);
	}

	return errorCode;
}

void SBarComputer::onBofListBox(CBofObject * /*pListBox*/, int nItemIndex) {
	if (_eMode == DRINKMODE) {
		_nDrinkSelect = nItemIndex;

		// Prevents the white flash when the show window is performed
		_pButtons[ORDER]->select();
		_pButtons[LISTD]->select();

		_pButtons[ORDER]->show();
		_pButtons[LISTD]->show();

		_pDrinkBox->repaintAll();

	} else {
		_nIngSelect = nItemIndex;
		// Prevents the white flash when the show window is performed
		_pButtons[LISTI]->select();
		_pButtons[LISTI]->show();

		_pIngBox->repaintAll();
	}

	validateAnscestors(nullptr);
	if (_eMode == DRINKMODE) {
		_pButtons[LISTD]->invalidateRect(nullptr);
	} else {
		_pButtons[LISTI]->invalidateRect(nullptr);
	}
	_pButtons[ORDER]->invalidateRect(nullptr);
	_pButtons[PGUP]->invalidateRect(nullptr);
	_pButtons[PGDOWN]->invalidateRect(nullptr);

	updateWindow();
}

void SBarComputer::setOn() {
	_pButtons[OFFBUT]->hide();
	_pButtons[ONBUT]->show();

	// Play switching-on sound
	BofPlaySound(BuildBarcDir(ON_AUDIO), SOUND_MIX);

	setDrink();
	updateWindow();
}

void SBarComputer::setOff() {
	if (_eMode != OFFMODE) {

		// Fix drink already selected bug
		if (_pDrinkBox != nullptr) {
			_pDrinkBox->setSelectedItem(-1, false);
		}

		if (_pIngBox != nullptr) {
			_pIngBox->setSelectedItem(-1, false);
		}

		_nDrinkSelect = -1;

		if (_pDrinkBox != nullptr)
			_pDrinkBox->hide();
		if (_pIngBox != nullptr)
			_pIngBox->hide();

		deleteTextBox();

		_eMode = OFFMODE;

		_pButtons[ONBUT]->hide();
		_pButtons[OFFBUT]->show();

		BofPlaySound(BuildBarcDir(ON_AUDIO), SOUND_MIX);

		for (int i = 1; i < NUM_COMPBUTT; i++) {
			// Hide all the buttons but HELP and QUIT
			if ((i != BCHELP) && (i != BCQUIT)) {
				_pButtons[i]->hide();
			}
		}
	}

	invalidateRect(&_compDisplay);
	updateWindow();

}

void SBarComputer::setDrink() {
	if (_eMode != DRINKMODE) {
		deleteTextBox();

		// Prevents the white flash when the show window is performed
		_pButtons[INGRED]->select();

		_pButtons[DRINKS]->hide();
		_pButtons[INGRED]->show();

		//_nIngSelect = -1;
		_pButtons[LISTI]->hide();

		// Prevents the white flash when the show window is performed
		_pButtons[PGUP]->select();
		_pButtons[PGDOWN]->select();

		_pButtons[PGUP]->show();
		_pButtons[PGDOWN]->show();
		_pButtons[BCBACK]->hide();

		if (_nDrinkSelect != -1) {
			// Prevents the white flash when the show window is performed
			_pButtons[LISTD]->select();
			_pButtons[LISTD]->show();
		}

		_pIngBox->hide();

		_pDrinkBox->select();
		_pDrinkBox->show();

		eraseBackdrop();

		_eMode = DRINKMODE;
	}
}

void SBarComputer::setIng() {
	if (_eMode != INGMODE) {
		deleteTextBox();

		// Prevents the white flash when the show window is performed
		_pButtons[DRINKS]->select();

		// Set up buttons
		_pButtons[INGRED]->hide();
		_pButtons[DRINKS]->show();

		// Hide list drink ingredient button
		_pButtons[LISTD]->hide();

		// Prevents the white flash when the show window is performed
		_pButtons[PGUP]->select();
		_pButtons[PGDOWN]->select();

		_pButtons[PGUP]->show();
		_pButtons[PGDOWN]->show();
		_pButtons[BCBACK]->hide();

		if (_nIngSelect != -1) {
			_pButtons[LISTI]->select();
			_pButtons[LISTI]->show();
		}

		_pDrinkBox->hide();
		_pIngBox->select();
		_pIngBox->show();

		eraseBackdrop();

		_eMode = INGMODE;
	}

	updateWindow();
}


void SBarComputer::setList() {
	if (_eMode != LISTMODE) {
		_pDrinkBox->hide();
		_pIngBox->hide();

		SBarCompItem CompItem;
		if (_eMode == DRINKMODE) {
			assert(_nDrinkSelect != -1);
			CompItem = _pDrinkList->getNodeItem(_nDrinkSelect);

		} else {

			assert(_nIngSelect != -1);
			CompItem = _pIngList->getNodeItem(_nIngSelect);
		}

		CBofString cText(CompItem._pList);

		_pDrinkBox->hide();
		_pIngBox->hide();

		createTextBox(cText);

		// Save off the previous mode for BACK
		_ePrevMode = _eMode;

		// Prevents the white flash when the show window is performed
		_pButtons[BCBACK]->select();
		// Set up buttons
		_pButtons[PGDOWN]->hide();
		_pButtons[BCBACK]->show();

		_pButtons[LISTD]->hide();
		_pButtons[LISTI]->hide();

		_pButtons[PGUP]->hide();
		_pButtons[PGDOWN]->hide();

		_pTBox->setPageLength(10);

		_eMode = LISTMODE;
		updateWindow();
	}
}

void SBarComputer::back() {
	if (_ePrevMode == DRINKMODE)
		setDrink();
	else
		setIng();

	_pButtons[BCBACK]->hide();
}

void SBarComputer::order() {
	if (_nDrinkSelect != -1) {
		// Get the associated drink name
		SBarCompItem CompItem = _pDrinkList->getNodeItem(_nDrinkSelect);

		// Deduct 1 Nugget from the player
		// Read in their total nuggets from game
		CBagVar *pVar = g_VarManager->getVariable("NUGGETS");
		CBagVar *pVar2 = g_VarManager->getVariable("HAVEDRINK");

		if (pVar) {
			int nCredits = pVar->getNumValue();
			int nHaveDrink = pVar2->getNumValue();

			// If the player is out of nuggets, then put up a text message.
			if (nCredits < 1) {
				CBofBitmap saveBackground(640, 480, (CBofPalette *)nullptr, false);
				saveBackground.captureScreen(this, &_compTextWindow);
				paintBeveledText(&_compTextWindow, szBroke, FONT_15POINT, TEXT_NORMAL, CTEXT_WHITE, JUSTIFY_WRAP, FORMAT_TOP_LEFT);

				waitForInput();

				saveBackground.paint(this, &_compTextWindow);

			} else {
				CBagStorageDev *pSoldierSDev = g_SDevManager->getStorageDevice("SOLDIER_WLD");

				CBofBitmap saveBackgroundTwo(640, 480, (CBofPalette *)nullptr, false);
				saveBackgroundTwo.captureScreen(this, &_compTextWindow);

				// Don't allow him to order if he has other drinks in the Soldier CIC or stash
				if (pSoldierSDev) {
					bool bRefuse = false;
					if (pSoldierSDev->getObject("DRINK1", true))
						bRefuse = true;
					else if (pSoldierSDev->getObject("DRINK2", true))
						bRefuse = true;
					else if (pSoldierSDev->getObject("DRINK3", true))
						bRefuse = true;
					else if (pSoldierSDev->getObject("DRINK4", true))
						bRefuse = true;
					else if (pSoldierSDev->getObject("DRINK5", true))
						bRefuse = true;
					else if (pSoldierSDev->getObject("DRINK6", true))
						bRefuse = true;
					else if (pSoldierSDev->getObject("DRINK7", true))
						bRefuse = true;
					else if (nHaveDrink > 0)
						bRefuse = true;
					else {
						pSoldierSDev->activateLocalObject(CompItem._pDrink);
						BofPlaySound(BuildBarcDir(ORDER_AUDIO), SOUND_MIX);
						pVar->setValue(nCredits - 1);
						pVar2->setValue(1);
					}

					if (bRefuse) {
						paintBeveledText(&_compTextWindow, szRefuse, FONT_15POINT, TEXT_NORMAL, CTEXT_WHITE, JUSTIFY_WRAP, FORMAT_TOP_LEFT);
						waitForInput();

						saveBackgroundTwo.paint(this, &_compTextWindow);
					}
				}

			}
		}
	}
}

void SBarComputer::pageUp() {
	if (_eMode == DRINKMODE) {
		if (_pDrinkBox) {
			_pDrinkBox->pageUp();
		}
	} else if (_pIngBox) {
		_pIngBox->pageUp();
	}
}

void SBarComputer::pageDown() {
	if (_eMode == DRINKMODE) {
		if (_pDrinkBox) {
			_pDrinkBox->pageDown();
		}
	} else if (_pIngBox) {
		_pIngBox->pageDown();
	}
}

void SBarComputer::onBofButton(CBofObject *pObject, int nState) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nState != BUTTON_CLICKED)
		return;

	CBofButton *pButton = (CBofButton *)pObject;

	switch (pButton->getControlID()) {
	case OFFBUT:
		setOn();
		break;
	case ONBUT:
		setOff();
		break;
	case DRINKS:
		setDrink();
		break;
	case INGRED:
		setIng();
		break;
	case LISTD:
	case LISTI:
		setList();
		break;
	case ORDER:
		order();
		break;
	case PGUP:
		pageUp();
		break;
	case PGDOWN:
		pageDown();
		break;
	case BCBACK:
		back();
		break;
	case BCQUIT:
		logInfo("\tClicked Quit");
		close();
		break;

	case BCHELP: {
		logInfo("\tClicked Help");

		CBagel *pApp = CBagel::getBagApp();

		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {
				pWin->onHelp(BuildBarcDir("BARCOMP.TXT"));
			}
		}
	}
	break;

	default:
		logWarning(buildString("Clicked Unknown Button with ID %d", pButton->getControlID()));
		break;
	}
}

void SBarComputer::onKeyHit(uint32 lKey, uint32 nRepCount) {
	assert(isValidObject(this));

	switch (lKey) {
	case BKEY_UP:
		if (_eMode == DRINKMODE) {
			if (_pDrinkBox) {
				_pDrinkBox->lineUp();
			}
		} else if (_pIngBox) {
			_pIngBox->lineUp();
		}
		break;

	case BKEY_DOWN:
		if (_eMode == DRINKMODE) {
			if (_pDrinkBox) {
				_pDrinkBox->lineDown();
			}
		} else if (_pIngBox) {
			_pIngBox->lineDown();
		}
		break;

	case BKEY_PAGEUP:
		if (_eMode == DRINKMODE) {
			if (_pDrinkBox) {
				_pDrinkBox->pageUp();
			}
		} else if (_pIngBox) {
			_pIngBox->pageUp();
		}
		break;

	case BKEY_PAGEDOWN:
		if (_eMode == DRINKMODE) {
			if (_pDrinkBox) {
				_pDrinkBox->pageDown();
			}
		} else if (_pIngBox) {
			_pIngBox->pageDown();
		}
		break;

	default:
		CBagStorageDevWnd::onKeyHit(lKey, nRepCount);
		break;
	}
}

void SBarComputer::onMouseMove(uint32 nFlags, CBofPoint *xPoint, void *) {
	// Let CBagStorageDevWnd check for EXIT area
	CBagStorageDevWnd::onMouseMove(nFlags, xPoint);
}

const char *BuildBarcDir(const char *pszFile) {
	return formPath(BDCA_DIR, pszFile);
}

} // namespace SpaceBar
} // namespace Bagel
