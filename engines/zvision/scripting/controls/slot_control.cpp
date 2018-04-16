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

#include "common/scummsys.h"

#include "zvision/scripting/controls/slot_control.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/graphics/render_manager.h"

#include "common/stream.h"

namespace ZVision {

SlotControl::SlotControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_SLOT),
	  _cursor(CursorIndex_Active),
	  _distanceId('0') {

	_renderedItem = 0;
	_bkg = NULL;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("hotspot", true)) {
			int x;
			int y;
			int width;
			int height;

			sscanf(values.c_str(), "%d %d %d %d", &x, &y, &width, &height);

			_hotspot = Common::Rect(x, y, width, height);
		} else if (param.matchString("rectangle", true)) {
			int x;
			int y;
			int width;
			int height;

			sscanf(values.c_str(), "%d %d %d %d", &x, &y, &width, &height);

			_rectangle = Common::Rect(x, y, width, height);
		} else if (param.matchString("cursor", true)) {
			_cursor = _engine->getCursorManager()->getCursorId(values);
		} else if (param.matchString("distance_id", true)) {
			sscanf(values.c_str(), "%c", &_distanceId);
		} else if (param.matchString("venus_id", true)) {
			_venusId = atoi(values.c_str());
		} else if (param.matchString("eligible_objects", true)) {
			char buf[256];
			memset(buf, 0, 256);
			strncpy(buf, values.c_str(), 255);

			char *curpos = buf;
			char *strend = buf + strlen(buf);
			while (true) {
				char *st = curpos;

				if (st >= strend)
					break;

				while (*curpos != ' ' && curpos < strend)
					curpos++;

				*curpos = 0;
				curpos++;

				int obj = atoi(st);

				_eligibleObjects.push_back(obj);
			}
		}

		line = stream.readLine();
		_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}

	if (_hotspot.isEmpty() || _rectangle.isEmpty()) {
		warning("Slot %u was parsed incorrectly", key);
	}
}

SlotControl::~SlotControl() {
	// Clear the state value back to 0
	//_engine->getScriptManager()->setStateValue(_key, 0);

	if (_bkg)
		delete _bkg;
}

bool SlotControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_hotspot.contains(backgroundImageSpacePos)) {
		setVenus();

		int item = _engine->getScriptManager()->getStateValue(_key);
		int mouseItem = _engine->getScriptManager()->getStateValue(StateKey_InventoryItem);
		if (item != 0) {
			if (mouseItem != 0) {
				if (eligeblity(mouseItem)) {
					_engine->getScriptManager()->inventoryDrop(mouseItem);
					_engine->getScriptManager()->inventoryAdd(item);
					_engine->getScriptManager()->setStateValue(_key, mouseItem);
				}
			} else {
				_engine->getScriptManager()->inventoryAdd(item);
				_engine->getScriptManager()->setStateValue(_key, 0);
			}
		} else if (mouseItem == 0) {
			if (eligeblity(0)) {
				_engine->getScriptManager()->inventoryDrop(0);
				_engine->getScriptManager()->setStateValue(_key, 0);
			}
		} else if (eligeblity(mouseItem)) {
			_engine->getScriptManager()->setStateValue(_key, mouseItem);
			_engine->getScriptManager()->inventoryDrop(mouseItem);
		}
	}
	return false;
}

bool SlotControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_hotspot.contains(backgroundImageSpacePos)) {
		_engine->getCursorManager()->changeCursor(_cursor);
		return true;
	}

	return false;
}

bool SlotControl::process(uint32 deltaTimeInMillis) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_engine->canRender()) {
		int curItem = _engine->getScriptManager()->getStateValue(_key);
		if (curItem != _renderedItem) {
			if (_renderedItem != 0 && curItem == 0) {
				_engine->getRenderManager()->blitSurfaceToBkg(*_bkg, _rectangle.left, _rectangle.top);
				_renderedItem = curItem;
			} else {
				if (_renderedItem == 0) {
					if (_bkg)
						delete _bkg;

					_bkg = _engine->getRenderManager()->getBkgRect(_rectangle);
				} else {
					_engine->getRenderManager()->blitSurfaceToBkg(*_bkg, _rectangle.left, _rectangle.top);
				}

				char buf[16];
				if (_engine->getGameId() == GID_NEMESIS)
					sprintf(buf, "%d%cobj.tga", curItem, _distanceId);
				else
					sprintf(buf, "g0z%cu%2.2x1.tga", _distanceId, curItem);

				Graphics::Surface *srf = _engine->getRenderManager()->loadImage(buf);

				int16 drawx = _rectangle.left;
				int16 drawy = _rectangle.top;

				if (_rectangle.width() > srf->w)
					drawx = _rectangle.left + (_rectangle.width() - srf->w) / 2;

				if (_rectangle.height() > srf->h)
					drawy = _rectangle.top + (_rectangle.height() - srf->h) / 2;

				_engine->getRenderManager()->blitSurfaceToBkg(*srf, drawx, drawy, 0);

				delete srf;

				_renderedItem = curItem;
			}
		}
	}
	return false;
}

bool SlotControl::eligeblity(int itemId) {
	for (Common::List<int>::iterator it = _eligibleObjects.begin(); it != _eligibleObjects.end(); it++)
		if (*it == itemId)
			return true;
	return false;
}

} // End of namespace ZVision
