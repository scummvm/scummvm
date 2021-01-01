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
#include "ultima/ultima8/gumps/movie_gump.h"

#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/graphics/avi_player.h"
#include "ultima/ultima8/graphics/skf_player.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/graphics/fade_to_modal_process.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/gumps/cru_status_gump.h"

#include "ultima/ultima8/filesys/file_system.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MovieGump)

MovieGump::MovieGump() : ModalGump(), _player(nullptr) {

}

MovieGump::MovieGump(int width, int height, Common::SeekableReadStream *rs,
                     bool introMusicHack, const byte *overridePal,
					 uint32 flags, int32 layer)
		: ModalGump(50, 50, width, height, 0, flags, layer) {
	uint32 stream_id = rs->readUint32BE();
	rs->seek(-4, SEEK_CUR);
	if (stream_id == 0x52494646) {// 'RIFF' - crusader AVIs
		_player = new AVIPlayer(rs, width, height, overridePal);
	} else {
		_player = new SKFPlayer(rs, width, height, introMusicHack);
	}
}

MovieGump::~MovieGump() {
	delete _player;
}

void MovieGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	_player->start();

	Mouse::get_instance()->pushMouseCursor();
	Mouse::get_instance()->setMouseCursor(Mouse::MOUSE_NONE);

	CruStatusGump *statusgump = CruStatusGump::get_instance();
	if (statusgump) {
		statusgump->HideGump();
	}
}

void MovieGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();

	CruStatusGump *statusgump = CruStatusGump::get_instance();
	if (statusgump) {
		statusgump->UnhideGump();
	}

	_player->stop();

	ModalGump::Close(no_del);
}

void MovieGump::run() {
	ModalGump::run();

	_player->run();
	if (!_player->isPlaying()) {
		Close();
	}
}

void MovieGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	_player->paint(surf, lerp_factor);
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
ProcId MovieGump::U8MovieViewer(Common::SeekableReadStream *rs, bool fade, bool introMusicHack) {
	ModalGump *gump;
	if (GAME_IS_U8)
		gump = new MovieGump(320, 200, rs, introMusicHack);
	else
		gump = new MovieGump(640, 480, rs, introMusicHack);

	if (fade) {
		FadeToModalProcess *p = new FadeToModalProcess(gump);
		Kernel::get_instance()->addProcess(p);
		return p->getPid();
	}
	else
	{
		gump->InitGump(nullptr);
		gump->setRelativePosition(CENTER);
		gump->CreateNotifier();
		return gump->GetNotifyProcess()->getPid();
	}
}

bool MovieGump::loadData(Common::ReadStream *rs) {
	return false;
}

void MovieGump::saveData(Common::WriteStream *ws) {

}

static Std::string _fixCrusaderMovieName(const Std::string &s) {
	/*
	 HACK! The game comes with movies MVA01.AVI etc, but the usecode mentions both
	 MVA01 and MVA1.  We do a translation here.	 These are the strings we need to fix:
	 008E: 0D	push string	"mva1"
	 036D: 0D	push string	"mva3a"
	 04E3: 0D	push string	"mva4"
	 0656: 0D	push string	"mva5a"
	 07BD: 0D	push string	"mva6"
	 0944: 0D	push string	"mva7"
	 0A68: 0D	push string	"mva8"
	 0B52: 0D	push string	"mva9"
	*/
	if (s.size() == 4)
		return Std::string::format("mva0%c", s[3]);
	else if (s.equals("mva3a"))
		return "mva03a";
	else if (s.equals("mva5a"))
		return "mva05a";

	return s;
}

static Common::SeekableReadStream *_tryLoadCruMovie(const Std::string &filename) {
	const Std::string path = Std::string::format("@game/flics/%s.avi", filename.c_str());
	FileSystem *filesys = FileSystem::get_instance();
	Common::SeekableReadStream *rs = filesys->ReadFile(path);
	if (!rs) {
		// Try with a "0" in the name
		const Std::string adjustedfn = Std::string::format("@game/flics/0%s.avi", filename.c_str());
		rs = filesys->ReadFile(adjustedfn);
		if (!rs) {
			warning("movie %s not found", filename.c_str());
			return 0;
		}
	}
	return rs;
}

uint32 MovieGump::I_playMovieOverlay(const uint8 *args,
        unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(name);
	ARG_UINT16(x);
	ARG_UINT16(y);

	PaletteManager *palman = PaletteManager::get_instance();

	if (item && palman) {
		if (name.hasPrefix("mva")) {
			name = _fixCrusaderMovieName(name);
		}

		const Palette *pal = palman->getPalette(PaletteManager::Pal_Game);
		assert(pal);

		Common::SeekableReadStream *rs = _tryLoadCruMovie(name);
		if (rs) {
			Gump *gump = new MovieGump(x, y, rs, false, pal->_palette);
			gump->InitGump(nullptr, true);
			gump->setRelativePosition(CENTER);
		}
	}

	return 0;
}

uint32 MovieGump::I_playMovieCutscene(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(name);
	ARG_UINT16(x);
	ARG_UINT16(y);

	if (item) {
		Common::SeekableReadStream *rs = _tryLoadCruMovie(name);
		if (rs) {
			// TODO: Support playback with gap lines for the CRT effect
			Gump *gump = new MovieGump(x * 3, y * 3, rs, false);
			gump->InitGump(nullptr, true);
			gump->setRelativePosition(CENTER);
		}
	}

	return 0;

}


} // End of namespace Ultima8
} // End of namespace Ultima
