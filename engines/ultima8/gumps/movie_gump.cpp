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
#include "ultima8/gumps/movie_gump.h"

#include "ultima8/filesys/raw_archive.h"
#include "ultima8/graphics/skf_player.h"
#include "ultima8/ultima8.h"
#include "ultima8/gumps/desktop_gump.h"
#include "ultima8/gumps/gump_notify_process.h"

#include "ultima8/filesys/file_system.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MovieGump, ModalGump)

MovieGump::MovieGump() : ModalGump(), player(0) {

}

MovieGump::MovieGump(int width, int height, RawArchive *movie,
                     bool introMusicHack, uint32 _Flags, int32 layer_)
	: ModalGump(50, 50, width, height, 0, _Flags, layer_) {
	player = new SKFPlayer(movie, width, height, introMusicHack);
}

MovieGump::~MovieGump() {
	delete player;
}

void MovieGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);
	player->start();

	Mouse::get_instance()->pushMouseCursor();
	Mouse::get_instance()->setMouseCursor(Mouse::MOUSE_NONE);
}

void MovieGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();

	ModalGump::Close(no_del);
}

void MovieGump::run() {
	ModalGump::run();

	player->run();
	if (!player->isPlaying()) {
		Close();
	}
}

void MovieGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	player->paint(surf, lerp_factor);
}

bool MovieGump::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_ESCAPE: {
		Close();
	}
	break;
	default:
		break;
	}

	return true;
}

//static
ProcId MovieGump::U8MovieViewer(RawArchive *movie, bool introMusicHack) {
	Gump *gump = new MovieGump(320, 200, movie, introMusicHack);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
	gump->CreateNotifier();
	return gump->GetNotifyProcess()->getPid();
}

//static
void MovieGump::ConCmd_play(const Console::ArgvType &argv) {
	if (argv.size() != 2) {
		pout << "play usage: play <moviename>" << std::endl;
		return;
	}

	std::string filename = "@game/static/" + argv[1] + ".skf";
	FileSystem *filesys = FileSystem::get_instance();
	IDataSource *skf = filesys->ReadFile(filename);
	if (!skf) {
		pout << "movie not found." << std::endl;
		return;
	}

	RawArchive *flex = new RawArchive(skf);
	U8MovieViewer(flex);
}


bool MovieGump::loadData(IDataSource *ids) {
	return false;
}

void MovieGump::saveData(ODataSource *ods) {

}

} // End of namespace Ultima8
