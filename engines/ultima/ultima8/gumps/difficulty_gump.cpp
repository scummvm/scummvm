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

#include "ultima/ultima8/gumps/difficulty_gump.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/gfx/texture.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(DifficultyGump)

DifficultyGump::DifficultyGump()
	: ModalGump(0, 0, 5, 5), _highlighted(3), _buttonWidth(0),
	_buttonHeight(0) {
}

DifficultyGump::~DifficultyGump() {
}

static const int BUTTON_X = 158;
static const int BUTTON_Y = 120;
static const int BUTTON_SPACE = 17;
static const int BUTTON_HEIGHT = 42;
static const int RIGHT_FRAME_IDX_OFFSET = 16;


// gumps: 73: "difficulty level"
//		74-77: levels, 2 frames per level (left/right)

void DifficultyGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor(Mouse::MOUSE_HAND);

	_dims.top = 0;
	_dims.left = 0;
	_dims.bottom = 480;
	_dims.right = 640;

	GumpShapeArchive *shapeArchive = GameData::get_instance()->getGumps();

	Shape *difficultyLevel = shapeArchive->getShape(73);

	Shape *levelShape[4];
	for (int s = 74; s < 78; s++) {
		levelShape[s - 74] = shapeArchive->getShape(s);
	}

	if (!difficultyLevel || !levelShape[0] || !levelShape[1] ||
							!levelShape[2] || !levelShape[3]) {
		error("Couldn't load shapes for difficulty level");
		return;
	}

	PaletteManager *palman = PaletteManager::get_instance();
	assert(palman);
	const Palette *pal = palman->getPalette(PaletteManager::Pal_Diff);
	assert(pal);
	difficultyLevel->setPalette(pal);
	for (int s = 0; s < 4; s++) {
		levelShape[s]->setPalette(pal);
	}

	const ShapeFrame *difficultyFrame = difficultyLevel->getFrame(0);
	if (!difficultyFrame) {
		error("Couldn't load shape frame for difficulty level");
		return;
	}
	Gump *diffGump = new Gump(185, 77, difficultyFrame->_width, difficultyFrame->_height);
	diffGump->SetShape(difficultyLevel, 0);
	diffGump->InitGump(this, false);

	for (int s = 0; s < 4; s++) {
		const int y = BUTTON_Y + (BUTTON_SPACE + BUTTON_HEIGHT) * s;
		const ShapeFrame *leftFrame = levelShape[s]->getFrame(0);
		const ShapeFrame *rightFrame = levelShape[s]->getFrame(1);
		if (!leftFrame || !rightFrame) {
			error("Couldn't load shape frame for difficulty level %d", s);
			return;
		}
		Gump *lGump = new Gump(BUTTON_X, y, leftFrame->_width, leftFrame->_height);
		lGump->SetShape(levelShape[s], 0);
		lGump->InitGump(this, false);
		lGump->SetIndex(s + 1);
		Gump *rGump = new Gump(BUTTON_X + leftFrame->_width, y, rightFrame->_width, rightFrame->_height);
		rGump->SetShape(levelShape[s], 1);
		rGump->InitGump(this, false);
		rGump->SetIndex(s + 1 + RIGHT_FRAME_IDX_OFFSET);

		_buttonHeight = MAX(_buttonHeight, leftFrame->_height);
		_buttonHeight = MAX(_buttonHeight, rightFrame->_height);
		_buttonWidth = MAX(_buttonWidth, static_cast<int16>(leftFrame->_width + rightFrame->_width));
	}

	// remove focus from children (just in case)
	if (_focusChild) _focusChild->OnFocus(false);
	_focusChild = 0;
}

void DifficultyGump::Close(bool no_del) {
	Mouse *mouse = Mouse::get_instance();
	mouse->popMouseCursor();
	ModalGump::Close(no_del);
}

void DifficultyGump::OnFocus(bool gain) {
}

void DifficultyGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Paint a highlight around the current level
	int highlihght_y = BUTTON_Y + ((_highlighted - 1) * (BUTTON_SPACE + BUTTON_HEIGHT));
	uint32 color = TEX32_PACK_RGB(0x80, 0x80, 0x80);
	surf->fill32(color, BUTTON_X - 1, highlihght_y - 1, _buttonWidth + 2, _buttonHeight + 2);
	ModalGump::PaintThis(surf, lerp_factor, scaled);
}

void DifficultyGump::onMouseClick(int button, int32 mx, int32 my) {
	if (button == Mouse::BUTTON_LEFT) {
		Gump *gump = FindGump(mx, my);
		if (gump && gump->GetIndex() > 0) {
			int idx = gump->GetIndex();
			if (idx > RIGHT_FRAME_IDX_OFFSET)
				idx -= RIGHT_FRAME_IDX_OFFSET;
			selectEntry(idx);
		}
	}
}

bool DifficultyGump::OnKeyDown(int key, int mod) {
	if (ModalGump::OnKeyDown(key, mod)) return true;

	if (key == Common::KEYCODE_ESCAPE) {
		// Don't allow closing, we have to choose a difficulty.
		return true;
	} else if (key >= Common::KEYCODE_1 && key <= Common::KEYCODE_4) {
		selectEntry(key - Common::KEYCODE_1 + 1);
	} else if (key == Common::KEYCODE_UP) {
		_highlighted--;
		if (_highlighted < 1)
			_highlighted = 4;
	} else if (key == Common::KEYCODE_DOWN) {
		_highlighted++;
		if (_highlighted > 4)
			_highlighted = 1;
	} else if (key == Common::KEYCODE_RETURN) {
		selectEntry(_highlighted);
	} else {
		return false;
	}

	return true;
}

void DifficultyGump::selectEntry(int num) {
	debug(6, "selected difficulty %d", num);
	World::get_instance()->setGameDifficulty(num);
	Close();
}

} // End of namespace Ultima8
} // End of namespace Ultima
