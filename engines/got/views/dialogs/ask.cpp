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

#include "got/views/dialogs/ask.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

Ask::Ask() : SelectOption("Ask") {
}

void Ask::show(const Common::String &title, const Common::StringArray &options) {
	Ask *view = (Ask *)g_events->findView("Ask");
	view->setContent(title, options);
	view->addView();
}

void Ask::closed() {
	_G(scripts).setAskResponse(0);
}

void Ask::selected() {
	_G(scripts).setAskResponse(_selectedItem + 1);
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
