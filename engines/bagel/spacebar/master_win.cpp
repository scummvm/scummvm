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
	return BuildString("Version: %s, %s", __DATE__, __TIME__);
}

CBagStorageDev *CSBarMasterWin::OnNewStorageDev(const CBofString &namestr, const CBofString &typestr) {
	Assert(IsValidObject(this));

	CBagStorageDev *pSDev;

	if (!typestr.Find("PDA")) {
		pSDev = new CBagPDA();

	} else if (!typestr.Find("WIELD")) {
		pSDev = new CBagWield();

	} else if (!typestr.Find("BMP")) {
		pSDev = new CBagStorageDevBmp();

	} else if (!typestr.Find("DLG")) {
		pSDev = new CBagStorageDevDlg();

	} else if (!typestr.Find("BARCOMP")) {
		pSDev = new SBarComputer();
		pSDev->SetCustom(true);

	} else if (!typestr.Find("SRACOMP")) {
		pSDev = new SrafComputer();
		pSDev->SetExitOnEdge(10);

	} else if (!typestr.Find("VIDEQUIP")) {
		pSDev = new SBarVidWnd();
		pSDev->SetCloseup(true);
		pSDev->SetExitOnEdge(80);
		pSDev->SetCustom(true);

	} else if (!typestr.Find("FULLSCREEN2")) {
		pSDev = new SBarFullWnd();
		pSDev->SetCloseup(true);
		((SBarFullWnd *)pSDev)->m_bAllowEventWorld = false;

	} else if (!typestr.Find("FULLSCREEN")) {
		pSDev = new SBarFullWnd();
		pSDev->SetCloseup(true);

	} else if (!typestr.Find("SBARSLOT")) {
		pSDev = new SBarSlotWnd();
		pSDev->SetCustom(true);

	} else if (!typestr.Find("BIBODDS")) {
		pSDev = new SBarBibOddsWnd();
		pSDev->SetCloseup(true);
		// pSDev->SetExitOnEdge(10);
		pSDev->SetCustom(true);

	} else if (!typestr.Find("INVWLD")) {
		pSDev = new CBagInv();

	} else if (!typestr.Find("MOOWLD")) {
		pSDev = new CBagMoo();

	} else if (!typestr.Find("ZOOMPDA")) {
		pSDev = new SBZoomPda();
		pSDev->SetCloseup(true);

	} else if (!typestr.Find("PLAYBIBBLE")) {
		pSDev = new CBibbleWindow();
		pSDev->SetCustom(true);

	} else if (!typestr.Find("PLAYNAV")) {
		pSDev = new CNavWindow();

		// delineate cic's
	} else if (!typestr.Find("CIC")) {
		if ((pSDev = new GAMEWINDOW(namestr)) != nullptr) {
			pSDev->SetCloseup(true);
			pSDev->SetCIC(true);
			pSDev->SetExitOnEdge(80);
		}

	} else if (!typestr.Find("CLOSEUP")) {
		if ((pSDev = new GAMEWINDOW(namestr)) != nullptr) {
			pSDev->SetCloseup(true);
			pSDev->SetExitOnEdge(80);
		}

	} else if (!typestr.Find("CHAT")) {
		if ((pSDev = new CBagChatWnd()) != nullptr) {
			pSDev->SetCloseup(true);
			//          pSDev->SetExitOnEdge(80);
		}

	} else if (!typestr.Find("EVENT")) {        // EVT STUFF
		pSDev = new CBagEventSDev();

	} else if (!typestr.Find("TURNEVENT")) {    // turn based storage device
		pSDev = new CBagTurnEventSDev();

	} else if (!typestr.Find("LOG")) {          // PDA LOG STUFF
		pSDev = new CBagLog();

	} else {
		pSDev = new GAMEWINDOW(namestr);
	}

	return pSDev;
}


CBagStorageDev *CSBarMasterWin::OnNewStorageDev(const CBofString &namestr, int nType) {
	Assert(IsValidObject(this));

	CBagStorageDev *pSDev;

	pSDev = nullptr;

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
		pSDev = new GAMEWINDOW(namestr);
		pSDev->SetExitOnEdge(10);
		break;

	case SDEV_WND:
		pSDev = new GAMEWINDOW(namestr);
		break;

	default:
		LogError(BuildString("FAILED to init storage device of type %d", nType));
		break;
	};

	return pSDev;
}


void CSBarMasterWin::OnKeyHit(uint32 lKey, uint32 lRepCount) {
	Assert(IsValidObject(this));

	if (lKey == BKEY_ALT_v) {
		BofMessageBox(GetBuildVersion(), "Version");
	} else {
		CBagMasterWin::OnKeyHit(lKey, lRepCount);
	}
}

void CSBarMasterWin::OnNewFilter(CBagStorageDev *pSDev, const CBofString &typestr) {
	Assert(IsValidObject(this));

	FilterFxn filterFunction = nullptr;
	int filterId = pSDev->GetFilterId();

	if (filterId != 0) {
		filterFunction = &DoFilters;
	}

	if (!typestr.Find("PDA")) {
	} else if (!typestr.Find("WIELD")) {
	} else if (!typestr.Find("BMP")) {
	} else if (!typestr.Find("DLG")) {
		pSDev->OnSetFilter(filterFunction);
	} else if (!typestr.Find("BARCOMP")) {
	} else if (!typestr.Find("SRACOMP")) {
	} else if (!typestr.Find("VIDEQUIP")) {
	} else if (!typestr.Find("FULLSCREEN")) {
	} else if (!typestr.Find("SBARSLOT")) {
	} else if (!typestr.Find("BIBODDS")) {
	} else if (!typestr.Find("INVWLD")) {
	} else if (!typestr.Find("MOOWLD")) {
	} else if (!typestr.Find("ZOOMPDA")) {
	} else if (!typestr.Find("PLAYBIBBLE")) {
	} else if (!typestr.Find("PLAYNAV")) {
	} else if (!typestr.Find("CLOSEUP")) {
		pSDev->OnSetFilter(filterFunction);
	} else if (!typestr.Find("CHAT")) {
		pSDev->OnSetFilter(filterFunction);
	} else if (!typestr.Find("EVENT")) {
	} else if (!typestr.Find("TURNEVENT")) {
	} else if (!typestr.Find("LOG")) {
	} else {
		// The game window
		pSDev->OnSetFilter(filterFunction);
	}
}

void CSBarMasterWin::OnNewFilter(CBagStorageDev *pSDev, const int nType) {
	Assert(IsValidObject(this));

	FilterFxn filterFunction = nullptr;
	int filterId = pSDev->GetFilterId();

	if (filterId != 0) {
		filterFunction = &DoFilters;
	}

	switch (nType) {
	case SDEV_PDA:
		break;

	case SDEV_WIELD:
		break;

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
