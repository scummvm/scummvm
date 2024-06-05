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

#include "bagel/spacebar/master_win.h"
#include "bagel/baglib/zoom_pda.h"
#include "bagel/spacebar/computer.h"
#include "bagel/spacebar/sraf_computer.h"
#include "bagel/boflib/std_keys.h"
#include "bagel/baglib/storage_dev_bmp.h"
#include "bagel/baglib/wield.h"
#include "bagel/spacebar/bibble_window.h"
#include "bagel/spacebar/nav_window.h"
#include "bagel/baglib/chat_wnd.h"
#include "bagel/spacebar/game_defs.h"
#include "bagel/spacebar/slot_wnd.h"
#include "bagel/spacebar/vid_wnd.h"
#include "bagel/spacebar/full_wnd.h"
#include "bagel/spacebar/bib_odds_wnd.h"
#include "bagel/baglib/inv.h"
#include "bagel/baglib/moo.h"
#include "bagel/baglib/log_msg.h"
#include "bagel/baglib/event_sdev.h"
#include "bagel/boflib/log.h"
#include "bagel/spacebar/filter.h"

namespace Bagel {
namespace SpaceBar {

static const char *GetBuildVersion() {
	return buildString("Version: %s, %s", __DATE__, __TIME__);
}

CBagStorageDev *CSBarMasterWin::onNewStorageDev(const CBofString &typeStr) {
	assert(isValidObject(this));

	CBagStorageDev *pSDev;

	if (!typeStr.find("PDA")) {
		pSDev = new CBagPDA();

	} else if (!typeStr.find("WIELD")) {
		pSDev = new CBagWield();

	} else if (!typeStr.find("BMP")) {
		pSDev = new CBagStorageDevBmp();

	} else if (!typeStr.find("DLG")) {
		pSDev = new CBagStorageDevDlg();

	} else if (!typeStr.find("BARCOMP")) {
		pSDev = new SBarComputer();
		pSDev->setCustom(true);

	} else if (!typeStr.find("SRACOMP")) {
		pSDev = new SrafComputer();
		pSDev->setExitOnEdge(10);

	} else if (!typeStr.find("VIDEQUIP")) {
		pSDev = new SBarVidWnd();
		pSDev->setCloseup(true);
		pSDev->setExitOnEdge(80);
		pSDev->setCustom(true);

	} else if (!typeStr.find("FULLSCREEN2")) {
		pSDev = new SBarFullWnd();
		pSDev->setCloseup(true);
		((SBarFullWnd *)pSDev)->_bAllowEventWorld = false;

	} else if (!typeStr.find("FULLSCREEN")) {
		pSDev = new SBarFullWnd();
		pSDev->setCloseup(true);

	} else if (!typeStr.find("SBARSLOT")) {
		pSDev = new SBarSlotWnd();
		pSDev->setCustom(true);

	} else if (!typeStr.find("BIBODDS")) {
		pSDev = new SBarBibOddsWnd();
		pSDev->setCloseup(true);
		pSDev->setCustom(true);

	} else if (!typeStr.find("INVWLD")) {
		pSDev = new CBagInv();

	} else if (!typeStr.find("MOOWLD")) {
		pSDev = new CBagMoo();

	} else if (!typeStr.find("ZOOMPDA")) {
		pSDev = new SBZoomPda();
		pSDev->setCloseup(true);

	} else if (!typeStr.find("PLAYBIBBLE")) {
		pSDev = new CBibbleWindow();
		pSDev->setCustom(true);

	} else if (!typeStr.find("PLAYNAV")) {
		pSDev = new CNavWindow();

		// delineate cic's
	} else if (!typeStr.find("CIC")) {
		pSDev = new GAMEWINDOW();
		pSDev->setCloseup(true);
		pSDev->setCIC(true);
		pSDev->setExitOnEdge(80);

	} else if (!typeStr.find("CLOSEUP")) {
		pSDev = new GAMEWINDOW();
		pSDev->setCloseup(true);
		pSDev->setExitOnEdge(80);

	} else if (!typeStr.find("CHAT")) {
		pSDev = new CBagChatWnd();
		pSDev->setCloseup(true);

	} else if (!typeStr.find("EVENT")) {        // EVT STUFF
		pSDev = new CBagEventSDev();

	} else if (!typeStr.find("TURNEVENT")) {    // turn based storage device
		pSDev = new CBagTurnEventSDev();

	} else if (!typeStr.find("LOG")) {          // PDA LOG STUFF
		pSDev = new CBagLog();

	} else {
		pSDev = new GAMEWINDOW();
	}

	return pSDev;
}


CBagStorageDev *CSBarMasterWin::onNewStorageDev(int nType) {
	assert(isValidObject(this));

	CBagStorageDev *pSDev = nullptr;

	switch (nType) {
	case SDEV_PDA:
		pSDev = new CBagPDA();
		break;

	case SDEV_WIELD:
		pSDev = new CBagWield();
		break;

	case SDEV_BMP:
		pSDev = new CBagStorageDevBmp();
		break;

	case SDEV_DLG:
		pSDev = new CBagStorageDevDlg();
		break;

	case SDEV_CLOSEP:
		pSDev = new GAMEWINDOW();
		pSDev->setExitOnEdge(10);
		break;

	case SDEV_WND:
		pSDev = new GAMEWINDOW();
		break;

	default:
		logError(buildString("FAILED to init storage device of type %d", nType));
		break;
	};

	return pSDev;
}


void CSBarMasterWin::onKeyHit(uint32 lKey, uint32 lRepCount) {
	assert(isValidObject(this));

	if (lKey == BKEY_ALT_v) {
		bofMessageBox(GetBuildVersion(), "Version");
	} else {
		CBagMasterWin::onKeyHit(lKey, lRepCount);
	}
}

void CSBarMasterWin::onNewFilter(CBagStorageDev *pSDev, const CBofString &typeStr) {
	assert(isValidObject(this));

	FilterFxn filterFunction = nullptr;
	int filterId = pSDev->getFilterId();

	if (filterId != 0) {
		filterFunction = &doFilters;
	}

	if (!typeStr.find("PDA")) {
	} else if (!typeStr.find("WIELD")) {
	} else if (!typeStr.find("BMP")) {
	} else if (!typeStr.find("DLG")) {
		pSDev->onSetFilter(filterFunction);
	} else if (!typeStr.find("BARCOMP")) {
	} else if (!typeStr.find("SRACOMP")) {
	} else if (!typeStr.find("VIDEQUIP")) {
	} else if (!typeStr.find("FULLSCREEN")) {
	} else if (!typeStr.find("SBARSLOT")) {
	} else if (!typeStr.find("BIBODDS")) {
	} else if (!typeStr.find("INVWLD")) {
	} else if (!typeStr.find("MOOWLD")) {
	} else if (!typeStr.find("ZOOMPDA")) {
	} else if (!typeStr.find("PLAYBIBBLE")) {
	} else if (!typeStr.find("PLAYNAV")) {
	} else if (!typeStr.find("CLOSEUP")) {
		pSDev->onSetFilter(filterFunction);
	} else if (!typeStr.find("CHAT")) {
		pSDev->onSetFilter(filterFunction);
	} else if (!typeStr.find("EVENT")) {
	} else if (!typeStr.find("TURNEVENT")) {
	} else if (!typeStr.find("LOG")) {
	} else {
		// The game window
		pSDev->onSetFilter(filterFunction);
	}
}

void CSBarMasterWin::onNewFilter(CBagStorageDev *pSDev, const int nType) {
	assert(isValidObject(this));

	FilterFxn filterFunction = nullptr;
	int filterId = pSDev->getFilterId();

	if (filterId != 0) {
		filterFunction = &doFilters;
	}

	switch (nType) {
	case SDEV_PDA:
	case SDEV_WIELD:
	case SDEV_BMP:
		break;

	case SDEV_DLG:
	case SDEV_CLOSEP:
	case SDEV_WND:
		pSDev->onSetFilter(filterFunction);
		break;

	default:
		break;
	}
}

} // namespace SpaceBar
} // namespace Bagel
