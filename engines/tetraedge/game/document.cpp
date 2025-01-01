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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/document.h"
#include "tetraedge/game/documents_browser.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_sprite_layout.h"

namespace Tetraedge {

Document::Document(DocumentsBrowser *browser) : _browser(browser) {

}

void Document::load(const Common::String &name) {
	setSizeType(RELATIVE_TO_PARENT);
	setSize(TeVector3f32(1, 1, 1));
	_gui.load("DocumentsBrowser/Document.lua");
	addChild(_gui.layoutChecked("object"));
	setName(name);
	const Common::Path sprPath = spritePath();
	_gui.spriteLayoutChecked("upLayout")->load(sprPath);
	_gui.buttonLayoutChecked("object")->onMouseClickValidated().add(this, &Document::onButtonDown);
	TeITextLayout *txtLayout = _gui.textLayout("text");
	if (!txtLayout)
		error("can't find text layout in document");
	Common::String header("<section style=\"center\" /><color r=\"255\" g=\"255\" b=\"255\"/><font file=\"Common/Fonts/arial.ttf\" size=\"16\" />");
	txtLayout->setText(header + _browser->documentName(name));
}

void Document::unload() {
	removeChild(_gui.layoutChecked("object"));
	_gui.unload();
}

bool Document::onButtonDown() {
	_onButtonDownSignal.call(*this);
	return false;
}

Common::Path Document::spritePath() const {
	return Common::Path("DocumentsBrowser/Documents").join(name()).append(".png");
}

} // end namespace Tetraedge
