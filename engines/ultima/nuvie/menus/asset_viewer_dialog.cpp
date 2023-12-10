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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_types.h"
#include "ultima/nuvie/gui/gui_text.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/gui/gui_callback.h"
#include "ultima/nuvie/gui/gui_area.h"
#include "ultima/nuvie/gui/gui_dialog.h"
#include "ultima/nuvie/menus/asset_viewer_dialog.h"
#include "ultima/nuvie/menus/video_dialog.h"
#include "ultima/nuvie/menus/audio_dialog.h"
#include "ultima/nuvie/menus/gameplay_dialog.h"
#include "ultima/nuvie/menus/input_dialog.h"
#include "ultima/nuvie/menus/cheats_dialog.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/nuvie.h"
#include "ultima/nuvie/files/u6_shape.h"
#include "ultima/nuvie/files/u6_bmp.h"

#include "common/keyboard.h"

namespace Ultima {
namespace Nuvie {

static const int AVD_WIDTH = 320;
static const int AVD_HEIGHT = 200;

AssetViewerDialog::AssetViewerDialog(CallBack *callback)
	: GUI_Dialog(Game::get_game()->get_game_x_offset() + (Game::get_game()->get_game_width() - AVD_WIDTH) / 2,
	             Game::get_game()->get_game_y_offset() + (Game::get_game()->get_game_height() - AVD_HEIGHT) / 2,
	             AVD_WIDTH, AVD_HEIGHT, 100, 100, 100, GUI_DIALOG_UNMOVABLE),
			callback_object(callback), _viewMode(TileViewMode) {
	init();
	grab_focus();
}

bool AssetViewerDialog::init() {
	_curIdx = 0;
	_shapeIdx = 0;
	_curShape = nullptr;

	GUI *gui = GUI::get_gui();
	GUI_Font *font = gui->get_font();
	TileManager *tileman = Game::get_game()->get_tile_manager();
	
	_maxIdx = tileman->get_numtiles();
	
	Common::String title = Common::String::format("Tile %d / %d", _curIdx, _maxIdx);
	_titleTxt = new GUI_Text(10, 10, 0, 0, 0, title.c_str(), font);
	AddWidget(_titleTxt);
	_infoTxt = new GUI_Text(10, 25, 0, 0, 0, "info text about the asset will appear here", font);
	AddWidget(_infoTxt);
	updateInfoTxt();

	return true;
}

AssetViewerDialog::~AssetViewerDialog() {
}

GUI_status AssetViewerDialog::close_dialog() {
	if (_curShape)
		delete _curShape;
	Delete(); // mark dialog as deleted. it will be freed by the GUI object
	callback_object->callback(GAMEMENUDIALOG_CB_DELETE, nullptr, this);
	GUI::get_gui()->unlock_input();
	return GUI_YUM;
}

void AssetViewerDialog::Display(bool full_redraw) {
	GUI_Dialog::Display(full_redraw);

	if (_viewMode == TileViewMode) {
		TileManager *tileman = Game::get_game()->get_tile_manager();
		Screen *gameScreen = Game::get_game()->get_screen();
		const Tile *tile = tileman->get_tile(_curIdx);
		
		if (tile) {
			gameScreen->blit(offset_x + 100, offset_y + 100, tile->data, 8, 16, 16, 16, tile->transparent);
			if (tile->dbl_width) {
				const Tile *left = tileman->get_tile(_curIdx - 1);
				gameScreen->blit(offset_x + 100 - 16, offset_y + 100, left->data, 8, 16, 16, 16, left->transparent);
			}
			if (tile->dbl_height && !tile->dbl_width) {
				const Tile *top = tileman->get_tile(_curIdx - 1);
				gameScreen->blit(offset_x + 100, offset_y + 100 - 16, top->data, 8, 16, 16, 16, top->transparent);
			} else if (tile->dbl_height) {
				// dbl width and double-height
				const Tile *topleft = tileman->get_tile(_curIdx - 3);
				gameScreen->blit(offset_x + 100 - 16, offset_y + 100 - 16, topleft->data, 8, 16, 16, 16, topleft->transparent);
				const Tile *top = tileman->get_tile(_curIdx - 2);
				gameScreen->blit(offset_x + 100, offset_y + 100 - 16, top->data, 8, 16, 16, 16, top->transparent);
			}
		}
	} else {
		assert(_viewMode == ScreenViewMode && _curShape);
		if (_curShape->get_data()) {
			uint16 w = 0;
			uint16 h = 0;
			_curShape->get_size(&w, &h);
			screen->blit(offset_x + 10, offset_y + 40, _curShape->get_data(), 8, w, h, w, true);
		}
	}
}

void AssetViewerDialog::updateInfoTxt() {
	if (_viewMode == TileViewMode) {
		TileManager *tileman = Game::get_game()->get_tile_manager();
		const Tile *tile = tileman->get_tile(_curIdx);

		Common::String title = Common::String::format("Tile %d / %d", _curIdx, _maxIdx);
		_titleTxt->setText(title.c_str());

		if (!tile) {
			_infoTxt->setText("((null tile))");
		} else {
			Common::String txt;
			if (tile->damages)
				txt += "Dmg ";
			if (tile->transparent)
				txt += "Trnsp ";
			if (tile->boundary)
				txt += "Bndry ";
			if (tile->dbl_height)
				txt += "DblH ";
			if (tile->dbl_width)
				txt += "DblW ";
			if (tile->passable)
				txt += "Pass ";
			if (tile->water)
				txt += "Water ";
			if (tile->toptile)
				txt += "Top ";

			_infoTxt->setText(txt.c_str());
		}
	} else {
		Common::String title = Common::String::format("Screen %d,%d / %d", _shapeIdx, _curIdx, _maxIdx);
		_titleTxt->setText(title.c_str());
		if (_curShape && _curShape->get_data()) {
			uint16 w = 0;
			uint16 h = 0;
			_curShape->get_size(&w, &h);
			if (w == 0 || h == 0) {
				_infoTxt->setText("(empty shape)");
			} else {
				uint16 x = 0;
				uint16 y = 0;
				_curShape->get_hot_point(&x, &y);
				Common::String info = Common::String::format("sz (%d,%d) hot (%d,%d)", w, h, x, y);
				_infoTxt->setText(info.c_str());
			}
		} else {
			_infoTxt->setText("(null shape)");
		}
	}
}

void AssetViewerDialog::updateShape() {
	if (_viewMode == ScreenViewMode) {
		if (_curShape)
			delete _curShape;
		if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
			_curShape = new U6Bmp();
			_curShape->load(_screenFile);
		} else {
			_curShape = new U6Shape();
			_curShape->load_from_lzc(_screenFile, _shapeIdx, _curIdx);
		}
	}
}

GUI_status AssetViewerDialog::KeyDown(const Common::KeyState &key) {
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);
	nuvie_game_t game_type = Game::get_game()->get_game_type();
	Configuration *config = Game::get_game()->get_config();

