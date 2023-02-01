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

#include "ultima/ultima8/gumps/shape_viewer_gump.h"

#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/shape_info.h"

#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/fonts/font_shape_archive.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/mouse_shape_archive.h"

#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/convert/u8/convert_shape_u8.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/usecode/usecode.h"

#include "ultima/ultima8/metaengine.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ShapeViewerGump)

ShapeViewerGump::ShapeViewerGump()
	: ModalGump(), _curArchive(0), _curShape(0), _curFrame(0),
	  _background(0x101010), _fontNo(0), _shapeW(0), _shapeH(0), _shapeX(0), _shapeY(0) {

}

ShapeViewerGump::ShapeViewerGump(int x, int y, int width, int height,
								 Common::Array<ShapeArchiveEntry> &archives,
								 uint32 flags, int32 layer)
		: ModalGump(x, y, width, height, 0, flags, layer), _archives(archives),
		_curArchive(0), _curShape(0), _curFrame(0), _background(0x101010), _fontNo(0),
		_shapeW(0), _shapeH(0), _shapeX(0), _shapeY(0) {

	if (GAME_IS_CRUSADER) {
		// Default to a decent font on Crusader
		_fontNo = 6;
	}
}

ShapeViewerGump::~ShapeViewerGump() {
	for (Common::Array<ShapeArchiveEntry>::iterator it = _archives.begin(); it != _archives.end(); it++) {
		if (it->_disposeAfterUse == DisposeAfterUse::YES) {
			delete it->_archive;
		}
	}
}

void ShapeViewerGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);
}

void ShapeViewerGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool /*scaled*/) {
	if (_archives.empty()) {
		Close();
		return;
	}

	surf->Fill32(_background, _dims);

	int32 posx = (_dims.width() - _shapeW) / 2 + _shapeX;
	int32 posy = (_dims.height() - _shapeH) / 2 + _shapeY - 25;

	ShapeArchive *archive = _archives[_curArchive]._archive;
	const Shape *shape = archive->getShape(_curShape);
	if (shape && _curFrame < shape->frameCount())
		surf->Paint(shape, _curFrame, posx, posy);

	RenderedText *rendtext;
	Font *font = FontManager::get_instance()->getGameFont(_fontNo, true);
	if (!font)
		return;

	unsigned int remaining;

	{
		// Basic shape/frame information
		char buf1[50];
		char buf2[200];
		if (!shape) {
			Common::sprintf_s(buf1, "NULL");
		} else {
			Common::sprintf_s(buf1, "Frame %d of %d", _curFrame+1, shape->frameCount());
		}
		Common::sprintf_s(buf2, "%s:  Shape %d, %s", _archives[_curArchive]._name.c_str(),
				_curShape, buf1);
		rendtext = font->renderText(buf2, remaining);
		rendtext->draw(surf, 8, 10);
		delete rendtext;
	}

	{
		// Dump the pixel val under the mouse cursor:
		int32 mx = 0;
		int32 my = 0;
		char buf2[200];

		Mouse::get_instance()->getMouseCoords(mx, my);
		ScreenSpaceToGump(mx, my);

		int32 relx = mx - (posx - _shapeX);
		int32 rely = my - (posy - _shapeY);
		if (shape && relx >= 0 && rely >= 0 && relx < _shapeW && rely < _shapeH) {
			// get color
			relx -= _shapeX;
			rely -= _shapeY;
			const ShapeFrame *frame = shape->getFrame(_curFrame);
			if (frame && frame->hasPoint(relx, rely)) {
				uint8 rawpx = frame->getPixelAtPoint(relx, rely);
				uint8 px_r = shape->getPalette()->_palette[rawpx * 3];
				uint8 px_g = shape->getPalette()->_palette[rawpx * 3 + 1];
				uint8 px_b = shape->getPalette()->_palette[rawpx * 3 + 2];

				Common::sprintf_s(buf2, "px: (%d, %d)(%d, %d): %d (%d, %d, %d)", relx, rely, frame->_xoff, frame->_yoff, rawpx, px_r, px_g, px_b);
				rendtext = font->renderText(buf2, remaining);
				rendtext->draw(surf, 8, 25);
				delete rendtext;
			}
		}

	}

	{
		// Additional shapeinfo (only in main shapes archive)
		MainShapeArchive *mainshapes = dynamic_cast<MainShapeArchive *>(archive);
		if (!mainshapes || !shape) return;

		char buf3[128];
		char buf4[128];
		char buf5[128];
		char buf6[512];
		const ShapeInfo *info = mainshapes->getShapeInfo(_curShape);
		if (info) {
			Common::sprintf_s(buf3, "x: %d, y: %d, z: %d\n flags: 0x%04X, family: %d",
					info->_x, info->_y, info->_z, info->_flags, info->_family);
			Common::sprintf_s(buf4, "equip type: %d, weight: %d, vol: %d",
					info->_equipType, info->_weight, info->_volume);
			Common::sprintf_s(buf5, "anim:  type: %d, data: %d, speed: %d",
					info->_animType, info->_animData, info->_animSpeed);
			Common::sprintf_s(buf6, "ShapeInfo: %s\n%s\n%s\nUsecode: %s",
					buf3, buf4, buf5, GameData::get_instance()->getMainUsecode()->get_class_name(_curShape));
			rendtext = font->renderText(buf6, remaining);

			int x, y;
			rendtext->getSize(x, y);
			rendtext->draw(surf, 8, _dims.height() - y);
			delete rendtext;
		}
	}
}

