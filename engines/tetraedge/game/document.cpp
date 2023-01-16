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

#include "tetraedge/game/document.h"

namespace Tetraedge {

Document::Document(DocumentsBrowser *browser) /*: _browser(browser)*/ {

}

void Document::load(const Common::String &name) {
	error("TODO: Implement Document::load");
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
