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

#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CCmdTarget, CObject)

const AFX_MSGMAP *CCmdTarget::GetThisMessageMap() {
	return nullptr;
}

void CCmdTarget::DoWaitCursor(int nCode) {
	AfxGetApp()->DoWaitCursor(nCode);
}

void CCmdTarget::BeginWaitCursor() {
	AfxGetApp()->DoWaitCursor(1);
}

void CCmdTarget::EndWaitCursor() {
	AfxGetApp()->DoWaitCursor(-1);
}

void CCmdTarget::RestoreWaitCursor() {
	AfxGetApp()->DoWaitCursor(0);
}

const AFX_MSGMAP_ENTRY *CCmdTarget::LookupMessage(unsigned int message) {
	// Iterate through this class and any ancestors
	for (const AFX_MSGMAP *map = GetMessageMap();
	        map; map = map->pfnGetBaseMap()) {
		// Scan the entries to look for the message
		for (const AFX_MSGMAP_ENTRY *entry = map->lpEntries;
		        entry->nMessage; ++entry) {
			if (entry->nMessage == message)
				return entry;
		}
	}

	return nullptr;
}

bool CCmdTarget::OnCmdMsg(unsigned int nID, int nCode, void *pExtra,
		AFX_CMDHANDLERINFO *pHandlerInfo) {
	// Determine the message number and code (packed into nCode)
	const AFX_MSGMAP *pMessageMap;
	const AFX_MSGMAP_ENTRY *lpEntry;
	unsigned int nMsg = 0;

	if ((uint)nCode != CN_UPDATE_COMMAND_UI) {
		nMsg = HIWORD(nCode);
		nCode = LOWORD(nCode);
	}

	// For backward compatibility HIWORD(nCode)==0 is WM_COMMAND
	if (nMsg == 0)
		nMsg = WM_COMMAND;

	// Look through message map to see if it applies to us
	for (pMessageMap = GetMessageMap(); pMessageMap != nullptr;
			pMessageMap = (*pMessageMap->pfnGetBaseMap)()) {
		lpEntry = AfxFindMessageEntry(pMessageMap->lpEntries, nMsg, nCode, nID);
		if (lpEntry) {
			// found it
			return _AfxDispatchCmdMsg(this, nID, nCode,
				lpEntry->pfn, pExtra, lpEntry->nSig, pHandlerInfo);
		}
	}

	// Not handled
	return false;
}

const AFX_MSGMAP_ENTRY *CCmdTarget::AfxFindMessageEntry(const AFX_MSGMAP_ENTRY *lpEntry,
		unsigned int nMsg, unsigned int nCode, unsigned int nID) {
	while (lpEntry->nSig != AfxSig_end) {
		if (lpEntry->nMessage == nMsg && lpEntry->nCode == nCode &&
			nID >= lpEntry->nID && nID <= lpEntry->nLastID) {
			return lpEntry;
		}

		lpEntry++;
	}

	return nullptr;
}

