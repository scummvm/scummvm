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

#include "zvision/scripting/controls/titler_control.h"

#include "zvision/zvision.h"
#include "zvision/text/text.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"

#include "common/stream.h"

namespace ZVision {

TitlerControl::TitlerControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_TITLER) {

	_surface = NULL;
	_curString = -1;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("string_resource_file", true)) {
			readStringsFile(values);
		} else if (param.matchString("rectangle", true)) {
			int x;
			int y;
			int x2;
			int y2;

			sscanf(values.c_str(), "%d %d %d %d", &x, &y, &x2, &y2);

			_rectangle = Common::Rect(x, y, x2, y2);
		}

		line = stream.readLine();
		_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}

	if (!_rectangle.isEmpty()) {
		_surface = new Graphics::Surface;
		_surface->create(_rectangle.width(), _rectangle.height(), _engine->_resourcePixelFormat);
		_surface->fillRect(Common::Rect(_surface->w, _surface->h), 0);
	}
}

TitlerControl::~TitlerControl() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

void TitlerControl::setString(int strLine) {
	if (strLine != _curString && strLine >= 0 && strLine < (int)_strings.size()) {
		_surface->fillRect(Common::Rect(_surface->w, _surface->h), 0);
		_engine->getTextRenderer()->drawTextWithWordWrapping(_strings[strLine], *_surface);
		_engine->getRenderManager()->blitSurfaceToBkg(*_surface, _rectangle.left, _rectangle.top);
		_curString = strLine;
	}
}

void TitlerControl::readStringsFile(const Common::String &fileName) {
	Common::File file;
	if (!_engine->getSearchManager()->openFile(file, fileName)) {
		warning("String_resource_file %s could could be opened", fileName.c_str());
		return;
	}

	_strings.clear();

	while (!file.eos()) {

		Common::String line = readWideLine(file);
		_strings.push_back(line);
	}
	file.close();
}

} // End of namespace ZVision
