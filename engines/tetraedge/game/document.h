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

#ifndef TETRAEDGE_GAME_DOCUMENT_H
#define TETRAEDGE_GAME_DOCUMENT_H

#include "common/str.h"

#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class DocumentsBrowser;

class Document : public TeLayout {
public:
	Document(DocumentsBrowser *browser);
	virtual ~Document() {
		unload();
		if (parent()) {
			parent()->removeChild(this);
			setParent(nullptr);
		}
	}

	void load(const Common::String &name);
	//void loadFromBackup(TiXmlElement &node) {
	// load(node->Attribute("id");
	//}

	bool onButtonDown();
	//void saveToBackup(TiXmlElement &node);
	Common::Path spritePath() const;
	void unload();

private:
	DocumentsBrowser *_browser;
	TeLuaGUI _gui;
	TeSignal1Param<Document &> _onButtonDownSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_DOCUMENT_H