	switch (keybinder->GetActionKeyType(a)) {
	case NEXT_PARTY_MEMBER_KEY:
		_shapeIdx++;
		if (_shapeIdx > 2)
			_shapeIdx = 0;
		updateShape();
		updateInfoTxt();
		break;
	case PREVIOUS_PARTY_MEMBER_KEY:
		_shapeIdx--;
		if (_shapeIdx < 0)
			_shapeIdx = 2;
		updateShape();
		updateInfoTxt();
		break;
	case NORTH_KEY:
		if (key.flags & Common::KBD_SHIFT)
			_curIdx -= 10;
		else
			_curIdx--;
		if (_curIdx < 0)
			_curIdx = _maxIdx + _curIdx;

		updateShape();
		updateInfoTxt();
		break;
	case SOUTH_KEY:
		if (key.flags & Common::KBD_SHIFT)
			_curIdx += 10;
		else
			_curIdx++;
		if (_curIdx >= _maxIdx)
			_curIdx -= _maxIdx;

		updateShape();
		updateInfoTxt();
		break;
	case WEST_KEY:
	case EAST_KEY:
		_curIdx = 0;
		if (_viewMode == TileViewMode) {
			_viewMode = ScreenViewMode;
			_screenFile = "";
			if (game_type == NUVIE_GAME_MD) {
				config_get_path(config, "mdscreen.lzc", _screenFile);
				_maxIdx = 4;
			} else if (game_type == NUVIE_GAME_SE) {
				config_get_path(config, "screen.lzc", _screenFile);
				_maxIdx = 4;
			} else { // U6
				config_get_path(config, "paper.bmp", _screenFile);
				_maxIdx = 1;
			}
			updateShape();
		} else {
			delete _curShape;
			_curShape = nullptr;
			_viewMode = TileViewMode;
			_maxIdx = Game::get_game()->get_tile_manager()->get_numtiles();
		}
		updateInfoTxt();
		break;
	case CANCEL_ACTION_KEY:
		return close_dialog();
	default:
		keybinder->handle_always_available_keys(a);
		break;
	}

	return GUI_YUM;
}

GUI_status AssetViewerDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	GUI *gui = GUI::get_gui();

	if (caller == this) {
		close_dialog();
	} else {
		gui->lock_input(this);
		return GUI_PASS;
	}
	return GUI_YUM;
}

} // End of namespace Nuvie
} // End of namespace Ultima
