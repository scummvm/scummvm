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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ZVISION_ACTION_NODE_H
#define ZVISION_ACTION_NODE_H

#include "common/types.h"

namespace ZVision {

class ZVision;

class ActionNode {
public:
	virtual ~ActionNode() {}
	virtual bool process(ZVision *engine, uint32 deltaTimeInMillis);
};

class NodeTimer : public ActionNode {
public:
	NodeTimer(uint32 key, uint32 timeInSeconds);
	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param engine               Pointer to the ZVision instance
	 * @param deltaTimeInMillis    Amount of time that has passed since the last frame
	 * @return    Node should be deleted after this (true) or kept (false)
	 */
	bool process(ZVision *engine, uint32 deltaTimeInMillis);

private:
	uint32 _key;
	uint32 _timeLeft;
};

} // End of namespace ZVision

#endif
