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

#ifndef TETRAEDGE_TE_TE_XML_GUI_H
#define TETRAEDGE_TE_TE_XML_GUI_H

#include "common/hash-str.h"
#include "common/str.h"
#include "common/path.h"

#include "tetraedge/te/te_button_layout.h"
#include "tetraedge/te/te_sprite_layout.h"

namespace Tetraedge {

class TeXmlGui {
public:
	TeXmlGui();

	Common::String value(const Common::String &key);

	void clear();

	void load(const Common::Path &path);
	void unload();

	TeSpriteLayout *sprite(const Common::String &name);
	TeButtonLayout *button(const Common::String &name);
	bool group(const Common::String &name);

private:
	Common::StringMap _map;
	bool _loaded;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_XML_GUI_H
