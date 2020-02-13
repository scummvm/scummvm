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

#ifndef NUVIE_CORE_COMMAND_BAR_NEW_UI_H
#define NUVIE_CORE_COMMAND_BAR_NEW_UI_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"

namespace Ultima {
namespace Nuvie {

class NuvieIO;
class Events;
class Game;
class GUI_Button;
class GUI_CallBack;
class Text;
class Font;

class CommandBarNewUI: public CommandBar {
protected:
	uint8 cur_pos;
	uint8 icon_w;
	uint8 icon_h;
	uint8 icon_y_offset;
	uint16 num_icons;

	Font *font;
public:
	CommandBarNewUI(Game *g);
	~CommandBarNewUI() override;

	void Display(bool full_redraw) override;
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;

private:
	const char *get_command_name(sint8 command_num);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
