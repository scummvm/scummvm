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

#include "bagel/spacebar/filter.h"
#include "bagel/spacebar/main_window.h"
#include "bagel/spacebar/spacebar.h"
#include "bagel/boflib/palette.h"
#include "bagel/boflib/string.h"
#include "bagel/spacebar/boflib/gfx/text.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/string_functions.h"

namespace Bagel {
namespace SpaceBar {

//
// Preprocessor definitions.
//
#define TRIFILTERBMP "$SBARDIR\\TRISECKS\\OBJ\\SUNGLASS.BMP"
#define VILDROIDTIPFILE "$SBARDIR\\VILDROID\\TIP.TXT"
#define VILDROIDADDFILE "$SBARDIR\\VILDROID\\ADD.TXT"
#define VILDROIDGRAFITTI "$SBARDIR\\VILDROID\\OBJ\\DANG.BMP"
#define VILDROIDSTATSTEXTSIZE 18
#define VILDROIDTIPSTEXTSIZE 18
#define VILDROIDCHIPTEXTWIDTH 480
#define VILDROIDTIPSCROLLSPEED  5
#define GREENCHIPFILE "$SBARDIR\\VILDROID\\GCHIP.TXT"
#define BLUECHIPFILE "$SBARDIR\\VILDROID\\BUCHIP.TXT"
#define BLUECHIPFILE "$SBARDIR\\VILDROID\\BUCHIP.TXT"
#define DISCEJECTSOUND "$SBARDIR\\VILDROID\\AUDIO\\OBJECTS\\DDTCPIC1.WAV"
#define THUNDERSOUND "$SBARDIR\\AUDITON\\AUDIO\\EVENTS\\LGHTN1.WAV"

// Static function prototypes.
//
static bool VildroidFilter(CBofBitmap *pBmp, CBofRect *pRect);
static bool TriFilter(CBofBitmap *pBmp, CBofRect *pRect);
static bool ZzazzlFilter(CBofBitmap *pBmp, CBofRect *pRect);
static bool HalucinateFilter(CBofBitmap *pBmp, CBofRect *pRect);
static bool LightningFilter(CBofBitmap *pBmp, CBofRect *pRect);

// Static data definitions.
//
static CBofBitmap *pTipBmp = nullptr;
static CBofBitmap *pChipBmp = nullptr;
static CBofBitmap *pGrafittiBmp = nullptr;
static CBofBitmap *pTriBmp = nullptr;
static bool initDone = false;
static bool triinitDone = false;
static bool lightninginitDone = false;
static char *buff = nullptr;
static const char *kPrecipString = "Chance Of Precipitation: ";
static const char *kDustString = "Dust Level: ";
static const char *kRadioOnString = "Internal Radio Receiver: On";
static const char *kRadioOffString = "Internal Radio Receiver: Off";
static uint32 dwTimeOfNextBolt = 0;
static CBofSound *pThunder = nullptr;
static const char *voiceNameArray[] = {
	"Voice ID: None",
	"Voice ID: Veeblecoach",
	"Voice ID: Pictureguarder",
	"Voice ID: Commentmaker",
	"Voice ID: Hairtrimmer",
	"Voice ID: President",
	"Voice ID: P.A. Announcer",
	"Voice ID: Playdescriber and Coloradder",
	"Voice ID: Unable to process multiple signals.",
	"Voice ID:"
};

static CBagVar *g_pHudOn = nullptr;
static CBagVar *g_pDGrafiti = nullptr;
static CBagVar *g_pDRadio = nullptr;
static CBagVar *g_pPrecip = nullptr;
static CBagVar *g_pPrecDecimal = nullptr;
static CBagVar *g_pDust = nullptr;
static CBagVar *g_pDustDecimal = nullptr;
static CBagVar *g_pDVoiceID = nullptr;
static CBagVar *g_pDChipID = nullptr;
static CBagVar *g_pTDig1 = nullptr;
static CBagVar *g_pTDig2 = nullptr;
static CBagVar *g_pTDig3 = nullptr;
static CBagVar *g_pTDig4 = nullptr;

static uint32 waitCount;

// Filter initialization code.
//
void vilInitFilters(CBofBitmap *pBmp) {
	if (!initDone) {
		char szCString[256];
		CBofString cString(szCString, 256);
		cString = VILDROIDTIPFILE;

		fixPathName(cString);
		CBofFile nfile(cString, CBF_READONLY);
		buff = new char[nfile.getLength() + 1];
		memset(buff, 0, nfile.getLength() + 1);
		nfile.read(buff, nfile.getLength());
		strreplaceChar(buff, '\n', ' ');
		strreplaceChar(buff, '\r', ' ');
		pTipBmp = new CBofBitmap((nfile.getLength() + 1) * 7, 20, pBmp->getPalette());

		CBofRect rct(0, 0, (nfile.getLength() + 1) * 7, 20);
		paintText(pTipBmp, &rct, buff, VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(107, 0, 255), JUSTIFY_LEFT, FORMAT_DEFAULT);
		nfile.close();
		delete[] buff;

		// Graffiti bitmap.
		//
		char szBString[256];
		CBofString cBString(szBString, 256);
		cBString = VILDROIDGRAFITTI;
		fixPathName(cBString);
		pGrafittiBmp = new CBofBitmap(cBString, pBmp->getPalette());

		// No initialization of the pChipBmp is done here - it's
		// done on the fly inside VildroidFilter.
		//
		// Init the timer check for Chip info display
		waitCount = 0;

		// Record the fact that the filter bitmaps have been created.
		initDone = true;
	}
}

void triInitFilters(CBofBitmap *pBmp) {
	// Trisecks sunglasses bitmap.
	char szTriStr[256];
	CBofString cTriStr(szTriStr, 256);
	cTriStr = TRIFILTERBMP;
	fixPathName(cTriStr);
	pTriBmp = new CBofBitmap(cTriStr, pBmp->getPalette());
	triinitDone = true;
}

void lightningInitFilters() {
	if (!lightninginitDone) {
		char szThunderFileName[256];
		CBofString cThunderFileName(szThunderFileName, 256);
		cThunderFileName = THUNDERSOUND;
		fixPathName(cThunderFileName);
		pThunder = new CBofSound(CBofApp::getApp()->getMainWindow(), szThunderFileName, SOUND_MIX);
		lightninginitDone = true;
	}
}

void destroyFilters() {
	delete pTipBmp;
	pTipBmp = nullptr;

	delete pGrafittiBmp;
	pGrafittiBmp = nullptr;

	// clean up trisecks bmp
	delete pTriBmp;
	pTriBmp = nullptr;

	// Chip bitmap is destroyed here is the cleanup function because
	// presumably we're being called when the game is ending.
	delete pChipBmp;
	pChipBmp = nullptr;

	// Clean up the lightning filter.
	delete pThunder;
	pThunder = nullptr;

	// Record the fact that the filter bitmaps need to be instantiated
	// before they can be used again.
	initDone = false;
	triinitDone = false;
	lightninginitDone = false;
}

bool doFilters(const uint16 nFilterId, CBofBitmap *pBmp, CBofRect *pRect) {
	bool bReturnValue = false;

	if ((nFilterId & kTriFilter) != 0) {
		bReturnValue = (TriFilter(pBmp, pRect) || bReturnValue);
	}

	if ((nFilterId & kHalucinateFilter) != 0) {
		bReturnValue = (HalucinateFilter(pBmp, pRect) || bReturnValue);
	}

	if ((nFilterId & kVildroidFilter) != 0) {
		bReturnValue = (VildroidFilter(pBmp, pRect) || bReturnValue);
	}

	if ((nFilterId & kZzazzlFilter) != 0) {
		bReturnValue = (ZzazzlFilter(pBmp, pRect) || bReturnValue);
	}

	if ((nFilterId & kLightningFilter) != 0) {
		bReturnValue = (LightningFilter(pBmp, pRect) || bReturnValue);
	}

	return bReturnValue;
}

static void GetVilVars() {
	char szBuf[256];
	szBuf[0] = '\0';
	CBofString cStr(szBuf, 256);

	// check if HUD is on
	cStr = "HUDON";
	g_pHudOn = g_VarManager->getVariable(cStr);

	cStr = "DGRAFITTI";
	g_pDGrafiti = g_VarManager->getVariable(cStr);

	cStr = "DRADIO";
	g_pDRadio = g_VarManager->getVariable(cStr);

	cStr = "PRECIPITATION";
	g_pPrecip = g_VarManager->getVariable(cStr);

	cStr = "PRECDECIMAL";
	g_pPrecDecimal = g_VarManager->getVariable(cStr);

	cStr = "DUST";
	g_pDust = g_VarManager->getVariable(cStr);

	cStr = "DUSTDECIMAL";
	g_pDustDecimal = g_VarManager->getVariable(cStr);

	cStr = "DVOICEID";
	g_pDVoiceID = g_VarManager->getVariable(cStr);

	cStr = "DCHIPID";
	g_pDChipID = g_VarManager->getVariable(cStr);

	cStr = "TDIG1";
	g_pTDig1 = g_VarManager->getVariable(cStr);

	cStr = "TDIG2";
	g_pTDig2 = g_VarManager->getVariable(cStr);

	cStr = "TDIG3";
	g_pTDig3 = g_VarManager->getVariable(cStr);

	cStr = "TDIG4";
	g_pTDig4 = g_VarManager->getVariable(cStr);
}


// Vildroid filter.
static bool VildroidFilter(CBofBitmap *pBmp, CBofRect *pRect) {
	if (!initDone) {
		vilInitFilters(pBmp);
	}

	// If get vars for 1st time
	if (g_engine->g_getVilVarsFl) {
		GetVilVars();
		g_engine->g_getVilVarsFl = false;
	}

	if (pRect != nullptr) {
		g_engine->viewRect = *pRect;

	} else {
		// A null rectangle implies that we're going to filter the
		// entire screen.  Take the bitmap's rectangle and intersect
		// that with the rectangle for the viewport. Filter the resulting
		// rectangle.

		// Now intersect that rectangle with the bitmap's rectangle.
		g_engine->viewRect.intersectRect(pBmp->getRect(), g_engine->viewPortRect);
	}

	if (g_pHudOn->getNumValue() == 1) {
		if (g_pDGrafiti->getNumValue()) {
			CBofRect SrcRect(pGrafittiBmp->getRect());
			pGrafittiBmp->paint(pBmp, g_engine->viewRect.left, g_engine->viewRect.top, &SrcRect, 1);
		}

		CBofRect rect(g_engine->viewRect.left, g_engine->viewRect.top, g_engine->viewRect.right, g_engine->viewRect.top + 20);

		// Display internal radio setting.
		if (g_pDRadio->getNumValue()) {
			paintText(pBmp, &rect, kRadioOnString, VILDROIDSTATSTEXTSIZE, TEXT_BOLD, RGB(0, 255, 6), JUSTIFY_LEFT, FORMAT_DEFAULT);
		} else {
			paintText(pBmp, &rect, kRadioOffString, VILDROIDSTATSTEXTSIZE, TEXT_BOLD, RGB(0, 255, 6), JUSTIFY_LEFT, FORMAT_DEFAULT);
		}

		// Display chance of precipitation.
		char szVBuff2[256];
		Common::sprintf_s(szVBuff2, "%s%d.%d%%", kPrecipString, g_pPrecip->getNumValue(), g_pPrecDecimal->getNumValue());
		CBofRect cleanRect((g_engine->viewRect.right - 250), g_engine->viewRect.top, g_engine->viewRect.right - 5, g_engine->viewRect.top + 20);
		paintText(pBmp, &cleanRect, szVBuff2, VILDROIDSTATSTEXTSIZE, TEXT_BOLD, RGB(0, 255, 6), JUSTIFY_RIGHT, FORMAT_DEFAULT);

		// Display dust level.
		Common::sprintf_s(szVBuff2, "%s%d.%dp/cmm", kDustString, g_pDust->getNumValue(), g_pDustDecimal->getNumValue());
		cleanRect.bottom += 20;
		cleanRect.top += 20;
		paintText(pBmp, &cleanRect, szVBuff2, VILDROIDSTATSTEXTSIZE, TEXT_BOLD, RGB(0, 255, 6), JUSTIFY_RIGHT, FORMAT_DEFAULT);

		int voiceId = g_pDVoiceID->getNumValue();
		const char *pVoiceIdString = voiceNameArray[voiceId];

		rect.top += 20;
		rect.bottom += 20;
		paintText(pBmp, &rect, pVoiceIdString, VILDROIDSTATSTEXTSIZE, TEXT_BOLD, RGB(255, 193, 0), JUSTIFY_LEFT, FORMAT_DEFAULT);

		CBofRect srcTipRect;

		srcTipRect.left = 0;
		srcTipRect.right = g_engine->viewRect.right - g_engine->viewRect.left;
		srcTipRect.top = 0;
		srcTipRect.bottom = 20;

		pTipBmp->scrollLeft(VILDROIDTIPSCROLLSPEED, nullptr);
		rect.left = g_engine->viewRect.left;
		rect.right = g_engine->viewRect.right;
		rect.top = g_engine->viewRect.bottom - 20;
		rect.bottom = g_engine->viewRect.bottom;
		pTipBmp->paint(pBmp, &rect, &srcTipRect, 0);

		// moved up here to use chipID later on bar
		int chipID = g_pDChipID->getNumValue();

		if (CMainWindow::chipdisp == false) {
			switch (chipID) {
			case 0: {
				break;
			}
			case 1: {
				delete pChipBmp;

				char szCString[256];
				CBofString cString(szCString, 256);
				cString = GREENCHIPFILE;

				fixPathName(cString);
				CBofFile nfile(cString, CBF_READONLY);
				buff = new char[nfile.getLength() + 1];
				memset(buff, 0, nfile.getLength() + 1);
				pChipBmp = new CBofBitmap(VILDROIDCHIPTEXTWIDTH, 300, pBmp->getPalette());
				nfile.read(buff, nfile.getLength());
				CBofRect txtRect(0, 0, VILDROIDCHIPTEXTWIDTH, 18);
				uint32 test = 0;
				char *pBuff1 = buff;
				char *pBuff2 = buff;
				bool bFinished = false;

				while (bFinished != true) {
					while (*pBuff1 != '\n' && *pBuff1 != '\r' && bFinished != true) {
						test++;
						pBuff1++;
						if (test >= nfile.getLength())
							bFinished = true;

					}
					if (test >= nfile.getLength())
						bFinished = true;
					else {
						*pBuff1 = '\0';
						paintText(pChipBmp, &txtRect, pBuff2, VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
						pBuff2 = pBuff1 + 1;
						txtRect.top += 18;
						txtRect.bottom += 18;
					}
				}
				nfile.close();
				CMainWindow::chipdisp = true;
				delete[] buff;
				break;
			}
			case 2: {
				delete pChipBmp;

				char szCString[256];
				CBofString cString(szCString, 256);
				cString = BLUECHIPFILE;

				fixPathName(cString);
				CBofFile nfile(cString, CBF_READONLY);
				buff = new char[nfile.getLength() + 1];
				memset(buff, 0, nfile.getLength() + 1);
				pChipBmp = new CBofBitmap(VILDROIDCHIPTEXTWIDTH, 300, pBmp->getPalette());
				nfile.read(buff, nfile.getLength());
				CBofRect txtRect(0, 0, VILDROIDCHIPTEXTWIDTH, 18);
				uint32 test = 0;
				char *pBuff1 = buff;
				char *pBuff2 = buff;
				bool bFinished = false;

				while (bFinished != true) {
					while (*pBuff1 != '\n' && *pBuff1 != '\r' && bFinished != true) {
						test++;
						pBuff1++;
						if (test >= nfile.getLength())
							bFinished = true;

					}
					if (test >= nfile.getLength())
						bFinished = true;
					else {
						*pBuff1 = '\0';
						paintText(pChipBmp, &txtRect, pBuff2, VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
						pBuff2 = pBuff1 + 1;
						txtRect.top += 18;
						txtRect.bottom += 18;
					}
				}
				nfile.close();
				CMainWindow::chipdisp = true;
				delete[] buff;
				break;
			}
			case 3: {
				if (pChipBmp != nullptr) {
					delete pChipBmp;
					pChipBmp = nullptr;
				}
				CMainWindow::chipdisp = true;
				break;
			}
			}
		} else if (g_pDChipID->getNumValue() == 3) {
			CBofRect txtRect(g_engine->viewRect);
			uint32 lDiff;
			uint32 timer = getTimer();

			if (waitCount == 0)
				waitCount = timer;

			lDiff = (timer - waitCount) / 1000;

			switch (lDiff) {
			case 0:
			case 1:
				paintText(pBmp, &txtRect, "VIMM chip accepted", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 2:
			case 3:
				paintText(pBmp, &txtRect, "Mark IV OS initialized", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 4:
			case 5:
				paintText(pBmp, &txtRect, "Reading flashbank....", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 6:
			case 7:
				paintText(pBmp, &txtRect, "Process completed", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 8:
			case 9:
				paintText(pBmp, &txtRect, "1. General Knowledge", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 10:
			case 11:
				paintText(pBmp, &txtRect, "2. Vildroid History", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 12:
			case 13:
				paintText(pBmp, &txtRect, "3. Recipe DataBank J12", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 14:
			case 15:
				paintText(pBmp, &txtRect, "4. Personal Database C238-87565", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 16:
			case 17:
				paintText(pBmp, &txtRect, "5. Veebleball Strategy Kit", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 18:
			case 19:
				paintText(pBmp, &txtRect, "6. Termite Infestation", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 20:
			case 21:
				paintText(pBmp, &txtRect, "7. Latex Glove Sizing Chart", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 22:
			case 23:
				paintText(pBmp, &txtRect, "8. Cleaning Dead Bugs Out From Behind Storm Windows", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 24:
			case 25:
				paintText(pBmp, &txtRect, "9. Algorithms 1233B thru 1245DD: 12 Steps to Whiter Tiles", VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0), JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;

			case 26:
			case 27:
			case 28:
			case 29: {
				paintText(pBmp, &txtRect,
				          buildString("You have been assigned Maintenance Mode PIN: %d%d%d%d",
				                      g_pTDig1->getNumValue(), g_pTDig2->getNumValue(),
				                      g_pTDig3->getNumValue(), g_pTDig4->getNumValue()),
				          VILDROIDTIPSTEXTSIZE, TEXT_BOLD, RGB(255, 7, 0),
				          JUSTIFY_CENTER, FORMAT_TOP_CENTER);
				break;
			}

			default: {
				CMainWindow::chipdisp = false;
				waitCount = 0;
				g_pDChipID->setValue(0);
				break;
			}

			}

		} else {
			// Chip is Green or Blue (ID 1 or 2)
			if (pChipBmp != nullptr) {
				int rdef = g_engine->viewRect.width() - VILDROIDCHIPTEXTWIDTH;
				int tdef = g_engine->viewRect.height() - 300;
				CBofRect tmprct(0, 0, VILDROIDCHIPTEXTWIDTH, 300);

				pChipBmp->paint(pBmp, ((rdef / 2) + g_engine->viewRect.left), (tdef + g_engine->viewRect.top), &tmprct, 0);
			}

			if (waitCount == 0)
				waitCount = getTimer();

			if ((getTimer() - waitCount) > 15000) {
				CMainWindow::chipdisp = false;
				waitCount = 0;
				g_pDChipID->setValue(0);

				char szCString[256];
				CBofString cString(szCString, 256);
				cString = DISCEJECTSOUND;
				fixPathName(cString);
				BofPlaySound(cString, SOUND_WAVE | SOUND_ASYNCH);
				CBagStorageDev *pWieldSDev = nullptr;
				pWieldSDev = g_SDevManager->getStorageDevice("BWIELD_WLD");
				if (pWieldSDev == nullptr)
					CBofError::fatalError(ERR_UNKNOWN, "Unable to get storage device 'BWIELD_WLD'");

				if (chipID == 1)
					pWieldSDev->activateLocalObject("GREENCHIP");
				else
					pWieldSDev->activateLocalObject("BLUECHIP");
			}
		}
	}

	return true;
}


// Trisecks filter
static bool TriFilter(CBofBitmap *pBmp, CBofRect *pRect) {
	if (!triinitDone) {
		triInitFilters(pBmp);
	}

	if (pRect != nullptr) {
		g_engine->viewRect = *pRect;
	} else {

		// A null rectangle implies that we're going to filter the
		// entire screen.  Take the bitmap's rectangle and intersect
		// that with the rectangle for the viewport. Filter the resulting
		// rectangle.

		// Now intersect that rectangle with the bitmap's rectangle.
		g_engine->viewRect.intersectRect(pBmp->getRect(), g_engine->viewPortRect);
	}

	if (g_VarManager->getVariable("GLASSESON")->getNumValue()) {
		CBofRect SrcRect(pTriBmp->getRect());
		pTriBmp->paint(pBmp, g_engine->viewRect.left, g_engine->viewRect.top, &SrcRect, 1);
	}
	return true;
}

// Zzazzlvision filter.
static bool ZzazzlFilter(CBofBitmap *pBmp, CBofRect *pRect) {
	char szZBuff[256];
	CBofString zStr(szZBuff, 256);
	bool bZzazzlVision = false;

	if (pRect != nullptr) {
		g_engine->viewRect = *pRect;
	} else {
		// A null rectangle implies that we're going to filter the
		// entire screen.  Take the bitmap's rectangle and intersect
		// that with the rectangle for the viewport. Filter the resulting
		// rectangle.

		// Now intersect that rectangle with the bitmap's rectangle.
		g_engine->viewRect.intersectRect(pBmp->getRect(), g_engine->viewPortRect);
	}

	zStr = "ZZAZZLVISION";
	CBagVar *pVar = g_VarManager->getVariable(zStr);
	if (pVar != nullptr) {

		bZzazzlVision = true;
		if (pVar->getNumValue() > 0) {
			CMainWindow::setZzazzlVision(true);             // zzazzl paint is on in the script
			int dx = g_engine->viewRect.width() / 3;  // + 1;
			int dy = g_engine->viewRect.height() / 3; // + 1;
			CBofPalette *pPal = pBmp->getPalette();

			CBofBitmap *pMiniBitmap = new CBofBitmap(dx, dy, pPal);

			CBofRect srcRect(g_engine->viewRect);
			CBofRect dstRect = pMiniBitmap->getRect();
			pBmp->paint(pMiniBitmap, &dstRect, &srcRect);

			CBofRect &filterRect = CMainWindow::getFilterRect();
			filterRect.setRect(g_engine->viewRect.left, g_engine->viewRect.top, g_engine->viewRect.left + dx, g_engine->viewRect.top + dy);

			int j, x;
			int y = g_engine->viewRect.top;
			for (int i = 0; i < 3; ++i) {
				if (i == 1) {
					j = 0;
					x = g_engine->viewRect.left;
				} else {
					j = 1;
					x = g_engine->viewRect.left + (dx >> 1);
				}
				for (; j < 3; ++j) {
					pMiniBitmap->paint(pBmp, x, y);
					x += dx;
				}
				y += dy;
			}

			delete pMiniBitmap;
		}
	}

	return bZzazzlVision;
}

// Halucination filter.
static bool HalucinateFilter(CBofBitmap *pBmp, CBofRect *pRect) {
	char szHBuff[256];
	CBofString hStr(szHBuff, 256);
	bool bHallucinating = false;

	if (pRect != nullptr) {
		g_engine->viewRect = *pRect;
	} else {
		// A null rectangle implies that we're going to filter the
		// entire screen.  Take the bitmap's rectangle and intersect
		// that with the rectangle for the viewport. Filter the resulting
		// rectangle.

		// Now intersect that rectangle with the bitmap's rectangle.
		g_engine->viewRect.intersectRect(pBmp->getRect(), g_engine->viewPortRect);
	}

	hStr = "HALLUCINATE";

	if (g_VarManager->getVariable(hStr)->getNumValue() > 0) {
		bHallucinating = true;
		CBofPalette *pPal = pBmp->getPalette();
		CBofBitmap *pTempBitmap = new CBofBitmap(g_engine->viewRect.width(), g_engine->viewRect.height(), pPal);
		CBofRect tempRect = pTempBitmap->getRect();
		CBofRect srcRect(g_engine->viewRect);
		CBofRect dstRect = tempRect;
		pBmp->paint(pTempBitmap, &dstRect, &srcRect);   // Copy the Screen's Bmp into Temp
		int nShiftAmount = 0;

		// Step through strips of bmp
		for (int y = tempRect.top; y < tempRect.bottom; y += 4) {
			srcRect.setRect(0, y, tempRect.right - nShiftAmount, y + 4); // Get everything over one
			dstRect.setRect(g_engine->viewRect.left + nShiftAmount, g_engine->viewRect.top + y,
			                g_engine->viewRect.right, (g_engine->viewRect.top + y) + 4);
			pTempBitmap->paint(pBmp, &dstRect, &srcRect);
			srcRect.setRect(tempRect.right - nShiftAmount, y, tempRect.right, y + 4);
			dstRect.setRect(g_engine->viewRect.left, g_engine->viewRect.top + y,
			                g_engine->viewRect.left + nShiftAmount, (g_engine->viewRect.top + y) + 4);
			pTempBitmap->paint(pBmp, &dstRect, &srcRect);
			// I'm sure there's some nerdy math formula that does
			//...this all pretty-like -- will fix it if it comes to me
			// Change the amount of shift depending on the row we're in
			if (((y / 4) % 20) < 3)
				nShiftAmount += 2;
			else if (((y / 4) % 20) < 8)
				nShiftAmount += 5;
			else if (((y / 4) % 20) < 10)
				nShiftAmount += 3;
			else if (((y / 4) % 20) < 15)
				nShiftAmount -= 3;
			else if (((y / 4) % 20) < 18)
				nShiftAmount -= 5;
			else
				nShiftAmount -= 2;
		}

		delete pTempBitmap;
	}

	return bHallucinating;
}

static bool LightningFilter(CBofBitmap *pBmp, CBofRect *pRect) {
	assert(pBmp != nullptr);

	bool bReturnValue = true;

	// See if lightning is on in this storage device.
	char szVBuf[256];
	CBofString vStr(szVBuf, 256);
	vStr = "LIGHTNINGDELAY";
	CBagVar *pVar = g_VarManager->getVariable(vStr);
	int nLightningDelay = 0;

	if (pVar != nullptr) {
		nLightningDelay = pVar->getNumValue();
	}

	if (nLightningDelay > 0) {
		if (!lightninginitDone) {
			lightningInitFilters();
		}

		// Check to see if it's time to "flash" a bolt of lightning.
		if (dwTimeOfNextBolt < getTimer()) {
			// Time to paint the effect of a lightning bolt
			// flash and play the sound of a thunderbolt.
			if (pRect != nullptr) {
				g_engine->viewRect = *pRect;
			} else {
				// A null rectangle implies that we're going to filter the
				// entire screen.  Take the bitmap's rectangle and intersect
				// that with the rectangle for the viewport. Filter the resulting
				// rectangle.

				// Now intersect that rectangle with the bitmap's rectangle.
				g_engine->viewRect.intersectRect(pBmp->getRect(), g_engine->viewPortRect);
			}

			// To give the illusion of a lightning bolt strike,
			// we'll spin through the bitmap and coerce pixels which are black
			// to be white and pixels which are not black be black.
			int nWidth = g_engine->viewRect.width();
			int nHeight = g_engine->viewRect.height();

			// Need to lock down this bitmap to make sure we can get it's bits
			pBmp->lock();

			for (int i = 0; i < nHeight; i++) {
				byte *pPixel = pBmp->getPixelAddress(g_engine->viewRect.left, g_engine->viewRect.top + i);

				// Fix this such that it uses predefined constants, makes for much
				// better lightning on the mac.
				for (int j = 0; j < nWidth; j++, pPixel++) {
					if (*pPixel != COLOR_BLACK) {
						*pPixel = COLOR_WHITE;
					}
				}
			}

			// Don't need a lock on it anymore
			pBmp->unlock();

			// Let's hear the thunder!
			pThunder->play();

			// Set the time that the next bolt of lightning will occur.
			// If only nature were so precise...
			dwTimeOfNextBolt = getTimer() + nLightningDelay;
		}

	}
	return bReturnValue;
}

} // namespace SpaceBar
} // namespace Bagel
