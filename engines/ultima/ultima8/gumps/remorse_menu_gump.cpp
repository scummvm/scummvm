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
#include "ultima/ultima8/gumps/remorse_menu_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/games/game.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/gumps/u8_save_gump.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/meta_engine.h"
#include "engines/dialogs.h"

#include "engines/dialogs.h"
#include "common/translation.h"
#include "gui/saveload.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(RemorseMenuGump)

static const int MENU_MUSIC_REMORSE = 20;
static const int MENU_MUSIC_REGRET = 18;

RemorseMenuGump::RemorseMenuGump()
	: ModalGump(0, 0, 640, 480, 0, FLAG_DONT_SAVE) {

	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor();
	mouse->setMouseCursor(Mouse::MOUSE_HAND);

	// Save old music state
	MusicProcess *musicprocess = MusicProcess::get_instance();
	if (musicprocess) {
		musicprocess->saveTrackState();

		int track = GAME_IS_REMORSE ? MENU_MUSIC_REMORSE : MENU_MUSIC_REGRET;
		// Play the menu music
		musicprocess->playMusic(track);
	}
	MetaEngine::setGameMenuActive(true);
}

RemorseMenuGump::~RemorseMenuGump() {
	MetaEngine::setGameMenuActive(false);
}

void RemorseMenuGump::Close(bool no_del) {
	// Restore old music state and palette.
	// Music state can be changed by the Intro and Credits
	MusicProcess *musicprocess = MusicProcess::get_instance();
	if (musicprocess)
		musicprocess->restoreTrackState();

	Mouse *mouse = Mouse::get_instance();
	mouse->popMouseCursor();

	ModalGump::Close(no_del);
}

static const int frameTopLeft = 54;
static const int firstMenuEntry = 58;

static const int numMenuEntries = 6;
static const int menuEntryX[] = {45, 45, 45, 446, 488, 550};
static const int menuEntryY[] = {50, 101, 151, 58, 151, 198};

void RemorseMenuGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	GumpShapeArchive *shapeArchive = GameData::get_instance()->getGumps();

	Shape *topLeft = shapeArchive->getShape(frameTopLeft);
	Shape *topRight = shapeArchive->getShape(frameTopLeft + 1);
	Shape *botLeft = shapeArchive->getShape(frameTopLeft + 2);
	Shape *botRight = shapeArchive->getShape(frameTopLeft + 3);

	if (!topLeft || !topRight || !botLeft || !botRight) {
		error("Couldn't load shapes for menu background");
		return;
	}

	PaletteManager *palman = PaletteManager::get_instance();
	assert(palman);
	const Palette *pal = palman->getPalette(PaletteManager::Pal_Misc);
	assert(pal);
	topLeft->setPalette(pal);
	topRight->setPalette(pal);
	botLeft->setPalette(pal);
	botRight->setPalette(pal);

	const ShapeFrame *tlFrame = topLeft->getFrame(0);
	const ShapeFrame *trFrame = topRight->getFrame(0);
	const ShapeFrame *blFrame = botLeft->getFrame(0);
	const ShapeFrame *brFrame = botRight->getFrame(0);
	if (!tlFrame || !trFrame || !blFrame || !brFrame) {
		error("Couldn't load shape frames for menu background");
		return;
	}

	_dims.left = 0;
	_dims.top = 0;
	_dims.setWidth(tlFrame->_width + trFrame->_width);
	_dims.setHeight(tlFrame->_height + brFrame->_height);

	Gump *tlGump = new Gump(0, 0, tlFrame->_width, tlFrame->_height);
	tlGump->SetShape(topLeft, 0);
	tlGump->InitGump(this, false);
	Gump *trGump = new Gump(tlFrame->_width, 0, trFrame->_width, trFrame->_height);
	trGump->SetShape(topRight, 0);
	trGump->InitGump(this, false);
	Gump *blGump = new Gump(0, tlFrame->_height, blFrame->_width, blFrame->_height);
	blGump->SetShape(botLeft, 0);
	blGump->InitGump(this, false);
	Gump *brGump = new Gump(blFrame->_width, trFrame->_height, brFrame->_width, brFrame->_height);
	brGump->SetShape(botRight, 0);
	brGump->InitGump(this, false);

	for (int i = 0; i < numMenuEntries; i++) {
		uint32 entryShapeNum = firstMenuEntry + i;
		Shape *menuEntry = shapeArchive->getShape(entryShapeNum);
		if (!menuEntry) {
			error("Couldn't load shape for menu entry %d", i);
			return;
		}
		menuEntry->setPalette(pal);

		const ShapeFrame *menuEntryFrame = menuEntry->getFrame(0);
		if (!menuEntryFrame || menuEntry->frameCount() != 2) {
			error("Couldn't load shape frame for menu entry %d", i);
			return;
		}

		FrameID frame_up(GameData::GUMPS, entryShapeNum, 0);
		FrameID frame_down(GameData::GUMPS, entryShapeNum, 1);
		Gump *widget = new ButtonWidget(menuEntryX[i], menuEntryY[i], frame_up, frame_down, true);
		widget->InitGump(this, false);
		widget->SetIndex(i + 1);
	}
}

void RemorseMenuGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

bool RemorseMenuGump::OnKeyDown(int key, int mod) {
	if (Gump::OnKeyDown(key, mod)) return true;

	if (key == Common::KEYCODE_ESCAPE) {
		// FIXME: this check should probably be in Game or GUIApp
		MainActor *av = getMainActor();
		if (av && !av->hasActorFlags(Actor::ACT_DEAD))
			Close(); // don't allow closing if dead/game over
	} else if (key >= Common::KEYCODE_1 && key <= Common::KEYCODE_6) {
		selectEntry(key - Common::KEYCODE_1 + 1);
	}

	return true;
}

void RemorseMenuGump::ChildNotify(Gump *child, uint32 message) {
	ButtonWidget *buttonWidget = dynamic_cast<ButtonWidget *>(child);
	if (buttonWidget && message == ButtonWidget::BUTTON_CLICK) {
		selectEntry(child->GetIndex());
	}
}

static void _openScummVmSaveLoad(bool isSave) {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	if (isSave) {
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

		slot = dialog->runModalWithCurrentTarget();
		desc = dialog->getResultString();

		if (desc.empty()) {
			// create our own description for the saved game, the user didnt enter it
			desc = dialog->createDefaultSaveDescription(slot);
		}

		if (desc.size() > 28)
			desc = Common::String(desc.c_str(), 28);
	} else {
		dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		slot = dialog->runModalWithCurrentTarget();
	}

	delete dialog;

	if (isSave)
		Ultima8Engine::get_instance()->saveGame(slot, desc, true);
	else
		Ultima8Engine::get_instance()->loadGameState(slot);
}

void RemorseMenuGump::selectEntry(int entry) {
	switch (entry) {
	case 1: // New Game
		Game::get_instance()->playIntroMovie(true);
		break;
	case 2:
	case 3: // Load/Save Game
		_openScummVmSaveLoad(entry == 3);
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
	default:
		break;
	}
}

bool RemorseMenuGump::OnTextInput(int unicode) {
	if (Gump::OnTextInput(unicode)) return true;
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
