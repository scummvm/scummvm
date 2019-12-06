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

#include "ultima8/misc/pent_include.h"
#include "ultima8/gumps/menu_gump.h"

#include "ultima8/games/game_data.h"
#include "ultima8/graphics/gump_shape_archive.h"
#include "ultima8/graphics/shape.h"
#include "ultima8/graphics/shape_frame.h"
#include "ultima8/ultima8.h"
#include "ultima8/gumps/desktop_gump.h"
#include "ultima8/gumps/widgets/button_widget.h"
#include "ultima8/gumps/widgets/text_widget.h"
#include "ultima8/gumps/quit_gump.h"
#include "ultima8/gumps/controls_gump.h"
#include "ultima8/gumps/options_gump.h"
#include "ultima8/gumps/paged_gump.h"
#include "ultima8/games/game.h"
#include "ultima8/world/actors/main_actor.h"
#include "ultima8/graphics/fonts/font.h"
#include "ultima8/graphics/fonts/rendered_text.h"
#include "ultima8/graphics/fonts/font_manager.h"
#include "ultima8/conf/setting_manager.h"
#include "ultima8/audio/music_process.h"
#include "ultima8/gumps/widgets/edit_widget.h"
#include "ultima8/gumps/u8_save_gump.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MenuGump, ModalGump)

MenuGump::MenuGump(bool nameEntryMode_)
	: ModalGump(0, 0, 5, 5, 0, FLAG_DONT_SAVE) {
	nameEntryMode = nameEntryMode_;

	Ultima8Engine *app = Ultima8Engine::get_instance();
	app->pushMouseCursor();
	if (!nameEntryMode)
		app->setMouseCursor(Ultima8Engine::MOUSE_HAND);
	else
		app->setMouseCursor(Ultima8Engine::MOUSE_NONE);

	// Save old music state
	MusicProcess *musicprocess = MusicProcess::get_instance();
	if (musicprocess) oldMusicTrack = musicprocess->getTrack();
	else oldMusicTrack = 0;
}

MenuGump::~MenuGump() {
}

void MenuGump::Close(bool no_del) {
	// Restore old music state
	MusicProcess *musicprocess = MusicProcess::get_instance();
	if (musicprocess) musicprocess->playMusic(oldMusicTrack);

	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	guiapp->popMouseCursor();

	ModalGump::Close(no_del);
}

static const int gumpShape = 35;
static const int paganShape = 32;
static const int menuEntryShape = 37;

void MenuGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	ShapeFrame *sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	Shape *logoShape;
	logoShape = GameData::get_instance()->getGumps()->getShape(paganShape);
	sf = logoShape->getFrame(0);
	assert(sf);

	Gump *logo = new Gump(42, 10, sf->width, sf->height);
	logo->SetShape(logoShape, 0);
	logo->InitGump(this, false);

	if (!nameEntryMode) {
		SettingManager *settingman = SettingManager::get_instance();
		bool endgame, quotes;
		settingman->get("endgame", endgame);
		settingman->get("quotes", quotes);

		int x_ = dims.w / 2 + 14;
		int y_ = 18;
		Gump *widget;
		for (int i = 0; i < 8; ++i) {
			if ((quotes || i != 6) && (endgame || i != 7)) {
				FrameID frame_up(GameData::GUMPS, menuEntryShape, i * 2);
				FrameID frame_down(GameData::GUMPS, menuEntryShape, i * 2 + 1);
				frame_up = _TL_SHP_(frame_up);
				frame_down = _TL_SHP_(frame_down);
				widget = new ButtonWidget(x_, y_, frame_up, frame_down, true);
				widget->InitGump(this, false);
				widget->SetIndex(i + 1);
			}

			y_ += 14;
		}

		MainActor *av = getMainActor();
		std::string name;
		if (av)
			name = av->getName();

		if (!name.empty()) {
			Pentagram::Rect rect;
			widget = new TextWidget(0, 0, name, true, 6);
			widget->InitGump(this, false);
			widget->GetDims(rect);
			widget->Move(90 - rect.w / 2, dims.h - 40);
		}
	} else {
		Gump *widget;
		widget = new TextWidget(0, 0, _TL_("Give thy name:"), true, 6); // CONSTANT!
		widget->InitGump(this, false);
		widget->Move(dims.w / 2 + 6, 10);

		Pentagram::Rect textdims;
		widget->GetDims(textdims);

		widget = new EditWidget(0, 0, "", true, 6, 110, 40, 15); // CONSTANTS!
		widget->InitGump(this, true);
		widget->Move(dims.w / 2 + 6, 10 + textdims.h);
		widget->MakeFocus();
	}
}


void MenuGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

bool MenuGump::OnKeyDown(int key, int mod) {
	if (Gump::OnKeyDown(key, mod)) return true;

	if (!nameEntryMode) {

		if (key == Common::KEYCODE_ESCAPE) {
			// FIXME: this check should probably be in Game or GUIApp
			MainActor *av = getMainActor();
			if (av && !(av->getActorFlags() & Actor::ACT_DEAD))
				Close(); // don't allow closing if dead/game over
		} else if (key >= Common::KEYCODE_1 && key <= Common::KEYCODE_9) {
			selectEntry(key - Common::KEYCODE_1 + 1);
		}

	}

	return true;
}

void MenuGump::ChildNotify(Gump *child, uint32 message) {
	if (child->IsOfType<EditWidget>() && message == EditWidget::EDIT_ENTER) {
		EditWidget *editwidget = p_dynamic_cast<EditWidget *>(child);
		assert(editwidget);
		std::string name = editwidget->getText();
		if (!name.empty()) {
			MainActor *av = getMainActor();
			av->setName(name);
			Close();
		}
	}

	if (child->IsOfType<ButtonWidget>() && message == ButtonWidget::BUTTON_CLICK) {
		selectEntry(child->GetIndex());
	}
}

void MenuGump::selectEntry(int entry) {
	SettingManager *settingman = SettingManager::get_instance();
	bool endgame, quotes;
	settingman->get("endgame", endgame);
	settingman->get("quotes", quotes);

	switch (entry) {
	case 1: // Intro
		Game::get_instance()->playIntroMovie();
		break;
	case 2:
	case 3: // Read/Write Diary
		U8SaveGump::showLoadSaveGump(this, entry == 3);
		break;
	case 4: { // Options
		PagedGump *gump = new PagedGump(34, -38, 3, gumpShape);
		gump->InitGump(this);

		OptionsGump *options = new OptionsGump();
		options->InitGump(gump, false);
		gump->addPage(options);
		gump->setRelativePosition(CENTER);
	}
	break;
	case 5: // Credits
		Game::get_instance()->playCredits();
		break;
	case 6: // Quit
		QuitGump::verifyQuit();
		break;
	case 7: // Quotes
		if (quotes) Game::get_instance()->playQuotes();
		break;
	case 8: // End Game
		if (endgame) Game::get_instance()->playEndgameMovie();
		break;
	default:
		break;
	}
}

bool MenuGump::OnTextInput(int unicode) {
	if (Gump::OnTextInput(unicode)) return true;

	return true;
}

//static
void MenuGump::showMenu() {
	ModalGump *gump = new MenuGump();
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
}

// static
void MenuGump::ConCmd_showMenu(const Console::ArgvType &argv) {
	MenuGump::showMenu();
}

//static
void MenuGump::inputName() {
	ModalGump *gump = new MenuGump(true);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
}

} // End of namespace Ultima8
