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

#ifndef HODJNPODJ_METAGAME_VIEWS_DIALOG_H
#define HODJNPODJ_METAGAME_VIEWS_DIALOG_H

#include "bagel/hodjnpodj/views/resource_view.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

constexpr int DIALOG_TOP = 48;
constexpr int DIALOG_BOTTOM = 47;

class Dialog : public ResourceView {
private:
	GfxSurface _background;

public:
	Dialog(const Common::String &name, const char *resFilename = nullptr) :
		ResourceView(name, resFilename ? resFilename : "") {
	}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	void draw() override;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
