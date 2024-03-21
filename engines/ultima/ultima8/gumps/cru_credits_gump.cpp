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
#include "image/bmp.h"

#include "ultima/ultima8/gumps/cru_credits_gump.h"

#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/audio/music_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CruCreditsGump)

CruCreditsGump::CruCreditsGump()
	: ModalGump(), _timer(0), _background(nullptr), _nextScreenStart(0),
		_screenNo(-1) {
}

CruCreditsGump::CruCreditsGump(Common::SeekableReadStream *txtrs,
									   Common::SeekableReadStream *bmprs,
									   uint32 flags, int32 layer)
		: ModalGump(0, 0, 640, 480, 0, flags, layer),
		_timer(0), _background(nullptr), _nextScreenStart(0), _screenNo(-1)
{
	Image::BitmapDecoder decoder;
	_background = RenderSurface::CreateSecondaryRenderSurface(640, 480);

	uint32 color = TEX32_PACK_RGB(0, 0, 0);
	_background->fill32(color, 0, 0, 640, 480); // black background

	if (decoder.loadStream(*bmprs)) {
		// This does an extra copy via the ManagedSurface, but it's a once-off.
		const Graphics::Surface *bmpsurf = decoder.getSurface();
		Graphics::ManagedSurface ms(bmpsurf);
		ms.setPalette(decoder.getPalette(), 0, decoder.getPaletteColorCount());
		Common::Rect srcRect(640, 480);
		_background->Blit(ms, srcRect, 0, 0);
	} else {
		warning("couldn't load bitmap background for credits.");
	}

	// Lots of extra copies here, but it's only 4kb of text so it's fine.
	CredScreen screen;
	CredLine credline;

	// not sure what these 4 bytes are?
	txtrs->readUint32LE();
	while (!txtrs->eos()) {
		Common::String line = txtrs->readString();
		if (!line.size())
			break;
		credline._text = line.substr(1);
		switch (line[0]) {
			case '@':
				credline._lineType = kCredTitle;
				screen._lines.push_back(credline);
				break;
			case '$':
				credline._lineType = kCredName;
				screen._lines.push_back(credline);
				break;
			case '*': {
				unsigned int i = 1;
				while (i < line.size() && line[i] == '*')
					i++;
				screen._delay = 60 * i;
				_screens.push_back(screen);
				screen._lines.clear();
				break;
			}
			default:
				if (line.size())
					debug(6, "unhandled line in credits: %s", line.c_str());
				break;
		}
	}
}

CruCreditsGump::~CruCreditsGump() {
	delete _background;

	for (Common::Array<RenderedText *>::iterator iter = _currentLines.begin(); iter != _currentLines.end(); iter++) {
		delete *iter;
	}
}

void CruCreditsGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	Mouse::get_instance()->pushMouseCursor(Mouse::MOUSE_NONE);

	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) {
		if (GAME_IS_REMORSE)
			musicproc->playMusic(19);
		else
			musicproc->playMusic(17);
	}
}

void CruCreditsGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();

	ModalGump::Close(no_del);

	// Just let it play out?
	//MusicProcess *musicproc = MusicProcess::get_instance();
	//if (musicproc) musicproc->restoreMusic();
}

void CruCreditsGump::run() {
	ModalGump::run();

	_timer++;

	if (_timer < _nextScreenStart)
		return;

	_screenNo++;
	if (_screenNo >= static_cast<int>(_screens.size())) {
		Close();
		return;
	}

	_nextScreenStart += _screens[_screenNo]._delay;
	for (Common::Array<RenderedText *>::iterator iter = _currentLines.begin();
		 iter != _currentLines.end(); iter++) {
		delete *iter;
	}
	_currentLines.clear();

	const Common::Array<CredLine> &lines = _screens[_screenNo]._lines;

	Font *titlefont = FontManager::get_instance()->getGameFont(16, true);
	Font *namefont = FontManager::get_instance()->getGameFont(17, true);
	Palette *pal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Cred);

	ShapeFont *titleshapefont = dynamic_cast<ShapeFont *>(titlefont);
	if (pal && titleshapefont)
		titleshapefont->setPalette(pal);
	ShapeFont *nameshapefont = dynamic_cast<ShapeFont *>(namefont);
	if (pal && nameshapefont)
		nameshapefont->setPalette(pal);

	for (Common::Array<CredLine>::const_iterator iter = lines.begin();
		 iter != lines.end(); iter++) {
		Font *linefont = (iter->_lineType == kCredTitle) ? titlefont : namefont;
		if (!linefont) {
			// shouldn't happen.. just to be sure?
			warning("can't render credits line type %d, font is null", iter->_lineType);
			break;
		}

		unsigned int remaining;
		RenderedText *rendered = linefont->renderText(iter->_text, remaining, 640, 0, Font::TEXT_CENTER);
		_currentLines.push_back(rendered);
	}
}

void CruCreditsGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Common::Rect srcRect(640, 480);
	surf->Blit(*_background->getRawSurface(), srcRect, 0, 0);

	unsigned int nlines = _currentLines.size();
	if (!nlines)
		return;

	int width, height;
	_currentLines[0]->getSize(width, height);
	int vlead = _currentLines[0]->getVlead();

	int total = nlines * (height + vlead);
	int yoffset = 240 - total / 2;

	for (Common::Array<RenderedText *>::iterator iter = _currentLines.begin();
		 iter != _currentLines.end(); iter++) {
		(*iter)->draw(surf, 0, yoffset);
		yoffset += (height + vlead);
	}
}

bool CruCreditsGump::OnKeyDown(int key, int mod) {
	if (key == Common::KEYCODE_ESCAPE)
		Close();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
