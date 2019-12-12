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
#include "ultima/ultima8/gumps/credits_gump.h"

#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/conf/setting_manager.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CreditsGump, ModalGump)

CreditsGump::CreditsGump()
	: ModalGump() {

}

CreditsGump::CreditsGump(const std::string &text_, int parskip_,
                         uint32 _Flags, int32 layer_)
	: ModalGump(0, 0, 320, 200, 0, _Flags, layer_) {
	text = text_;
	parskip = parskip_;

	timer = 0;
	title = 0;
	nexttitle = 0;
	state = CS_PLAYING;
}

CreditsGump::~CreditsGump() {
	delete scroll[0];
	delete scroll[1];
	delete scroll[2];
	delete scroll[3];

	delete title;
	delete nexttitle;
}

void CreditsGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	scroll[0] = RenderSurface::CreateSecondaryRenderSurface(256, 200);
	scroll[1] = RenderSurface::CreateSecondaryRenderSurface(256, 200);
	scroll[2] = RenderSurface::CreateSecondaryRenderSurface(256, 200);
	scroll[3] = RenderSurface::CreateSecondaryRenderSurface(256, 200);
	scroll[0]->Fill32(0xFF000000, 0, 0, 256, 200); // black background
	scroll[1]->Fill32(0xFF000000, 0, 0, 256, 200);
	scroll[2]->Fill32(0xFF000000, 0, 0, 256, 200);
	scroll[3]->Fill32(0xFF000000, 0, 0, 256, 200);
	scrollheight[0] = 156;
	scrollheight[1] = 0;
	scrollheight[2] = 0;
	scrollheight[3] = 0;

	currentsurface = 0;
	currenty = 0;

	Mouse::get_instance()->pushMouseCursor();
	Mouse::get_instance()->setMouseCursor(Mouse::MOUSE_NONE);
}

void CreditsGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();

	ModalGump::Close(no_del);

	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(0);
}

void CreditsGump::extractLine(std::string &text_,
                              char &modifier, std::string &line) {
	if (text_.empty()) {
		line = "";
		modifier = 0;
		return;
	}

	if (text_[0] == '+' || text_[0] == '&' || text_[0] == '}' || text_[0] == '~' ||
	        text_[0] == '@') {
		modifier = text_[0];
		text_.erase(0, 1);
	} else {
		modifier = 0;
	}

	std::string::size_type starpos = text_.find('*');

	line = text_.substr(0, starpos);

	// replace '%%' by '%'.
	// (Original interpreted these strings as format strings??)
	std::string::size_type ppos;
	while ((ppos = line.find("%%")) != std::string::npos) {
		line.replace(ppos, 2, "%");
	}

	if (starpos != std::string::npos) starpos++;
	text_.erase(0, starpos);
}


