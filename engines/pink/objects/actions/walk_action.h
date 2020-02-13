/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PINK_WALK_ACTION_H
#define PINK_WALK_ACTION_H

#include "pink/objects/actions/action_cel.h"

namespace Pink {

class WalkAction : public ActionCEL {
public:
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	void update() override;

	void setWalkMgr(WalkMgr *mgr) { _mgr = mgr; }
	void setType(bool horizontal) { _horizontal = horizontal; }

protected:
	void onStart() override;

private:
	WalkMgr *_mgr;
	Common::Point _start;
	Common::Point _end;
	uint _curFrame;
	uint _frameCount;
	bool _horizontal;
	bool _toCalcFramePositions;
};

} // End of namespace Pink

#endif
