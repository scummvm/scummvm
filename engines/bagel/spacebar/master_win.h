
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

#ifndef BAGEL_SPACEBAR_MAsTER_WIN_H
#define BAGEL_SPACEBAR_MAsTER_WIN_H

#include "bagel/spacebar/baglib/master_win.h"

namespace Bagel {
namespace SpaceBar {

/**
 * This is the main window that is never destroyed throughout the game.
 * It handles bringing new windows in and out of focus as well as
 * containing the parsing file and info.  It also captures keyboard entry
 * and will save state information
 */
class CSBarMasterWin : public CBagMasterWin {
public:
	virtual CBagStorageDev *onNewStorageDev(const CBofString &typeStr);
	virtual CBagStorageDev *onNewStorageDev(int nType);

	virtual void onKeyHit(uint32 lKey, uint32 lRepCount);

	void onNewFilter(CBagStorageDev *pSDev, const CBofString &typeStr);
	void onNewFilter(CBagStorageDev *pSDev, int nType);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
