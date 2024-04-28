
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

#ifndef BAGEL_BAGLIB_CHAT_WND_H
#define BAGEL_BAGLIB_CHAT_WND_H

#include "bagel/baglib/storage_dev_win.h"

namespace Bagel {

class CBagChatWnd : public CBagStorageDevWnd {
public:
	CBagChatWnd();

	/**
	 * Called to overload new set backgrounds. Calls the set backdrop pure
	 * virtual function by default.
	 */
	ErrorCode attach() override;
	void onTimer(uint32 timerId) override;
	void onMouseMove(uint32 flags, CBofPoint *pos, void * = nullptr) override;
};

} // namespace Bagel

#endif
