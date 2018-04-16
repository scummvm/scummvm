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

#include "zvision/scripting/controls/paint_control.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/graphics/render_manager.h"

namespace ZVision {

PaintControl::PaintControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_PAINT) {

	_cursor = CursorIndex_Active;
	_paint = NULL;
	_bkg = NULL;
	_brush = NULL;
	_colorKey = 0;
	_mouseDown = false;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("rectangle", true)) {
			int x;
			int y;
			int width;
			int height;

			sscanf(values.c_str(), "%d %d %d %d", &x, &y, &width, &height);

			_rectangle = Common::Rect(x, y, width + x, height + y);
		} else if (param.matchString("cursor", true)) {
			_cursor = _engine->getCursorManager()->getCursorId(values);
		} else if (param.matchString("brush_file", true)) {
			_brush = _engine->getRenderManager()->loadImage(values, false);
		} else if (param.matchString("venus_id", true)) {
			_venusId = atoi(values.c_str());
		} else if (param.matchString("paint_file", true)) {
			_paint = _engine->getRenderManager()->loadImage(values, false);
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

	if (_paint) {
		_colorKey = _paint->format.RGBToColor(255, 0, 255);
		_bkg = new Graphics::Surface;
		_bkg->create(_rectangle.width(), _rectangle.height(), _paint->format);
		_bkg->fillRect(Common::Rect(_rectangle.width(), _rectangle.height()), _colorKey);

		Graphics::Surface *tmp = new Graphics::Surface;
		tmp->create(_rectangle.width(), _rectangle.height(), _paint->format);
		_engine->getRenderManager()->blitSurfaceToSurface(*_paint, _rectangle, *tmp, 0, 0);
		_paint->free();
		delete _paint;
		_paint = tmp;
	}
}

PaintControl::~PaintControl() {
	// Clear the state value back to 0
	//_engine->getScriptManager()->setStateValue(_key, 0);
	if (_paint) {
		_paint->free();
		delete _paint;
	}
	if (_brush) {
		_brush->free();
		delete _brush;
	}
	if (_bkg) {
		_bkg->free();
		delete _bkg;
	}
}

bool PaintControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	_mouseDown = false;

	return false;
}

bool PaintControl::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_rectangle.contains(backgroundImageSpacePos)) {
		int mouseItem = _engine->getScriptManager()->getStateValue(StateKey_InventoryItem);

		if (eligeblity(mouseItem)) {
			setVenus();
			_mouseDown = true;
		}
	}

	return false;
}

bool PaintControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_rectangle.contains(backgroundImageSpacePos)) {
		int mouseItem = _engine->getScriptManager()->getStateValue(StateKey_InventoryItem);

		if (eligeblity(mouseItem)) {
			_engine->getCursorManager()->changeCursor(_cursor);

			if (_mouseDown) {
				Common::Rect bkgRect = paint(backgroundImageSpacePos);
				if (!bkgRect.isEmpty()) {
					Common::Rect imgRect = bkgRect;
					imgRect.translate(-_rectangle.left, -_rectangle.top);

					Graphics::Surface imgUpdate = _bkg->getSubArea(imgRect);

					_engine->getRenderManager()->blitSurfaceToBkg(imgUpdate, bkgRect.left, bkgRect.top, _colorKey);
				}
			}
			return true;
		}
	}

	return false;
}

bool PaintControl::eligeblity(int itemId) {
	for (Common::List<int>::iterator it = _eligibleObjects.begin(); it != _eligibleObjects.end(); it++)
		if (*it == itemId)
			return true;
	return false;
}

Common::Rect PaintControl::paint(const Common::Point &point) {
	Common::Rect paintRect = Common::Rect(_brush->w, _brush->h);
	paintRect.moveTo(point);
	paintRect.clip(_rectangle);

	if (!paintRect.isEmpty()) {
		Common::Rect brushRect = paintRect;
		brushRect.translate(-point.x, -point.y);

		Common::Rect bkgRect = paintRect;
		bkgRect.translate(-_rectangle.left, -_rectangle.top);

		for (int yy = 0; yy < brushRect.height(); yy++) {
			uint16 *mask = (uint16 *)_brush->getBasePtr(brushRect.left, brushRect.top + yy);
			uint16 *from = (uint16 *)_paint->getBasePtr(bkgRect.left, bkgRect.top + yy);
			uint16 *to   = (uint16 *)_bkg->getBasePtr(bkgRect.left, bkgRect.top + yy);
			for (int xx = 0; xx < brushRect.width(); xx++) {
				if (*mask != 0)
					*(to + xx) = *(from + xx);

				mask++;
			}
		}

	}
	return paintRect;
}

} // End of namespace ZVision