bool CCmdTarget::_AfxDispatchCmdMsg(CCmdTarget *pTarget, unsigned int nID, int nCode,
		AFX_PMSG pfn, void *pExtra, unsigned int nSig, AFX_CMDHANDLERINFO *pHandlerInfo) {
	union MessageMapFunctions mmf;
	mmf.pfn = pfn;
	bool bResult = true; // default is ok

	if (pHandlerInfo != nullptr) {
		// just fill in the information, don't do it
		pHandlerInfo->pTarget = pTarget;
		pHandlerInfo->pmf = mmf.pfn;
		return true;
	}

	switch (nSig) {
	case AfxSig_vv:
	case AfxSigCmd_v:
		// normal command or control notification
		assert(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		assert(pExtra == nullptr);
		(pTarget->*mmf.pfn_COMMAND)();
		break;

	case AfxSig_bv:
		// Normal command or control notification
		assert(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		assert(pExtra == nullptr);
		bResult = (pTarget->*mmf.pfn_bCOMMAND)();
		break;

	case AfxSig_vw:
		// Normal command or control notification in a range
		assert(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		assert(pExtra == nullptr);
		(pTarget->*mmf.pfn_COMMAND_RANGE)(nID);
		break;

	case AfxSig_bw:
		// extended command (passed ID, returns bContinue)
		assert(pExtra == nullptr);
		bResult = (pTarget->*mmf.pfn_COMMAND_EX)(nID);
		break;

	case AfxSig_vNMHDRpl: {
		AFX_NOTIFY *pNotify = (AFX_NOTIFY *)pExtra;
		assert(pNotify != nullptr);
		assert(pNotify->pResult != nullptr);
		assert(pNotify->pNMHDR != nullptr);
		(pTarget->*mmf.pfn_NOTIFY)(pNotify->pNMHDR, pNotify->pResult);
	}
	break;

	case AfxSig_bNMHDRpl: {
		AFX_NOTIFY *pNotify = (AFX_NOTIFY *)pExtra;
		assert(pNotify != nullptr);
		assert(pNotify->pResult != nullptr);
		assert(pNotify->pNMHDR != nullptr);
		bResult = (pTarget->*mmf.pfn_bNOTIFY)(pNotify->pNMHDR, pNotify->pResult);
	}
	break;
	case AfxSig_vwNMHDRpl: {
		AFX_NOTIFY *pNotify = (AFX_NOTIFY *)pExtra;
		assert(pNotify != nullptr);
		assert(pNotify->pResult != nullptr);
		assert(pNotify->pNMHDR != nullptr);
		(pTarget->*mmf.pfn_NOTIFY_RANGE)(nID, pNotify->pNMHDR,
			pNotify->pResult);
	}
	break;

	case AfxSig_bwNMHDRpl: {
		AFX_NOTIFY *pNotify = (AFX_NOTIFY *)pExtra;
		assert(pNotify != nullptr);
		assert(pNotify->pResult != nullptr);
		assert(pNotify->pNMHDR != nullptr);
		bResult = (pTarget->*mmf.pfn_NOTIFY_EX)(nID, pNotify->pNMHDR,
			pNotify->pResult);
	}
	break;

	case AfxSig_cmdui: {
		// ON_UPDATE_COMMAND_UI or ON_UPDATE_COMMAND_UI_REFLECT case
		assert(CN_UPDATE_COMMAND_UI == (unsigned int)-1);
		assert((uint)nCode == CN_UPDATE_COMMAND_UI || nCode == 0xFFFF);
		assert(pExtra != nullptr);
		CCmdUI *pCmdUI = (CCmdUI *)pExtra;
		assert(!pCmdUI->m_bContinueRouting);    // idle - not set
		(pTarget->*mmf.pfn_UPDATE_COMMAND_UI)(pCmdUI);
		bResult = !pCmdUI->m_bContinueRouting;
		pCmdUI->m_bContinueRouting = false;     // go back to idle
	}
	break;

	case AfxSig_cmduiw: {
		// ON_UPDATE_COMMAND_UI case
		assert((uint)nCode == CN_UPDATE_COMMAND_UI);
		assert(pExtra != nullptr);
		CCmdUI *pCmdUI = (CCmdUI *)pExtra;
		assert(pCmdUI->m_nID == (int)nID);           // sanity assert
		assert(!pCmdUI->m_bContinueRouting);    // idle - not set
		(pTarget->*mmf.pfn_UPDATE_COMMAND_UI_RANGE)(pCmdUI, nID);
		bResult = !pCmdUI->m_bContinueRouting;
		pCmdUI->m_bContinueRouting = false;     // go back to idle
	}
	break;

	// general extensibility hooks
	case AfxSig_vpv:
		(pTarget->*mmf.pfn_OTHER)(pExtra);
		break;

	case AfxSig_bpv:
		bResult = (pTarget->*mmf.pfn_OTHER_EX)(pExtra);
		break;

	default:
		// Illegal
		error("Invalid sig");
	}

	return bResult;
}

} // namespace MFC
} // namespace Bagel
