
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

#include "bagel/spacebar/baglib/chat_wnd.h"

namespace Bagel {
namespace SpaceBar {

#define NUM_PAYOFFS 16

class SBarBibOddsWnd : public CBagChatWnd {
public:
	SBarBibOddsWnd();
	virtual ~SBarBibOddsWnd();

	virtual ErrorCode attach();        // Save wielded cursor
	virtual ErrorCode detach();        // This function detaches the background and necessary bitmaps

	void onMouseMove(uint32 nFlags, CBofPoint *xPoint, void * = nullptr);
	void onKeyHit(uint32 lKey, uint32 lRepCount);

	static void initialize();
	static CBagObject *_wieldedObject;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
