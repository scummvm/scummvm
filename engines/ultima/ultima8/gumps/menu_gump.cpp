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

#include "common/config-manager.h"

#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/gumps/cru_menu_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/games/game.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/gumps/widgets/edit_widget.h"
#include "ultima/ultima8/gumps/u8_save_gump.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/metaengine.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MenuGump)

MenuGump::MenuGump(bool nameEntryMode)
	: ModalGump(0, 0, 5, 5, 0, FLAG_DONT_SAVE) {
	_nameEntryMode = nameEntryMode;

	Mouse *mouse = Mouse::get_instance();
	if (!_nameEntryMode)
		mouse->pushMouseCursor(Mouse::MOUSE_HAND);
	else
		mouse->pushMouseCursor(Mouse::MOUSE_NONE);

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
}

MenuGump::~MenuGump() {
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
		bool endgame = ConfMan.getBool("endgame");
		bool quotes = ConfMan.getBool("quotes");

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
			Gump *widget = new TextWidget(0, 0, name, true, 6);
			widget->InitGump(this, false);
			Common::Rect32 rect = widget->getDims();
			widget->Move(90 - rect.width() / 2, _dims.height() - 40);
		}
	} else {
		Gump *widget;
		widget = new TextWidget(0, 0, _TL_("Give thy name:"), true, 6); // CONSTANT!
		widget->InitGump(this, false);
		widget->Move(_dims.width() / 2 + 6, 10);

		Common::Rect32 textdims = widget->getDims();

		widget = new EditWidget(0, 0, "", true, 6, 110, 40, 15); // CONSTANTS!
		widget->InitGump(this, true);
		widget->Move(_dims.width() / 2 + 6, 10 + textdims.height());
		widget->MakeFocus();
	}
}


void MenuGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

void MenuGump::onMouseDouble(int button, int32 mx, int32 my) {
	// FIXME: this check should probably be in Game or GUIApp
	MainActor *av = getMainActor();
	if (av && !av->hasActorFlags(Actor::ACT_DEAD))
		Close(); // don't allow closing if dead/game over
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
	if (buttonWidget) {
		if (message == ButtonWidget::BUTTON_CLICK || message == ButtonWidget::BUTTON_DOUBLE) {
			selectEntry(buttonWidget->GetIndex());
		}
	}
}

void MenuGump::selectEntry(int entry) {
	bool endgame = ConfMan.getBool("endgame");
	bool quotes = ConfMan.getBool("quotes");

	switch (entry) {
	case 1: // Intro
		Game::get_instance()->playIntroMovie(true);
		break;
	case 2:
	case 3: // Read/Write Diary
		U8SaveGump::showLoadSaveGump(this, entry == 3);
		break;
	case 4: {
		Ultima8Engine::get_instance()->openConfigDialog();
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
	ModalGump *gump;
	if (GAME_IS_U8)
		gump = new MenuGump();
	else
		gump = new CruMenuGump();
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
}

//static
void MenuGump::inputName() {
	ModalGump *gump;
	if (GAME_IS_U8)
		gump = new MenuGump(true);
	else
		gump = new CruMenuGump();
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
}

} // End of namespace Ultima8
} // End of namespace Ultima
