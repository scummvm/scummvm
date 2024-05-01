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
#include "bagel/baglib/master_win.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/inv.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define DRINKFILE       "$SBARDIR\\BAR\\CLOSEUP\\BDCA\\DRINKS.TXT"
#define INGRDFILE       "$SBARDIR\\BAR\\CLOSEUP\\BDCA\\ING.TXT"
#define BDCADIR         "$SBARDIR\\BAR\\CLOSEUP\\BDCA\\"
#define ORDERAUDIO      "BDNDPSC1.WAV"
#define REFUSEAUDIO     "BDTOORC1.WAV"
#define ONAUDIO         "ON.WAV"

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
	SetCloseup(true);
}
SBarComputer::~SBarComputer() {
}

void SBarComputer::onMainLoop() {
	if (_bFirstPaint) {
		_bFirstPaint = false;
		AttachActiveObjects();
	}
}

void SBarComputer::eraseBackdrop() {
	invalidateRect(&_compDisplay);

	updateWindow();
}

void  SBarComputer::onPaint(CBofRect *pRect) {
	if (getBackdrop()) {
		Assert(GetWorkBmp() != nullptr);

		// Erase everything from the background
		GetWorkBmp()->paint(getBackdrop(), pRect, pRect);
		// Paint all the objects to the background
		PaintStorageDevice(nullptr, getBackdrop(), pRect);
	}

	if (_pTBox != nullptr)
		_pTBox->display();

	// Paint the backdrop
	if (getBackdrop())
		paintBackdrop();
}

ErrorCode SBarComputer::attach() {
	LogInfo("Attaching SBarComputer...");

	ErrorCode rc = CBagStorageDevWnd::attach();
	if (rc == ERR_NONE) {
		g_bWaitOK = false;

		_pDrinkList = new CBofList<SBarCompItem>;
		_pIngList = new CBofList<SBarCompItem>;

		rc = readDrnkFile();
		if (rc == ERR_NONE)
			rc = readIngFile();

		// Create the list box that display the lists
		createDrinksListBox();
		createIngListBox();

		// Must have a valid backdrop by now
		Assert(_pBackdrop != nullptr);
		CBofPalette *pPal = _pBackdrop->getPalette();

		// Build all our buttons
		for (int i = 0; i < NUM_COMPBUTT; i++) {
			if ((_pButtons[i] = new CBofBmpButton) != nullptr) {

				CBofBitmap *pUp = loadBitmap(BuildBarcDir(g_stButtons[i]._pszUp), pPal);
				CBofBitmap *pDown = loadBitmap(BuildBarcDir(g_stButtons[i]._pszDown), pPal);
				CBofBitmap *pFocus = loadBitmap(BuildBarcDir(g_stButtons[i]._pszFocus), pPal);
				CBofBitmap *pDis = loadBitmap(BuildBarcDir(g_stButtons[i]._pszDisabled), pPal);

				_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
				_pButtons[i]->create(g_stButtons[i]._pszName, g_stButtons[i]._nLeft, g_stButtons[i]._nTop, g_stButtons[i]._nWidth, g_stButtons[i]._nHeight, this, g_stButtons[i]._nID);
				_pButtons[i]->hide();
			}
		}

		show();
		_pButtons[OFFBUT]->show();
		_pButtons[BCHELP]->show();
		_pButtons[BCQUIT]->show();

		// Fix drink already selected bug
		if (_pDrinkBox != nullptr) {
			_pDrinkBox->setSelectedItem(-1, false);
		}
		if (_pIngBox != nullptr) {
			_pIngBox->setSelectedItem(-1, false);
		}

		updateWindow();
	}

	CBagCursor::showSystemCursor();

	return rc;
}

ErrorCode SBarComputer::detach() {
	CBagCursor::hideSystemCursor();

	if (_pDrinkBuff != nullptr) {
		delete _pDrinkBuff;
		_pDrinkBuff = nullptr;
	}

	if (_pIngBuff != nullptr) {
		delete _pIngBuff;
		_pIngBuff = nullptr;
	}

	deleteListBox();
	deleteTextBox();

	if (_pDrinkList != nullptr) {
		delete _pDrinkList;
		_pDrinkList = nullptr;
	}

	if (_pIngList != nullptr) {
		delete _pIngList;
		_pIngList = nullptr;
	}

	// Destroy all buttons
	for (int i = 0; i < NUM_COMPBUTT; i++) {

		if (_pButtons[i] != nullptr) {
			delete _pButtons[i];
			_pButtons[i] = nullptr;
		}
	}

	_nDrinkSelect = -1;
	_nIngSelect = -1;

	_eMode = OFFMODE;

	CBagStorageDevWnd::detach();

	// Going into mr drinkmaster makes 1 turn go by
	VARMNGR->IncrementTimers();

	LogInfo("Detached SBarComputer");

	return ERR_NONE;
}

