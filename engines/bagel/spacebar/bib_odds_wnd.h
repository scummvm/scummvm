
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

#ifndef BAGEL_SPACEBAR_BIB_ODDS_WND_H
#define BAGEL_SPACEBAR_BIB_ODDS_WND_H

#include "bagel/boflib/gui/window.h"
#include "bagel/baglib/chat_wnd.h"

namespace Bagel {
namespace SpaceBar {

#define NUM_PAYOFFS 16

class SBarBibOddsWnd : public CBagChatWnd {
public:
	SBarBibOddsWnd();
	virtual ~SBarBibOddsWnd();

	virtual	ERROR_CODE Attach();		// Save wielded cursor
	virtual ERROR_CODE Detach();		// This function detachs the background and nessasary bitmaps

	VOID OnMouseMove(UINT nFlags, CBofPoint *xPoint, void * = nullptr);
	VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	static void initStatics();
	static CBagObject *m_pWieldedObject;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
