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
#include "common/events.h"

#include "ultima/ultima8/gumps/credits_gump.h"

#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/texture.h"
#include "ultima/ultima8/gfx/fonts/rendered_text.h"
#include "ultima/ultima8/gfx/fonts/font.h"
#include "ultima/ultima8/gfx/fonts/font_manager.h"
#include "ultima/ultima8/audio/music_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CreditsGump)

CreditsGump::CreditsGump()
	: ModalGump(), _parSkip(0), _timer(0), _title(nullptr),
	  _nextTitle(nullptr), _state(CS_PLAYING),
	  _nextTitleSurf(0), _currentSurface(0), _currentY(0) {
	for (int i = 0; i < 4; i++) {
	  _scroll[i] = nullptr;
	  _scrollHeight[i] = 0;
	}
}

CreditsGump::CreditsGump(const Std::string &text, int parskip,
						 uint32 flags, int32 layer)
		: ModalGump(0, 0, 320, 200, 0, flags, layer), _text(text), _parSkip(parskip),
		_timer(0), _title(nullptr), _nextTitle(nullptr), _state(CS_PLAYING),
		_nextTitleSurf(0), _currentSurface(0), _currentY(0) {
	for (int i = 0; i < 4; i++) {
		_scroll[i] = nullptr;
		_scrollHeight[i] = 0;
	}
}

CreditsGump::~CreditsGump() {
	delete _scroll[0];
	delete _scroll[1];
	delete _scroll[2];
	delete _scroll[3];

	delete _title;
	delete _nextTitle;
}

void CreditsGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	Graphics::Screen *screen = Ultima8Engine::get_instance()->getScreen();
	uint32 width = 256;
	uint32 height = 280;

	_scroll[0] = new RenderSurface(width, height, screen->format);
	_scroll[1] = new RenderSurface(width, height, screen->format);
	_scroll[2] = new RenderSurface(width, height, screen->format);
	_scroll[3] = new RenderSurface(width, height, screen->format);

	uint32 color = TEX32_PACK_RGB(0, 0, 0);
	_scroll[0]->fill32(color, 0, 0, width, height); // black background
	_scroll[1]->fill32(color, 0, 0, width, height);
	_scroll[2]->fill32(color, 0, 0, width, height);
	_scroll[3]->fill32(color, 0, 0, width, height);

	_scrollHeight[0] = 156;
	_scrollHeight[1] = 0;
	_scrollHeight[2] = 0;
	_scrollHeight[3] = 0;

	_currentSurface = 0;
	_currentY = 0;

	Mouse::get_instance()->pushMouseCursor(Mouse::MOUSE_NONE);
}

void CreditsGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();

	ModalGump::Close(no_del);

	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(0);
}

void CreditsGump::extractLine(Std::string &text,
							  char &modifier, Std::string &line) {
	if (!text.empty() && (text[0] == '+' || text[0] == '&' || text[0] == '}' ||
							text[0] == '~' || text[0] == '@')) {
		modifier = text[0];
		text.erase(0, 1);
	} else {
		modifier = 0;
	}

	if (text.empty()) {
		line = "";
		return;
	}

	Std::string::size_type starpos = text.find('*');

	line = text.substr(0, starpos);

	// replace '%%' by '%'.
	// (Original interpreted these strings as format strings??)
	Std::string::size_type ppos;
	while ((ppos = line.find("%%")) != Std::string::npos) {
		line.replace(ppos, 2, "%");
	}

	if (starpos != Std::string::npos) starpos++;
	text.erase(0, starpos);
}