ErrorCode SBarComputer::readDrnkFile() {
	CBofString DrinkString(DRINKFILE);
	MACROREPLACE(DrinkString);

	// Open the text files
	CBofFile fpDrinkFile(DrinkString);
	if (fpDrinkFile.GetErrorCode() != ERR_NONE)
		return fpDrinkFile.GetErrorCode();

	// Check that buffers are null
	if (_pDrinkBuff) {
		delete _pDrinkBuff;
		_pDrinkBuff = nullptr;
	}

	// Allocate the buffers
	_pDrinkBuff = (char *)BofAlloc(fpDrinkFile.GetLength() + 1);
	if (_pDrinkBuff == nullptr)
		return ERR_MEMORY;

	// Read the text file into buffers
	fpDrinkFile.Read(_pDrinkBuff, fpDrinkFile.GetLength());

	// Get pointers indexing into Drink buffers
	char *pPosInBuff = _pDrinkBuff;
	while (pPosInBuff < _pDrinkBuff + fpDrinkFile.GetLength()) {
		SBarCompItem *pCompItem = new SBarCompItem();
		if (!pCompItem)
			error("Couldn't allocate a new SBarCompItem");

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
		while ((pPosInBuff < _pDrinkBuff + fpDrinkFile.GetLength()) && (*pPosInBuff != '$'))
			pPosInBuff++;

		pPosInBuff++;   // Increment past it

		_pDrinkList->addToTail(*pCompItem);
		delete pCompItem;
	}

	fpDrinkFile.close();
	return ERR_NONE;
}