bool ShapeViewerGump::OnKeyDown(int key, int mod) {
	ShapeArchive *archive = _archives[_curArchive]._archive;
	bool shapechanged = false;
	unsigned int delta = 1;
	if (mod & Common::KBD_SHIFT) delta = 10;

	switch (key) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_k:
		if (delta >= archive->getCount())
			delta = 1;
		if (_curShape < delta)
			_curShape = archive->getCount() + _curShape - delta;
		else
			_curShape -= delta;
		shapechanged = true;
		_curFrame = 0;
		break;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_j:
		if (delta >= archive->getCount())
			delta = 1;
		if (_curShape + delta >= archive->getCount())
			_curShape = _curShape + delta - archive->getCount();
		else
			_curShape += delta;
		_curFrame = 0;
		shapechanged = true;
		break;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_h: {
		const Shape *shape = archive->getShape(_curShape);
		if (shape && shape->frameCount()) {
			if (delta >= shape->frameCount()) delta = 1;
			if (_curFrame < delta)
				_curFrame = shape->frameCount() + _curFrame - delta;
			else
				_curFrame -= delta;
		}
	}
	break;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_l: {
		const Shape *shape = archive->getShape(_curShape);
		if (shape && shape->frameCount()) {
			if (delta >= shape->frameCount()) delta = 1;
			if (_curFrame + delta >= shape->frameCount())
				_curFrame = _curFrame + delta - shape->frameCount();
			else
				_curFrame += delta;
		}
	}
	break;
	case Common::KEYCODE_COMMA:
	case Common::KEYCODE_PAGEUP: {
		if (_curArchive == 0)
			_curArchive = _archives.size() - 1;
		else
			_curArchive--;

		archive = _archives[_curArchive]._archive;
		shapechanged = true;
		_curShape = 0;
		_curFrame = 0;
	}
	break;
	case Common::KEYCODE_PERIOD:
	case Common::KEYCODE_PAGEDOWN: {
		if (_curArchive + 1 == _archives.size())
			_curArchive = 0;
		else
			_curArchive++;

		archive = _archives[_curArchive]._archive;
		shapechanged = true;
		_curShape = 0;
		_curFrame = 0;
	}
	break;
	case Common::KEYCODE_f: {
		_fontNo++;
		if (_fontNo >= GameData::get_instance()->getFonts()->getCount() ||
			_fontNo > 17) {
			_fontNo = 0;
		}
	}
	break;
	case Common::KEYCODE_b: {
		_background += 0x808080;
		_background &= 0xF0F0F0;
	} break;
	case Common::KEYCODE_ESCAPE: {
		Close();
	}
	break;
	default:
		break;
	}

	if (shapechanged) {
		const Shape *shape = archive->getShape(_curShape);
		if (shape)
			shape->getTotalDimensions(_shapeW, _shapeH, _shapeX, _shapeY);
	}

	return true;
}

//static
void ShapeViewerGump::U8ShapeViewer() {
	GameData *gamedata = GameData::get_instance();

	Common::Array<ShapeArchiveEntry> archives;
	archives.push_back(ShapeArchiveEntry("shapes", gamedata->getMainShapes()));
	archives.push_back(ShapeArchiveEntry("gumps", gamedata->getGumps()));
	archives.push_back(ShapeArchiveEntry("fonts", gamedata->getFonts()));

	ShapeArchive *mouseShapes = new MouseShapeArchive(gamedata->getMouse(), GameData::OTHER);
	archives.push_back(ShapeArchiveEntry("mouse", mouseShapes, DisposeAfterUse::YES));

	FileSystem *filesys = FileSystem::get_instance();
	Common::SeekableReadStream *eintro = filesys->ReadFile("static/eintro.skf");
	if (eintro) {
		ShapeArchive *eintroshapes = new ShapeArchive(eintro, GameData::OTHER,
		        PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game),
		        &U8SKFShapeFormat);
		archives.push_back(ShapeArchiveEntry("eintro", eintroshapes, DisposeAfterUse::YES));
	}

	Common::SeekableReadStream *endgame = filesys->ReadFile("static/endgame.skf");
	if (endgame) {
		ShapeArchive *endgameshapes = new ShapeArchive(endgame, GameData::OTHER,
		        PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game),
		        &U8SKFShapeFormat);
		archives.push_back(ShapeArchiveEntry("endgame", endgameshapes, DisposeAfterUse::YES));
	}

	Gump *desktopGump = Ultima8Engine::get_instance()->getDesktopGump();
	Rect res;
	desktopGump->GetDims(res);

	int xoff, yoff, width, height;

	if (res.height() > 240) {
		width = (res.width() * 4) / 5;
		height = (res.height() * 5) / 6;
		xoff = res.width() / 10;
		yoff = res.height() / 12;
	} else {
		width = (res.width() * 9) / 10;
		height = (res.height() * 11) / 12;
		xoff = res.width() / 20;
		yoff = res.height() / 24;
	}

	ModalGump *gump = new ShapeViewerGump(xoff, yoff, width, height, archives);
	gump->InitGump(0);
}

bool ShapeViewerGump::loadData(Common::ReadStream *rs) {
	warning("Trying to load ModalGump");
	return false;
}

void ShapeViewerGump::saveData(Common::WriteStream *ws) {
	warning("Trying to save ModalGump");
}

} // End of namespace Ultima8
} // End of namespace Ultima
