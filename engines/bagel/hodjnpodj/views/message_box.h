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

#ifndef HODJNPODJ_VIEWS_MESSAGE_BOX_H
#define HODJNPODJ_VIEWS_MESSAGE_BOX_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {

class MessageBox: public View {
private:
	GfxSurface _background;
	OkButton _okButton;
	Common::String _line1;
	Common::String _line2;
	const char *_closeMessage = nullptr;

public:
	MessageBox();
	virtual ~MessageBox() {}

	static void show(const Common::String &title,
		const Common::String &msg,
		const char *closeMessage = nullptr);

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
	void close();
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