ErrorCode SBarComputer::readIngFile() {
	CBofString IngString(INGRDFILE);
	MACROREPLACE(IngString);

	// Open the text files
	CBofFile fpIngFile(IngString);
	if (fpIngFile.GetErrorCode() != ERR_NONE)
		return fpIngFile.GetErrorCode();

	// Check that buffers are null
	if (_pIngBuff) {
		delete _pIngBuff;
		_pIngBuff = nullptr;
	}

	// Allocate the buffers
	_pIngBuff = (char *)BofAlloc(fpIngFile.GetLength() + 1);
	if (_pIngBuff == nullptr)
		return ERR_MEMORY;

	// Read the text file into buffers
	fpIngFile.Read(_pIngBuff, fpIngFile.GetLength());

	// Get pointers indexing into Ingredient buffers
	char *pPosInBuff = _pIngBuff;
	while (pPosInBuff < _pIngBuff + fpIngFile.GetLength()) {
		SBarCompItem *pCompItem = new SBarCompItem();
		if (!pCompItem)
			error("Couldn't allocate a new SBarCompItem");

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
		while ((pPosInBuff < _pIngBuff + fpIngFile.GetLength()) && (*pPosInBuff != '$'))
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
		Assert(_pTBox != nullptr);
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
	if (_pTBox) {
		delete _pTBox;
		_pTBox = nullptr;
	}
}

void SBarComputer::deleteListBox() {
	if (_pDrinkBox) {
		delete _pDrinkBox;
		_pDrinkBox = nullptr;
	}
	if (_pIngBox) {
		delete _pIngBox;
		_pIngBox = nullptr;
	}
}

ErrorCode SBarComputer::createDrinksListBox() {
	ErrorCode error = ERR_NONE;

	if (_pDrinkBox == nullptr) { // We need to create one

		_pDrinkBox = new CBofListBox();
		if (_pDrinkBox != nullptr) {
			error = _pDrinkBox->create("ListBox", &_compDisplay, this);
			if (error != ERR_NONE) {
				return error;
			}

			_pDrinkBox->hide();
			_pDrinkBox->setPointSize(12);
			_pDrinkBox->setItemHeight(20);

			CBofPalette *pPal = _pBackdrop->getPalette();
			byte PalIdx = pPal->getNearestIndex(RGB(255, 0, 0));

			_pDrinkBox->setHighlightColor(pPal->getColor(PalIdx));

			// Populate listbox
			int numItems = _pDrinkList->GetCount();
			for (int i = 0; i < numItems; ++i) {
				SBarCompItem CompItem = _pDrinkList->GetNodeItem(i);
				_pDrinkBox->addToTail(CBofString(CompItem._pItem), false);
			}
		} else {
			return ERR_MEMORY;
		}
	}

	return error;
}

ErrorCode SBarComputer::createIngListBox() {
	ErrorCode error = ERR_NONE;

	if (_pIngBox == nullptr) {
		// We need to create one
		_pIngBox = new CBofListBox();
		if (_pIngBox != nullptr) {
			error = _pIngBox->create("ListBox", &_compDisplay, this);
			if (error != ERR_NONE) {
				return error;
			}

			_pIngBox->hide();
			_pIngBox->setPointSize(12);
			_pIngBox->setItemHeight(20);

			CBofPalette *pPal = _pBackdrop->getPalette();
			byte PalIdx = pPal->getNearestIndex(RGB(255, 0, 0));

			_pIngBox->setHighlightColor(pPal->getColor(PalIdx));


			// Populate listbox
			int numItems = _pIngList->GetCount();
			for (int i = 0; i < numItems; ++i) {
				SBarCompItem CompItem = _pIngList->GetNodeItem(i);
				_pIngBox->addToTail(CBofString(CompItem._pItem), false);
			}

		} else
			return ERR_MEMORY;
	}

	return error;
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
	BofPlaySound(BuildBarcDir(ONAUDIO), SOUND_MIX);

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

		_pDrinkBox->hide();
		_pIngBox->hide();

		deleteTextBox();

		_eMode = OFFMODE;

		_pButtons[ONBUT]->hide();
		_pButtons[OFFBUT]->show();

		BofPlaySound(BuildBarcDir(ONAUDIO), SOUND_MIX);

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
			Assert(_nDrinkSelect != -1);
			CompItem = _pDrinkList->GetNodeItem(_nDrinkSelect);

		} else {

			Assert(_nIngSelect != -1);
			CompItem = _pIngList->GetNodeItem(_nIngSelect);
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
		SBarCompItem CompItem = _pDrinkList->GetNodeItem(_nDrinkSelect);

		// Deduct 1 Nugget from the player
		// Read in their total nuggets from game
		CBagVar *pVar = VARMNGR->GetVariable("NUGGETS");
		CBagVar *pVar2 = VARMNGR->GetVariable("HAVEDRINK");

		if (pVar) {
			int nCredits = pVar->GetNumValue();
			int nHaveDrink = pVar2->GetNumValue();

			// If the player is out of nuggets, then put up a text message.
			if (nCredits < 1) {
				CBofBitmap saveBackground(640, 480, (CBofPalette *)nullptr, false);
				saveBackground.captureScreen(this, &_compTextWindow);
				PaintBeveledText(this, &_compTextWindow, szBroke, FONT_15POINT, TEXT_NORMAL, RGB(255, 255, 255), JUSTIFY_WRAP, FORMAT_TOP_LEFT);

				WaitForInput();

				saveBackground.paint(this, &_compTextWindow);

			} else {
				CBagStorageDev *pSoldierSDev = nullptr;
				pSoldierSDev = SDEVMNGR->GetStorageDevice("SOLDIER_WLD");

				CBofBitmap saveBackgroundTwo(640, 480, (CBofPalette *)nullptr, false);
				saveBackgroundTwo.captureScreen(this, &_compTextWindow);

				// Don't allow him to order if he has other drinks in the Soldier CIC or stash
				if (pSoldierSDev) {
					bool bRefuse = false;
					if (pSoldierSDev->GetObject("DRINK1", true))
						bRefuse = true;
					else if (pSoldierSDev->GetObject("DRINK2", true))
						bRefuse = true;
					else if (pSoldierSDev->GetObject("DRINK3", true))
						bRefuse = true;
					else if (pSoldierSDev->GetObject("DRINK4", true))
						bRefuse = true;
					else if (pSoldierSDev->GetObject("DRINK5", true))
						bRefuse = true;
					else if (pSoldierSDev->GetObject("DRINK6", true))
						bRefuse = true;
					else if (pSoldierSDev->GetObject("DRINK7", true))
						bRefuse = true;
					else if (nHaveDrink > 0)
						bRefuse = true;
					else {
						pSoldierSDev->activateLocalObject(CompItem._pDrink);
						BofPlaySound(BuildBarcDir(ORDERAUDIO), SOUND_MIX);
						pVar->SetValue(nCredits - 1);
						pVar2->SetValue(1);
					}

					if (bRefuse) {
						PaintBeveledText(this, &_compTextWindow, szRefuse, FONT_15POINT, TEXT_NORMAL, RGB(255, 255, 255), JUSTIFY_WRAP, FORMAT_TOP_LEFT);
						WaitForInput();

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
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

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
		setList();
		break;
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
		LogInfo("\tClicked Quit");
		close();
		break;

	case BCHELP: {
		LogInfo("\tClicked Help");

		CBagel *pApp = CBagel::getBagApp();

		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {
				pWin->OnHelp(BuildBarcDir("BARCOMP.TXT"));
			}
		}
		}
		break;

	default:
		LogWarning(BuildString("Clicked Unknown Button with ID %d", pButton->getControlID()));
		break;
	}
}

void SBarComputer::onKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

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
	return formPath(BDCADIR, pszFile);
}

} // namespace SpaceBar
} // namespace Bagel
