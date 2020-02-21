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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/game_map_gump.h"

namespace Ultima {
namespace Ultima8 {

Debugger::Debugger() : Shared::Debugger() {
	registerCmd("GameMapGump::toggleHighlightItems", WRAP_METHOD(Debugger, cmdToggleHighlightItems));
	registerCmd("GameMapGump::dumpMap", WRAP_METHOD(Debugger, cmdDumpMap));
	registerCmd("GameMapGump::incrementSortOrder", WRAP_METHOD(Debugger, cmdIncrementSortOrder));
	registerCmd("GameMapGump::decrementSortOrder", WRAP_METHOD(Debugger, cmdDecrementSortOrder));
}

bool Debugger::cmdToggleHighlightItems(int argc, const char **argv) {
	GameMapGump::Set_highlightItems(!GameMapGump::is_highlightItems());
	return false;
}

bool Debugger::cmdDumpMap(int argc, const char **argv) {
#ifdef TODO
	// We only support 32 bits per pixel for now
	if (RenderSurface::format.s_bpp != 32) return;

	// Save because we're going to potentially break the game by enlarging
	// the fast area and available object IDs.
	Std::string savefile = "@save/dumpmap";
	Ultima8Engine::get_instance()->saveGame(savefile, "Pre-dumpMap save");

	// Increase number of available object IDs.
	ObjectManager::get_instance()->allow64kObjects();

	// Actual size
	int32 awidth = 8192;
	int32 aheight = 8192;

	int32 xpos = 0;
	int32 ypos = 0;

	int32 left = 16384;
	int32 right = -16384;
	int32 top = 16384;
	int32 bot = -16384;

	int32 camheight = 256;

	// Work out the map limit we do this very coarsly
	// Now render the map
	for (int32 y = 0; y < 64; y++) {
		for (int32 x = 0; x < 64; x++) {
			const Std::list<Item *> *list =
				World::get_instance()->getCurrentMap()->getItemList(x, y);

			// Should iterate the items!
			// (items could extend outside of this chunk and they have height)
			if (list && list->size() != 0) {
				int32 l = (x * 512 - y * 512) / 4 - 128;
				int32 r = (x * 512 - y * 512) / 4 + 128;
				int32 t = (x * 512 + y * 512) / 8 - 256;
				int32 b = (x * 512 + y * 512) / 8;

				t -= 256; // approx. adjustment for height of items in chunk

				if (l < left) left = l;
				if (r > right) right = r;
				if (t < top) top = t;
				if (b > bot) bot = b;
			}
		}
	}

	if (right == -16384) return;

	// camera height
	bot += camheight;
	top += camheight;

	awidth = right - left;
	aheight = bot - top;

	ypos = top;
	xpos = left;

	// Buffer Size
	int32 bwidth = awidth;
	int32 bheight = 256;

	// Tile size
	int32 twidth = bwidth / 8;
	int32 theight = bheight;


	Debugger *g = new Debugger(0, 0, twidth, theight);

	// HACK: Setting both INVISIBLE and TRANSPARENT flags on the Avatar
	// will make him completely invisible.
	getMainActor()->setFlag(Item::FLG_INVISIBLE);
	getMainActor()->setExtFlag(Item::EXT_TRANSPARENT);
	World::get_instance()->getCurrentMap()->setWholeMapFast();

	RenderSurface *s = RenderSurface::CreateSecondaryRenderSurface(bwidth,
		bheight);
	Texture *t = s->GetSurfaceAsTexture();
	// clear buffer
	Std::memset(t->buffer, 0, 4 * bwidth * bheight);


	// Write tga header
	Std::string filename = "@home/mapdump";
	char buf[32];
	sprintf(buf, "%02d", World::get_instance()->getCurrentMap()->getNum());
	filename += buf;
	filename += ".png";
	ODataSource *ds = FileSystem::get_instance()->WriteFile(filename);
	Std::string pngcomment = "Map ";
	pngcomment += buf;
	pngcomment += ", dumped by Pentagram.";

	PNGWriter *pngw = new PNGWriter(ds);
	pngw->init(awidth, aheight, pngcomment);

	// Now render the map
	for (int32 y = 0; y < aheight; y += theight) {
		for (int32 x = 0; x < awidth; x += twidth) {
			// Work out 'effective' and world coords
			int32 ex = xpos + x + twidth / 2;
			int32 ey = ypos + y + theight / 2;
			int32 wx = ex * 2 + ey * 4;
			int32 wy = ey * 4 - ex * 2;

			s->SetOrigin(x, y % bheight);
			CameraProcess::SetCameraProcess(
				new CameraProcess(wx + 4 * camheight, wy + 4 * camheight, camheight));
			g->Paint(s, 256, false);

		}

		// Write out the current buffer
		if (((y + theight) % bheight) == 0) {
			for (int i = 0; i < bwidth * bheight; ++i) {
				// Convert to correct pixel format
				uint8 r, g, b;
				UNPACK_RGB8(t->buffer[i], r, g, b);
				uint8 *buf = reinterpret_cast<uint8 *>(&t->buffer[i]);
				buf[0] = b;
				buf[1] = g;
				buf[2] = r;
				buf[3] = 0xFF;
			}

			pngw->writeRows(bheight, t);

			// clear buffer for next set
			Std::memset(t->buffer, 0, 4 * bwidth * bheight);
		}
	}

	pngw->finish();
	delete pngw;

	delete ds;

	delete g;
	delete s;

	// Reload
	Ultima8Engine::get_instance()->loadGame(savefile);

	debugPrintf("Map dumped\n");
#endif
	return false;
}

bool Debugger::cmdIncrementSortOrder(int argc, const char **argv) {
	GameMapGump *gump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gump)
		gump->IncSortOrder(1);
	return false;
}

bool Debugger::cmdDecrementSortOrder(int argc, const char **argv) {
	GameMapGump *gump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gump)
		gump->IncSortOrder(-1);
	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
