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

#ifndef TETRAEDGE_TE_TE_INPUT_MGR_H
#define TETRAEDGE_TE_TE_INPUT_MGR_H

#include "common/events.h"

#include "tetraedge/te/te_signal.h"

namespace Tetraedge {

class TeInputMgr {
public:
	TeInputMgr();

	void handleEvent(const Common::Event &e);

	TeSignal1Param<const Common::KeyState &> _keyDownSignal;
	TeSignal1Param<const Common::KeyState &> _keyUpSignal;
	TeSignal1Param<const Common::Point &> _mouseMoveSignal;
	TeSignal1Param<const Common::Point &> _mouseLDownSignal;
	TeSignal1Param<const Common::Point &> _mouseLUpSignal;
	TeSignal1Param<const Common::Point &> _mouseRDownSignal;
	TeSignal1Param<const Common::Point &> _mouseRUpSignal;

	const Common::Point &lastMousePos() {
		return _lastMousePos;
	}

private:
	Common::Point _lastMousePos;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_INPUT_MGR_H
