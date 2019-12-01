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
#include "ultima8/gumps/shape_viewer_gump.h"

#include "ShapeArchive.h"
#include "ultima8/graphics/render_surface.h"
#include "ultima8/kernel/gui_app.h"
#include "Shape.h"
#include "ultima8/graphics/shape_info.h"

#include "RenderedText.h"
#include "ultima8/graphics/fonts/font.h"
#include "ultima8/graphics/fonts/font_manager.h"

#include "ultima8/games/game_data.h"
#include "FontShapeArchive.h"
#include "MainShapeArchive.h"
#include "GumpShapeArchive.h"
#include "desktop_gump.h"

#include "ultima8/filesys/file_system.h"
#include "u8/ConvertShapeU8.h"
#include "ultima8/graphics/palette_manager.h"
#include "Usecode.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ShapeViewerGump, ModalGump);

ShapeViewerGump::ShapeViewerGump()
	: ModalGump(), curflex(0), flex(0), curshape(0), curframe(0), background(0) {

}

ShapeViewerGump::ShapeViewerGump(int width, int height,
                                 std::vector<std::pair<std::string, ShapeArchive *> > &flexes_,
                                 uint32 _Flags, int32 layer)
	: ModalGump(50, 50, width, height, 0, _Flags, layer),
	  flexes(flexes_), curflex(0), curshape(0), curframe(0), background(0) {
	if (flexes.size())
		flex = flexes[0].second;
	else
		flex = 0;
}

ShapeViewerGump::~ShapeViewerGump() {

}

void ShapeViewerGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool /*scaled*/) {
	if (flexes.empty()) {
		Close();
		return;
	}

	surf->Fill32(background, 0, 0, dims.w, dims.h);

	int32 posx = (dims.w - shapew) / 2 + shapex;
	int32 posy = (dims.h - shapeh) / 2 + shapey;

	Shape *shape = flex->getShape(curshape);
	if (shape && curframe < shape->frameCount())
		surf->Paint(shape, curframe, posx, posy);

	RenderedText *rendtext;
	Pentagram::Font *font = FontManager::get_instance()->getGameFont(0, true);
	unsigned int remaining;

	char buf1[50];
	char buf2[200];
	if (!shape) {
		sprintf(buf1, "NULL");
	} else {
		sprintf(buf1, "Frame %d of %d", curframe, shape->frameCount());
	}
	sprintf(buf2, "%s\nShape %d, %s", flexes[curflex].first.c_str(),
	        curshape, buf1);
	rendtext = font->renderText(buf2, remaining);
	rendtext->draw(surf, 20, 20);
	delete rendtext;

	MainShapeArchive *mainshapes = p_dynamic_cast<MainShapeArchive *>(flex);
	if (!mainshapes || !shape) return;

	char buf3[128];
	char buf4[128];
	char buf5[128];
	char buf6[512];
	ShapeInfo *info = mainshapes->getShapeInfo(curshape);
	if (info) {
		sprintf(buf3, "x = %d, y = %d, z = %d\n flags = 0x%04X, family = %d",
		        info->x, info->y, info->z, info->flags, info->family);
		sprintf(buf4, "equip type = %d\n unknown flags = 0x%02X\n weight = %d",
		        info->equiptype, info->unknown, info->weight);
		sprintf(buf5, "volume = %d\n animtype = %d, animdata = %d",
		        info->animtype, info->animdata, info->volume);
		sprintf(buf6, "ShapeInfo:\n %s\n %s, %s\nUsecode: %s",
		        buf3, buf4, buf5, GameData::get_instance()->getMainUsecode()->get_class_name(curshape));
		rendtext = font->renderText(buf6, remaining);
		rendtext->draw(surf, 300, 20);
		delete rendtext;
	}
}

