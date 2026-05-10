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

#ifndef ULTIMA0_VIEWS_TITLE_H
#define ULTIMA0_VIEWS_TITLE_H

#include "ultima/ultima0/views/view.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

class Title : public View {
	class TitleOption : public UIElement {
	public:
		int _index;
		Common::String _text;
		byte _color = 0;
	public:
		TitleOption(Title *parent, int index, const Common::String &text, int row);
		void draw() override;
		bool msgMouseEnter(const MouseEnterMessage &msg) override;
	};

private:
	TitleOption _options[4] = {
		TitleOption(this, 0, "Introduction", 16),
		TitleOption(this, 1, "Create a Character", 17),
		TitleOption(this, 2, "Acknowledgements", 18),
		TitleOption(this, 3, "Journey Onwards", 19)
	};
	int _highlightedOption = 0;

	void updateSelections();
	void selectOption();

public:
	Title();
	~Title() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
};

} // namespace Views
} // namespace Ultima0
} // namespace Ultima

#endif
