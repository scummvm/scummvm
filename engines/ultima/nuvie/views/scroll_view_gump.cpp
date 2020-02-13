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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/fonts/font_manager.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/scroll_widget_gump.h"
#include "ultima/nuvie/views/scroll_view_gump.h"

namespace Ultima {
namespace Nuvie {

#define SIGN_BG_W (SCROLLWIDGETGUMP_W + 16)
#define SIGN_BG_H (SCROLLWIDGETGUMP_H + 16)

ScrollViewGump::ScrollViewGump(Configuration *cfg) : DraggableView(cfg) {
	scroll_widget = NULL;
}

ScrollViewGump::~ScrollViewGump() {

}

bool ScrollViewGump::init(Screen *tmp_screen, void *view_manager, Font *f, Party *p, TileManager *tm, ObjManager *om, Std::string text_string) {
	uint16 x_off = Game::get_game()->get_game_x_offset();
	uint16 y_off = Game::get_game()->get_game_y_offset();

	x_off += (Game::get_game()->get_game_width() - SIGN_BG_W) / 2;
	y_off += (Game::get_game()->get_game_height() - SIGN_BG_H) / 2;

	View::init(x_off, y_off, f, p, tm, om);
	SetRect(area.left, area.top, SIGN_BG_W, SIGN_BG_H);
	/*
	    Std::string datadir = GUI::get_gui()->get_data_dir();
	    Std::string imagefile;
	    Std::string path;

	    build_path(datadir, "images", path);
	    datadir = path;
	    build_path(datadir, "gumps", path);
	    datadir = path;
	    build_path(datadir, "sign", path);
	    datadir = path;

	    build_path(datadir, "sign_bg.bmp", imagefile);
	    bg_image = SDL_LoadBMP(imagefile.c_str());

	    set_bg_color_key(0, 0x70, 0xfc);
	*/
	scroll_widget = new ScrollWidgetGump(config, tmp_screen);
	scroll_widget->init(config, Game::get_game()->get_font_manager()->get_conv_font());

	scroll_widget->display_string(text_string);

	AddWidget(scroll_widget);

	return true;
}


void ScrollViewGump::Display(bool full_redraw) {
	/*
	Common::Rect dst;
	dst = area;
	SDL_BlitSurface(bg_image, NULL, surface, &dst);
	*/
	screen->fill(26, area.left, area.top, area.width(), area.height());
	DisplayChildren(full_redraw);

//font->textOut(screen->get_sdl_surface(), area.left + 29, area.top + 6, "This is a test sign");

//font->drawString(screen, sign_text, strlen(sign_text), area.left + (area.width() - font->getStringWidth(sign_text)) / 2, area.top + (area.height() - 19) / 2, 0, 0);
	update_display = false;
	screen->update(area.left, area.top, area.width(), area.height());


	return;
}

GUI_status ScrollViewGump::MouseDown(int x, int y, Shared::MouseButton button) {
	if (scroll_widget->MouseDown(x, y, button) != GUI_YUM) {
		Game::get_game()->get_view_manager()->close_gump(this);
	}
	return GUI_YUM;
}

GUI_status ScrollViewGump::KeyDown(const Common::KeyState &key) {
	if (scroll_widget->KeyDown(key) != GUI_YUM) {
		Game::get_game()->get_view_manager()->close_gump(this);
	}

	return GUI_YUM;
}

GUI_status ScrollViewGump::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	Game::get_game()->get_view_manager()->close_gump(this);
	return GUI_YUM;
}

} // End of namespace Nuvie
} // End of namespace Ultima
