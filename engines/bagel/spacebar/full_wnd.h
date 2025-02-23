
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

#ifndef BAGEL_SPACEBAR_FULL_WND_H
#define BAGEL_SPACEBAR_FULL_WND_H

#include "bagel/spacebar/baglib/storage_dev_win.h"
#include "bagel/spacebar/baglib/object.h"

namespace Bagel {
namespace SpaceBar {

class SBarFullWnd : public CBagStorageDevWnd {
public:
	SBarFullWnd();

	virtual ErrorCode attach();
	virtual ErrorCode detach();

	virtual void onTimer(uint32 nTimerId);

	void onMouseMove(uint32 nFlags, CBofPoint *, void * = nullptr);

	bool _bAllowEventWorld;

	static CBagObject *_pWieldedObject;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
