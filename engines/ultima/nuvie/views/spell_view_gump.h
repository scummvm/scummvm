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

#ifndef NUVIE_VIEWS_SPELL_VIEW_GUMP_H
#define NUVIE_VIEWS_SPELL_VIEW_GUMP_H

#include "ultima/nuvie/views/spell_view.h"
#include "ultima/nuvie/files/nuvie_bmp_file.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class Actor;
class Font;
class U6Bmp;
class Spell;

#define SPELLVIEWGUMP_WIDTH 162

class SpellViewGump : public SpellView {

	GUI_Button *gump_button;
	sint16 selected_spell;
	GUI_Font *font;
	NuvieBmpFile bmp;
public:
	SpellViewGump(Configuration *cfg);
	~SpellViewGump() override;

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om);

	void Display(bool full_redraw) override;

	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseMotion(int x, int y, uint8 state) override {
		return DraggableView::MouseMotion(x, y, state);
	}
	GUI_status MouseWheel(sint32 x, sint32 y) override;
	void MoveRelative(int dx, int dy) override {
		return DraggableView::MoveRelative(dx, dy);
	}


	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
protected:

	sint16 getSpell(int x, int y);

	uint8 fill_cur_spell_list() override;
	void loadCircleString(Std::string datadir);
	void loadCircleSuffix(Std::string datadir, Std::string image);
	void printSpellQty(uint8 spell_num, uint16 x, uint16 y);

	void close_spellbook();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
