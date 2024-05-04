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

CBagStorageDev *CSBarMasterWin::onNewStorageDev(const CBofString &typestr) {
	assert(isValidObject(this));

	CBagStorageDev *pSDev;

	if (!typestr.find("PDA")) {
		pSDev = new CBagPDA();

	} else if (!typestr.find("WIELD")) {
		pSDev = new CBagWield();

	} else if (!typestr.find("BMP")) {
		pSDev = new CBagStorageDevBmp();

	} else if (!typestr.find("DLG")) {
		pSDev = new CBagStorageDevDlg();

	} else if (!typestr.find("BARCOMP")) {
		pSDev = new SBarComputer();
		pSDev->SetCustom(true);

	} else if (!typestr.find("SRACOMP")) {
		pSDev = new SrafComputer();
		pSDev->SetExitOnEdge(10);

	} else if (!typestr.find("VIDEQUIP")) {
		pSDev = new SBarVidWnd();
		pSDev->SetCloseup(true);
		pSDev->SetExitOnEdge(80);
		pSDev->SetCustom(true);

	} else if (!typestr.find("FULLSCREEN2")) {
		pSDev = new SBarFullWnd();
		pSDev->SetCloseup(true);
		((SBarFullWnd *)pSDev)->_bAllowEventWorld = false;

	} else if (!typestr.find("FULLSCREEN")) {
		pSDev = new SBarFullWnd();
		pSDev->SetCloseup(true);

	} else if (!typestr.find("SBARSLOT")) {
		pSDev = new SBarSlotWnd();
		pSDev->SetCustom(true);

	} else if (!typestr.find("BIBODDS")) {
		pSDev = new SBarBibOddsWnd();
		pSDev->SetCloseup(true);
		// pSDev->SetExitOnEdge(10);
		pSDev->SetCustom(true);

	} else if (!typestr.find("INVWLD")) {
		pSDev = new CBagInv();

	} else if (!typestr.find("MOOWLD")) {
		pSDev = new CBagMoo();

	} else if (!typestr.find("ZOOMPDA")) {
		pSDev = new SBZoomPda();
		pSDev->SetCloseup(true);

	} else if (!typestr.find("PLAYBIBBLE")) {
		pSDev = new CBibbleWindow();
		pSDev->SetCustom(true);

	} else if (!typestr.find("PLAYNAV")) {
		pSDev = new CNavWindow();

		// delineate cic's
	} else if (!typestr.find("CIC")) {
		if ((pSDev = new GAMEWINDOW()) != nullptr) {
			pSDev->SetCloseup(true);
			pSDev->SetCIC(true);
			pSDev->SetExitOnEdge(80);
		}

	} else if (!typestr.find("CLOSEUP")) {
		if ((pSDev = new GAMEWINDOW()) != nullptr) {
			pSDev->SetCloseup(true);
			pSDev->SetExitOnEdge(80);
		}

	} else if (!typestr.find("CHAT")) {
		if ((pSDev = new CBagChatWnd()) != nullptr) {
			pSDev->SetCloseup(true);
			//          pSDev->SetExitOnEdge(80);
		}

	} else if (!typestr.find("EVENT")) {        // EVT STUFF
		pSDev = new CBagEventSDev();

	} else if (!typestr.find("TURNEVENT")) {    // turn based storage device
		pSDev = new CBagTurnEventSDev();

	} else if (!typestr.find("LOG")) {          // PDA LOG STUFF
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
		pSDev->SetExitOnEdge(10);
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

void CSBarMasterWin::onNewFilter(CBagStorageDev *pSDev, const CBofString &typestr) {
	assert(isValidObject(this));

	FilterFxn filterFunction = nullptr;
	int filterId = pSDev->GetFilterId();

	if (filterId != 0) {
		filterFunction = &doFilters;
	}

	if (!typestr.find("PDA")) {
	} else if (!typestr.find("WIELD")) {
	} else if (!typestr.find("BMP")) {
	} else if (!typestr.find("DLG")) {
		pSDev->OnSetFilter(filterFunction);
	} else if (!typestr.find("BARCOMP")) {
	} else if (!typestr.find("SRACOMP")) {
	} else if (!typestr.find("VIDEQUIP")) {
	} else if (!typestr.find("FULLSCREEN")) {
	} else if (!typestr.find("SBARSLOT")) {
	} else if (!typestr.find("BIBODDS")) {
	} else if (!typestr.find("INVWLD")) {
	} else if (!typestr.find("MOOWLD")) {
	} else if (!typestr.find("ZOOMPDA")) {
	} else if (!typestr.find("PLAYBIBBLE")) {
	} else if (!typestr.find("PLAYNAV")) {
	} else if (!typestr.find("CLOSEUP")) {
		pSDev->OnSetFilter(filterFunction);
	} else if (!typestr.find("CHAT")) {
		pSDev->OnSetFilter(filterFunction);
	} else if (!typestr.find("EVENT")) {
	} else if (!typestr.find("TURNEVENT")) {
	} else if (!typestr.find("LOG")) {
	} else {
		// The game window
		pSDev->OnSetFilter(filterFunction);
	}
}

void CSBarMasterWin::onNewFilter(CBagStorageDev *pSDev, const int nType) {
	assert(isValidObject(this));

	FilterFxn filterFunction = nullptr;
	int filterId = pSDev->GetFilterId();

	if (filterId != 0) {
		filterFunction = &doFilters;
	}

	switch (nType) {
	case SDEV_PDA:
	case SDEV_WIELD:
	case SDEV_BMP:
		break;

	case SDEV_DLG:
		pSDev->OnSetFilter(filterFunction);
		break;

	case SDEV_CLOSEP:
		pSDev->OnSetFilter(filterFunction);
		break;

	case SDEV_WND:
		pSDev->OnSetFilter(filterFunction);
		break;

	default:
		break;
	}
}

} // namespace SpaceBar
} // namespace Bagel