void CreditsGump::run() {
	ModalGump::run();

	if (timer) {
		timer--;
		return;
	}

	if (state == CS_CLOSING) {
		// pout << "CreditsGump: closing" << std::endl;
		Close();
		return;
	}

	timer = 1;

	int available = scrollheight[currentsurface] - currenty;
	int nextblock = -1;
	for (int i = 1; i < 4; i++) {
		available += scrollheight[(currentsurface + i) % 4];
		if (nextblock == -1 && scrollheight[(currentsurface + i) % 4] == 0)
			nextblock = (currentsurface + i) % 4;
	}
	if (available == 0) nextblock = 0;

	if (state == CS_FINISHING && available <= 156) {
		// pout << "CreditsGump: waiting before closing" << std::endl;
		timer = 120;
		state = CS_CLOSING;

		if (!configkey.empty()) {
			SettingManager *settingman = SettingManager::get_instance();
			settingman->set(configkey, true);
			settingman->write();
		}

		return;
	}

	if (state == CS_PLAYING && available <= 160) {
		// time to render next block

		scroll[nextblock]->Fill32(0xFF000000, 0, 0, 256, 200);
		// scroll[nextblock]->Fill32(0xFFFFFFFF,0,0,256,5); // block marker
		scrollheight[nextblock] = 0;

		Pentagram::Font *redfont, *yellowfont;

		redfont = FontManager::get_instance()->getGameFont(6, true);
		yellowfont = FontManager::get_instance()->getGameFont(8, true);

		bool done = false;
		bool firstline = true;
		while (!text.empty() && !done) {
			std::string::size_type endline = text.find('\n');
			std::string line = text.substr(0, endline);

			if (line.empty()) {
				text.erase(0, 1);
				continue;
			}

			// pout << "Rendering paragraph: " << line << std::endl;

			if (line[0] == '+') {
				// set title
				if (!firstline) {
					// if this isn't the first line of the block,
					// postpone setting title until next block
					done = true;
					continue;
				}

				std::string titletext;
				char modifier;

				extractLine(line, modifier, titletext);

				unsigned int remaining;
				nexttitle = redfont->renderText(titletext, remaining, 192, 0,
				                                Pentagram::Font::TEXT_CENTER);

				if (!title) {
					title = nexttitle;
					nexttitle = 0;
				} else {
					nexttitlesurf = nextblock;
					scrollheight[nextblock] = 160; // skip some space
				}

			} else {

				int height = 0;

				Pentagram::Font *font = redfont;
				Pentagram::Font::TextAlign align = Pentagram::Font::TEXT_LEFT;
				int indent = 0;

				while (!line.empty()) {
					std::string outline;
					char modifier;
					unsigned int remaining;
					extractLine(line, modifier, outline);

					// pout << "Rendering line: " << outline << std::endl;

					switch (modifier) {
					case '&':
						font = yellowfont;
						align = Pentagram::Font::TEXT_CENTER;
						break;
					case '}':
						font = redfont;
						align = Pentagram::Font::TEXT_CENTER;
						break;
					case '~':
						font = yellowfont;
						align = Pentagram::Font::TEXT_LEFT;
						indent = 32;
						break;
					case '@':
						// pout << "CreditsGump: done, finishing" << std::endl;
						state = CS_FINISHING;
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

						if (scrollheight[nextblock] + height + 7 > 200) {
							done = true;
							break;
						}

						int linewidth = outline.size() * 8;
						if (linewidth > 192) linewidth = 192;

						scroll[nextblock]->
						Fill32(0xFFD43030, 128 - (linewidth / 2),
						       scrollheight[nextblock] + height + 3,
						       linewidth, 1);
						height += 7;
						continue;
					}

					RenderedText *rt = font->renderText(outline, remaining,
					                                    256 - indent, 0,
					                                    align);
					int xd, yd;
					rt->getSize(xd, yd);

					if (scrollheight[nextblock] + height + yd > 200) {
						delete rt;
						done = true;
						break;
					}

					rt->draw(scroll[nextblock], indent,
					         scrollheight[nextblock] + height +
					         font->getBaseline());

					height += yd + rt->getVlead();

					delete rt;
				}

				if (state == CS_PLAYING)
					height += parskip;

				if (scrollheight[nextblock] + height > 200) {
					if (firstline) {
						height = 200 - scrollheight[nextblock];
						assert(height >= 0);
					} else {
						done = true;
					}
				}

				if (done) break; // no room

				scrollheight[nextblock] += height;
			}


			if (endline != std::string::npos) endline++;
			text.erase(0, endline);

			firstline = false;
		}
	}

	currenty++;

	if (currenty >= scrollheight[currentsurface]) {
		// next surface
		currenty -= scrollheight[currentsurface];
		scrollheight[currentsurface] = 0;
		currentsurface = (currentsurface + 1) % 4;

		if (nexttitle && currentsurface == nexttitlesurf) {
			delete title;
			title = nexttitle;
			nexttitle = 0;
		}
	}
}

void CreditsGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	surf->Fill32(0xFF000000, 0, 0, 320, 200); // black background
	surf->Fill32(0xFFD43030, 64, 41, 192, 1); // line between title and scroller

	if (title)
		title->draw(surf, 64, 34);

	Texture *tex = scroll[currentsurface]->GetSurfaceAsTexture();
	int h = scrollheight[currentsurface] - currenty;
	if (h > 156) h = 156;
	if (h > 0)
		surf->Blit(tex, 0, currenty, 256, h, 32, 44);

	int y_ = h;
	for (int i = 1; i < 4; i++) {
		if (h == 156) break;

		int s = (currentsurface + i) % 4;
		tex = scroll[s]->GetSurfaceAsTexture();
		h = scrollheight[s];
		if (h > 156 - y_) h = 156 - y_;
		if (h > 0)
			surf->Blit(tex, 0, 0, 256, h, 32, 44 + y_);
		y_ += h;
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
