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

#include "common/debug.h"
#include "common/hash-str.h"
#include "common/textconsole.h"
#include "tetraedge/te/te_xml_gui.h"
#include "tetraedge/te/te_name_val_xml_parser.h"

namespace Tetraedge {

TeXmlGui::TeXmlGui() : _loaded(false) {
}

Common::String TeXmlGui::value(const Common::String &key) {
	if (_map.contains(key))
		return _map.getVal(key);
	return "";
}

void TeXmlGui::load(const Common::Path &path) {
	clear();

	TeNameValXmlParser parser;
	if (!parser.loadFile(path.toString()))
		error("TeXmlGui::load: failed to load xml.");

	_map = parser.getMap();
	_loaded = true;
}

void TeXmlGui::clear() {
	_map.clear();
	// TODO: Finish TeXmlGui.clear()
}

void TeXmlGui::unload() {
	clear();
	_loaded = false;
}

TeSpriteLayout *TeXmlGui::sprite(const Common::String &name) {
	error("TODO: Implement TeXmlGui::sprite");
}

TeButtonLayout *TeXmlGui::button(const Common::String &name) {
	error("TODO: Implement TeXmlGui::sprite");
}

bool TeXmlGui::group(const Common::String &name) {
	error("TODO: Implement TeXmlGui::group");
}

} // end namespace Tetraedge