bool ShapeViewerGump::OnKeyDown(int key, int mod) {
	bool shapechanged = false;
	unsigned int delta = 1;
	if (mod & KMOD_SHIFT) delta = 10;

	switch (key) {
	case SDLK_UP:
		if (delta >= flex->getCount()) delta = 1;
		if (curshape < delta)
			curshape = flex->getCount() + curshape - delta;
		else
			curshape -= delta;
		shapechanged = true;
		curframe = 0;
		break;
	case SDLK_DOWN:
		if (delta >= flex->getCount()) delta = 1;
		if (curshape + delta >= flex->getCount())
			curshape = curshape + delta - flex->getCount();
		else
			curshape += delta;
		curframe = 0;
		shapechanged = true;
		break;
	case SDLK_LEFT: {
		Shape *shape = flex->getShape(curshape);
		if (shape && shape->frameCount()) {
			if (delta >= shape->frameCount()) delta = 1;
			if (curframe < delta)
				curframe = shape->frameCount() + curframe - delta;
			else
				curframe -= delta;
		}
	}
	break;
	case SDLK_RIGHT: {
		Shape *shape = flex->getShape(curshape);
		if (shape && shape->frameCount()) {
			if (delta >= shape->frameCount()) delta = 1;
			if (curframe + delta >= shape->frameCount())
				curframe = curframe + delta - shape->frameCount();
			else
				curframe += delta;
		}
	}
	break;
	case SDLK_PAGEUP: {
		if (curflex == 0)
			curflex = flexes.size() - 1;
		else
			curflex--;

		flex = flexes[curflex].second;
		shapechanged = true;
		curshape = 0;
		curframe = 0;
	}
	break;
	case SDLK_PAGEDOWN: {
		if (curflex + 1 == flexes.size())
			curflex = 0;
		else
			curflex++;

		flex = flexes[curflex].second;
		shapechanged = true;
		curshape = 0;
		curframe = 0;
	}
	break;
	case SDLK_ESCAPE: {
		Close();
	}
	break;
	default:
		break;
	}

	if (shapechanged) {
		Shape *shape = flex->getShape(curshape);
		if (shape)
			shape->getTotalDimensions(shapew, shapeh, shapex, shapey);
	}

	return true;
}

bool ShapeViewerGump::OnTextInput(int unicode) {
	switch (unicode) {
	case 'b':
		background += 0x808080;
		background &= 0xF0F0F0;
		break;
	default:
		break;
	}

	return true;
}


//static
void ShapeViewerGump::U8ShapeViewer() {
	GameData *gamedata = GameData::get_instance();

	std::vector<std::pair<std::string, ShapeArchive *> > flexes;
	std::pair<std::string, ShapeArchive *> flex;
	flex.first = "u8shapes";
	flex.second = gamedata->getMainShapes();
	flexes.push_back(flex);
	flex.first = "u8gumps";
	flex.second = gamedata->getGumps();
	flexes.push_back(flex);
	flex.first = "u8fonts";
	flex.second = gamedata->getFonts();
	flexes.push_back(flex);
	FileSystem *filesys = FileSystem::get_instance();
	IDataSource *eintro = filesys->ReadFile("@game/static/eintro.skf");
	if (eintro) {
		ShapeArchive *eintroshapes = new ShapeArchive(eintro, GameData::OTHER,
		        PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game),
		        &U8SKFShapeFormat);
		flex.first = "eintro";
		flex.second = eintroshapes;
		flexes.push_back(flex);
		// !! memory leak
	}

	IDataSource *endgame = filesys->ReadFile("@game/static/endgame.skf");
	if (endgame) {
		ShapeArchive *endgameshapes = new ShapeArchive(endgame, GameData::OTHER,
		        PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game),
		        &U8SKFShapeFormat);
		flex.first = "endgame";
		flex.second = endgameshapes;
		flexes.push_back(flex);
		// !! memory leak
	}

	Gump *desktopGump = GUIApp::get_instance()->getDesktopGump();
	Pentagram::Rect res;
	desktopGump->GetDims(res);

	ModalGump *gump = new ShapeViewerGump((res.w * 3) / 4, (res.h * 3) / 4, flexes);
	gump->InitGump(0);
}

void ShapeViewerGump::ConCmd_U8ShapeViewer(const Console::ArgvType &argv) {
	ShapeViewerGump::U8ShapeViewer();
}

bool ShapeViewerGump::loadData(IDataSource *ids) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return false;
}

void ShapeViewerGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}