void CreditsGump::run() {
	ModalGump::run();

	if (_timer) {
		_timer--;
		return;
	}

	if (_state == CS_CLOSING) {
		Close();
		return;
	}

	_timer = 1;

	int available = _scrollHeight[_currentSurface] - _currentY;
	int nextblock = -1;
	for (int i = 1; i < 4; i++) {
		available += _scrollHeight[(_currentSurface + i) % 4];
		if (nextblock == -1 && _scrollHeight[(_currentSurface + i) % 4] == 0)
			nextblock = (_currentSurface + i) % 4;
	}
	if (available == 0) nextblock = 0;

	if (_state == CS_FINISHING && available <= 156) {
		debug(6, "CreditsGump: waiting before closing");
		_timer = 120;
		_state = CS_CLOSING;

		if (!_configKey.empty()) {
			ConfMan.setBool(_configKey, true);
			ConfMan.flushToDisk();
		}

		return;
	}

	if (_state == CS_PLAYING && available <= 160) {
		// This shouldn't happen, but just in case..
		if (nextblock == -1)
			nextblock = 0;
		// time to render next block
		Common::Rect32 bounds = _scroll[nextblock]->getSurfaceDims();

		uint32 color = TEX32_PACK_RGB(0, 0, 0);
		_scroll[nextblock]->fill32(color, 0, 0, bounds.width(), bounds.height());

		//color = TEX32_PACK_RGB(0xFF, 0xFF, 0xFF);
		//_scroll[nextblock]->fill32(color, 0, 0, bounds.width(), 2); // block marker
		_scrollHeight[nextblock] = 0;

		Font *redfont, *yellowfont;

		redfont = FontManager::get_instance()->getGameFont(6, true);
		yellowfont = FontManager::get_instance()->getGameFont(8, true);

		bool done = false;
		bool firstline = true;
		while (!_text.empty() && !done) {
			Std::string::size_type endline = _text.find('\n');
			Std::string line = _text.substr(0, endline);

			if (line.empty()) {
				_text.erase(0, 1);
				continue;
			}

			debug(6, "Rendering paragraph: %s", line.c_str());

			if (line[0] == '+') {
				// set _title
				if (!firstline) {
					// if this isn't the first line of the block,
					// postpone setting _title until next block
					done = true;
					continue;
				}

				Std::string titletext;
				char modifier;

				extractLine(line, modifier, titletext);

				unsigned int remaining;
				_nextTitle = redfont->renderText(titletext, remaining, 192, 0,
				                                Font::TEXT_CENTER);

				if (!_title) {
					_title = _nextTitle;
					_nextTitle = nullptr;
				} else {
					_nextTitleSurf = nextblock;
					_scrollHeight[nextblock] = 160; // skip some space
				}

			} else {

				int height = 0;

				Font *font = redfont;
				Font::TextAlign align = Font::TEXT_LEFT;
				int indent = 0;

				while (!line.empty()) {
					Std::string outline;
					char modifier;
					unsigned int remaining;
					extractLine(line, modifier, outline);

					debug(6, "Rendering line: %s", outline.c_str());

					switch (modifier) {
					case '&':
						font = yellowfont;
						align = Font::TEXT_CENTER;
						break;
					case '}':
						font = redfont;
						align = Font::TEXT_CENTER;
						break;
					case '~':
						font = yellowfont;
						align = Font::TEXT_LEFT;
						indent = 32;
						break;
					case '@':
						debug(6, "CreditsGump: done, finishing");
						_state = CS_FINISHING;
						break;
					default:
						break;
					}

					if (!modifier && outline.empty()) {
						height += 48;
						continue;
					}

					if (outline.hasPrefix("&")) {
						// horizontal line

						if (_scrollHeight[nextblock] + height + 7 > bounds.height()) {
							done = true;
							break;
						}

						int linewidth = outline.size() * 8;
						if (linewidth > 192) linewidth = 192;

						color = TEX32_PACK_RGB(0xD4, 0x30, 0x30);
						_scroll[nextblock]->fill32(color, 128 - (linewidth / 2),
						       _scrollHeight[nextblock] + height + 3,
						       linewidth, 1);
						height += 7;
						continue;
					}

					RenderedText *rt = font->renderText(outline, remaining,
					                                    bounds.width() - indent, 0,
					                                    align);
					int xd, yd;
					rt->getSize(xd, yd);

					if (_scrollHeight[nextblock] + height + yd > bounds.height()) {
						delete rt;
						done = true;
						break;
					}

					rt->draw(_scroll[nextblock], indent,
					         _scrollHeight[nextblock] + height +
					         font->getBaseline());

					height += yd + rt->getVlead();

					delete rt;
				}

				if (_state == CS_PLAYING)
					height += _parSkip;

				if (_scrollHeight[nextblock] + height > bounds.height()) {
					if (firstline) {
						height = bounds.height() - _scrollHeight[nextblock];
						assert(height >= 0);
					} else {
						done = true;
					}
				}

				if (done) break; // no room

				_scrollHeight[nextblock] += height;
			}


			if (endline != Std::string::npos) endline++;
			_text.erase(0, endline);

			firstline = false;
		}
	}

	_currentY++;

	if (_currentY >= _scrollHeight[_currentSurface]) {
		// next surface
		_currentY -= _scrollHeight[_currentSurface];
		_scrollHeight[_currentSurface] = 0;
		_currentSurface = (_currentSurface + 1) % 4;

		if (_nextTitle && _currentSurface == _nextTitleSurf) {
			delete _title;
			_title = _nextTitle;
			_nextTitle = nullptr;
		}
	}
}

void CreditsGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	uint32 color = TEX32_PACK_RGB(0, 0, 0);
	surf->fill32(color, 0, 0, 320, 200);

	// line between _title and scroller
	color = TEX32_PACK_RGB(0xD4, 0x30, 0x30);
	surf->fill32(color, 64, 41, 192, 1);

	if (_title)
		_title->draw(surf, 64, 34);

	int h = _scrollHeight[_currentSurface] - _currentY;
	if (h > 156) h = 156;
	if (h > 0) {
		Graphics::ManagedSurface* ms = _scroll[_currentSurface]->getRawSurface();
		Common::Rect srcRect(0, _currentY, ms->w, _currentY + h);
		surf->Blit(*ms, srcRect, 32, 44);
	}

	int y = h;
	for (int i = 1; i < 4; i++) {
		if (h == 156) break;

		int s = (_currentSurface + i) % 4;
		h = _scrollHeight[s];
		if (h > 156 - y) h = 156 - y;
		if (h > 0) {
			Graphics::ManagedSurface* ms = _scroll[s]->getRawSurface();
			Common::Rect srcRect(0, 0, ms->w, h);
			surf->Blit(*ms, srcRect, 32, 44 + y);
		}
		y += h;
	}
}

bool CreditsGump::OnKeyDown(int key, int mod) {
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

} // End of namespace Ultima8
} // End of namespace Ultima
