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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/gumps/remorse_menu_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/gumps/paged_gump.h"
#include "ultima/ultima8/games/game.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/gumps/widgets/edit_widget.h"
#include "ultima/ultima8/gumps/u8_save_gump.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/meta_engine.h"
#include "engines/dialogs.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MenuGump)

MenuGump::MenuGump(bool nameEntryMode)
	: ModalGump(0, 0, 5, 5, 0, FLAG_DONT_SAVE) {
	_nameEntryMode = nameEntryMode;

	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor();
	if (!_nameEntryMode)
		mouse->setMouseCursor(Mouse::MOUSE_HAND);
	else
		mouse->setMouseCursor(Mouse::MOUSE_NONE);

	// Save old music state
	MusicProcess *musicprocess = MusicProcess::get_instance();
	if (musicprocess) {
		musicprocess->saveTrackState();
		// Stop any playing music.
		musicprocess->playCombatMusic(0);
	}

	// Save old palette transform
	PaletteManager *palman = PaletteManager::get_instance();
	palman->getTransformMatrix(_oldPalTransform, PaletteManager::Pal_Game);
	palman->untransformPalette(PaletteManager::Pal_Game);

	MetaEngine::setGameMenuActive(true);
}

MenuGump::~MenuGump() {
	MetaEngine::setGameMenuActive(false);
}


void MenuGump::Close(bool no_del) {
	// Restore old music state and palette.
	// Music state can be changed by the Intro and Credits
	MusicProcess *musicprocess = MusicProcess::get_instance();
	if (musicprocess)
		musicprocess->restoreTrackState();
	PaletteManager *palman = PaletteManager::get_instance();
	palman->transformPalette(PaletteManager::Pal_Game, _oldPalTransform);

	Mouse *mouse = Mouse::get_instance();
	mouse->popMouseCursor();

	ModalGump::Close(no_del);
}

static const int gumpShape = 35;
static const int paganShape = 32;
static const int menuEntryShape = 37;

static const char *MENU_TXT[] = {
	"1.Intro",
	"2.Read Diary",
	"3.Write Diary",
	"4.Options",
	"5.Credits",
	"6.Quit",
	"7.Quotes",
	"8.End Game"
};

void MenuGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	_shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	UpdateDimsFromShape();

	Shape *logoShape;
	logoShape = GameData::get_instance()->getGumps()->getShape(paganShape);
	const ShapeFrame *sf = logoShape->getFrame(0);
	assert(sf);

	Gump *logo = new Gump(42, 10, sf->_width, sf->_height);
	logo->SetShape(logoShape, 0);
	logo->InitGump(this, false);

	if (!_nameEntryMode) {
		SettingManager *settingman = SettingManager::get_instance();
		bool endgame = false;
		bool quotes = false;
		settingman->get("endgame", endgame);
		settingman->get("quotes", quotes);

		int x = _dims.width() / 2 + 14;
		int y = 18;
		for (int i = 0; i < 8; ++i) {
			if ((quotes || i != 6) && (endgame || i != 7)) {
				FrameID frame_up(GameData::GUMPS, menuEntryShape, i * 2);
				FrameID frame_down(GameData::GUMPS, menuEntryShape, i * 2 + 1);
				frame_up = _TL_SHP_(frame_up);
				frame_down = _TL_SHP_(frame_down);
				Gump *widget;
				if (frame_up._shapeNum) {
					widget = new ButtonWidget(x, y, frame_up, frame_down, true);
				} else {
					// JA U8 has text labels
					widget = new ButtonWidget(x, y, _TL_(MENU_TXT[i]), true, 0);
				}
				widget->InitGump(this, false);
				widget->SetIndex(i + 1);
			}

			y += 14;
		}

		const MainActor *av = getMainActor();
		Std::string name;
		if (av)
			name = av->getName();

		if (!name.empty()) {
			Rect rect;
			Gump *widget = new TextWidget(0, 0, name, true, 6);
			widget->InitGump(this, false);
			widget->GetDims(rect);
			widget->Move(90 - rect.width() / 2, _dims.height() - 40);
		}
	} else {
		Gump *widget;
		widget = new TextWidget(0, 0, _TL_("Give thy name:"), true, 6); // CONSTANT!
		widget->InitGump(this, false);
		widget->Move(_dims.width() / 2 + 6, 10);

		Rect textdims;
		widget->GetDims(textdims);

		widget = new EditWidget(0, 0, "", true, 6, 110, 40, 15); // CONSTANTS!
		widget->InitGump(this, true);
		widget->Move(_dims.width() / 2 + 6, 10 + textdims.height());
		widget->MakeFocus();
	}
}


void MenuGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

bool MenuGump::OnKeyDown(int key, int mod) {
	if (Gump::OnKeyDown(key, mod)) return true;

	if (!_nameEntryMode) {

		if (key == Common::KEYCODE_ESCAPE) {
			// FIXME: this check should probably be in Game or GUIApp
			MainActor *av = getMainActor();
			if (av && !av->hasActorFlags(Actor::ACT_DEAD))
				Close(); // don't allow closing if dead/game over
		} else if (key >= Common::KEYCODE_1 && key <= Common::KEYCODE_9) {
			selectEntry(key - Common::KEYCODE_1 + 1);
		}

	}

	return true;
}

void MenuGump::ChildNotify(Gump *child, uint32 message) {
	EditWidget *editwidget = dynamic_cast<EditWidget *>(child);
	if (editwidget && message == EditWidget::EDIT_ENTER) {
		Std::string name = editwidget->getText();
		if (!name.empty()) {
			MainActor *av = getMainActor();
			av->setName(name);
			Close();
		}
	}

	ButtonWidget *buttonWidget = dynamic_cast<ButtonWidget *>(child);
	if (buttonWidget && message == ButtonWidget::BUTTON_CLICK) {
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
		Game::get_instance()->playIntroMovie(true);
		break;
	case 2:
	case 3: // Read/Write Diary
		U8SaveGump::showLoadSaveGump(this, entry == 3);
		break;
	case 4: {
		// Options - show the ScummVM options dialog
		GUI::ConfigDialog dlg;
		dlg.runModal();
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
		if (endgame) Game::get_instance()->playEndgameMovie(true);
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
	Gump *gump = Ultima8Engine::get_instance()->getMenuGump();

	if (gump) {
		gump->Close();
	} else {
		if (GAME_IS_U8)
			gump = new MenuGump();
		else
			gump = new RemorseMenuGump();
		gump->InitGump(0);
		gump->setRelativePosition(CENTER);
	}
}

//static
void MenuGump::inputName() {
	ModalGump *gump;
	if (GAME_IS_U8)
		gump = new MenuGump(true);
	else
		gump = new RemorseMenuGump();
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
}

} // End of namespace Ultima8
} // End of namespace Ultima
