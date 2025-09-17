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

#include "common/file.h"

#include "ultima/ultima8/gumps/movie_gump.h"

#include "ultima/ultima8/gfx/avi_player.h"
#include "ultima/ultima8/gfx/skf_player.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/gfx/texture.h"
#include "ultima/ultima8/gfx/fade_to_modal_process.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/games/game.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/gumps/cru_status_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"

namespace Ultima {
namespace Ultima8 {

// Some fourCCs used in IFF files
static const uint32 IFF_MAGIC   = MKTAG('F', 'O', 'R', 'M');
static const uint32 IFF_LANG    = MKTAG('L', 'A', 'N', 'G');
static const uint32 IFF_LANG_FR = MKTAG('F', 'R', 'E', 'N');
static const uint32 IFF_LANG_EN = MKTAG('E', 'N', 'G', 'L');
static const uint32 IFF_LANG_DE = MKTAG('G', 'E', 'R', 'M');

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

static Common::SeekableReadStream *_tryLoadCruMovieFile(const Std::string &filename, const char *extn) {
	const Std::string path = Std::string::format("flics/%s.%s", filename.c_str(), extn);
	auto *rs = new Common::File();
	if (!rs->open(path.c_str())) {
		// Try with a "0" in the name
		const Std::string adjustedfn = Std::string::format("flics/0%s.%s", filename.c_str(), extn);
		if (!rs->open(adjustedfn.c_str())) {
			delete rs;
			return nullptr;
		}
	}
	return rs;
}

static Common::SeekableReadStream *_tryLoadCruAVI(const Std::string &filename) {
	Common::SeekableReadStream *rs = _tryLoadCruMovieFile(filename, "avi");
	if (!rs)
		warning("movie %s not found", filename.c_str());
	return rs;
}

// Convenience function that tries to open both TXT (No Remorse)
// and IFF (No Regret) subtitle formats.
static Common::SeekableReadStream *_tryLoadCruSubtitle(const Std::string &filename) {
	Common::SeekableReadStream *txtfile = _tryLoadCruMovieFile(filename, "txt");
	if (txtfile)
		return txtfile;
	return _tryLoadCruMovieFile(filename, "iff");
}


DEFINE_RUNTIME_CLASSTYPE_CODE(MovieGump)

MovieGump::MovieGump() : ModalGump(), _player(nullptr), _subtitleWidget(0), _lastFrameNo(-1) {

}

MovieGump::MovieGump(int width, int height, Common::SeekableReadStream *rs,
					 bool introMusicHack, bool noScale, const byte *overridePal,
					 uint32 flags, int32 layer)
		: ModalGump(50, 50, width, height, 0, flags, layer), _subtitleWidget(0), _lastFrameNo(-1) {
	uint32 stream_id = rs->readUint32BE();
	rs->seek(-4, SEEK_CUR);
	if (stream_id == 0x52494646) {// 'RIFF' - crusader AVIs
		_player = new AVIPlayer(rs, width, height, overridePal, noScale);
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

	Mouse::get_instance()->pushMouseCursor(Mouse::MOUSE_NONE);

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

	// TODO: It would be nice to refactor this
	AVIPlayer *aviplayer = dynamic_cast<AVIPlayer *>(_player);
	if (aviplayer) {
		// The AVI player can skip frame numbers, so search back from the
		// last frame to make sure we don't miss subtitles
		const int frameno = aviplayer->getFrameNo();
		for (int f = _lastFrameNo + 1; f <= frameno; f++) {
			if (_subtitles.contains(f)) {
				TextWidget *subtitle = dynamic_cast<TextWidget *>(getGump(_subtitleWidget));
				if (subtitle)
					subtitle->Close();
				// Create a new TextWidget. No Regret uses font 3
				int subtitle_font = GAME_IS_REMORSE ? 4 : 3;
				TextWidget *widget = new TextWidget(0, 0, _subtitles[f], true, subtitle_font, 640, 10);
				widget->InitGump(this);
				widget->setRelativePosition(BOTTOM_CENTER, 0, -10);
				// Subtitles should be white.
				widget->setBlendColour(TEX32_PACK_RGBA(0xFF, 0xFF, 0xFF, 0xFF));
				_subtitleWidget = widget->getObjId();
			}
		}
		_lastFrameNo = frameno;
	}

	if (!_player->isPlaying()) {
		Close();
	}
}

void MovieGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
	_player->paint(surf, lerp_factor);

	// If displaying subtitles, put a black box behind them.  The box should be ~600px across.
	if (_subtitleWidget) {
		TextWidget *subtitle = dynamic_cast<TextWidget *>(getGump(_subtitleWidget));
		if (subtitle) {
			int32 x, y;
			
			subtitle->getLocation(x, y);
			Common::Rect32 textdims = subtitle->getDims();
			Common::Rect32 screendims = surf->getSurfaceDims();
			surf->fill32(TEX32_PACK_RGB(0, 0, 0),
						 screendims.width() / 2 - 300 - screendims.left,
						 y - 3,
						 600,
						 textdims.height() + 5);
		}
	}

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

void MovieGump::ClearPlayerOffset() {
	if (!_shape || !_player)
		return;
	_player->setOffset(0, 0);
}

/*static*/
ProcId MovieGump::U8MovieViewer(Common::SeekableReadStream *rs, bool fade, bool introMusicHack, bool noScale) {
	ModalGump *gump;
	if (GAME_IS_U8)
		gump = new MovieGump(320, 200, rs, introMusicHack, noScale);
	else
		gump = new MovieGump(640, 480, rs, introMusicHack, noScale);

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

/*static*/ MovieGump *MovieGump::CruMovieViewer(const Std::string fname, int x, int y, const byte *pal, Gump *parent, uint16 frameshape) {
	Common::SeekableReadStream *rs = _tryLoadCruAVI(fname);
	if (!rs)
		return nullptr;

	MovieGump *gump = new MovieGump(x, y, rs, false, false, pal);

	gump->InitGump(parent, true);

	if (frameshape) {
		GumpShapeArchive *gumpshapes = GameData::get_instance()->getGumps();
		if (!gumpshapes) {
			warning("failed to add movie frame: no gump shape archive");
		} else {
			gump->SetShape(gumpshapes->getShape(frameshape), 0);
			gump->UpdateDimsFromShape();
			gump->ClearPlayerOffset();
		}
	}

	gump->setRelativePosition(CENTER);
	gump->loadSubtitles(_tryLoadCruSubtitle(fname));
	return gump;
}

void MovieGump::loadSubtitles(Common::SeekableReadStream *rs) {
	if (!rs)
		return;

	const uint32 id = rs->readUint32BE();
	rs->seek(0);

	if (id == IFF_MAGIC) {
		loadIFFSubs(rs);
	} else {
		loadTXTSubs(rs);
	}
}

void MovieGump::loadTXTSubs(Common::SeekableReadStream *rs) {
	int frameno = 0;
	while (!rs->eos()) {
		Common::String line = rs->readLine();
		if (line.hasPrefix("@frame ")) {
			if (frameno > 0) {
				// two @frame directives in a row means that the last
				// subtitle should be turned *off* at the first frame
				_subtitles[frameno] = "";
			}
			frameno = atoi(line.c_str() + 7);
		} else if (frameno >= 0) {
			_subtitles[frameno] = line;
			frameno = -1;
		}
	}
}

void MovieGump::loadIFFSubs(Common::SeekableReadStream *rs) {
	uint32 magic = rs->readUint32BE();
	if (magic != IFF_MAGIC) {
		warning("Error loading IFF file, invalid magic.");
		return;
	}

	rs->skip(2);
	uint16 totalsize = rs->readUint16BE();
	if (totalsize != rs->size() - rs->pos()) {
		warning("Error loading IFF file: size invalid.");
		return;
	}

	uint32 lang_magic = rs->readUint32BE();
	if (lang_magic != IFF_LANG) {
		warning("Error loading IFF file: invalid magic.");
		return;
	}

	const Common::Language lang = Ultima8Engine::get_instance()->getLanguage();
	while (rs->pos() < rs->size()) {
		uint32 lang_code = rs->readUint32BE();
		uint32 lang_len = rs->readUint32BE();
		uint32 lang_end = rs->pos() + lang_len;
		if ((lang == Common::FR_FRA && lang_code == IFF_LANG_FR)
			|| (lang == Common::DE_DEU && lang_code == IFF_LANG_DE)
			|| (lang == Common::EN_ANY && lang_code == IFF_LANG_EN)) {
			while (rs->pos() < lang_end) {
				// Take care of the mix of LE and BE.
				uint16 frameoff = rs->readUint16LE();
				rs->skip(1); // what's this?
				uint32 slen = rs->readUint16BE();
				const Common::String line = rs->readString('\0', slen);
				_subtitles[frameoff] = line;
			}
		} else {
			rs->skip(lang_len);
		}
	}
}

bool MovieGump::loadData(Common::ReadStream *rs) {
	return false;
}

void MovieGump::saveData(Common::WriteStream *ws) {

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

		CruMovieViewer(name, x, y, pal->data(), nullptr, 52);
	}

	return 0;
}

uint32 MovieGump::I_playMovieCutscene(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(name);
	ARG_UINT16(x);
	ARG_UINT16(y);

	if (item) {
		CruMovieViewer(name, x * 3, y * 3, nullptr, nullptr, 0);
	}

	return 0;
}

uint32 MovieGump::I_playMovieCutsceneAlt(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item); // TODO: Unused? Center on this first?
	ARG_STRING(name);
	ARG_UINT16(x);
	ARG_UINT16(y);

	if (!x)
		x = 640;
	else
		x *= 3;

	if (!y)
		y = 480;
	else
		y *= 3;

	warning("MovieGump::I_playMovieCutsceneAlt: TODO: This intrinsic should pause and fade the background to grey (%s, %d)",
			name.c_str(), item ? item->getObjId() : 0);

	CruMovieViewer(name, x, y, nullptr, nullptr, 0);

	return 0;
}

uint32 MovieGump::I_playMovieCutsceneRegret(const uint8 *args, unsigned int /*argsize*/) {
	ARG_STRING(name);
	ARG_UINT8(fade);

	warning("MovieGump::I_playMovieCutsceneRegret: TODO: use fade argument %d", fade);

	CruMovieViewer(name, 640, 480, nullptr, nullptr, 0);

	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
