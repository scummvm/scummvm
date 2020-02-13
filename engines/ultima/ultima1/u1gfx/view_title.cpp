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

#include "ultima/ultima1/u1gfx/view_title.h"
#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/file.h"
#include "ultima/shared/gfx/text_cursor.h"
#include "ultima/shared/early/font_resources.h"
#include "ultima/shared/early/ultima_early.h"
#include "image/bmp.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(ViewTitle, Shared::Gfx::VisualItem)
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(KeypressMsg)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

void load16(Graphics::ManagedSurface &s, Common::ReadStream &in) {
	byte *destP = (byte *)s.getPixels();
	byte v;
	for (int idx = 0; idx < (s.w * s.h); idx += 2) {
		v = in.readByte();
		*destP++ = v & 0xf;
		*destP++ = v >> 4;
	}
}

ViewTitle::ViewTitle(Shared::TreeItem *parent) : Shared::Gfx::VisualItem("Title", Rect(0, 0, 320, 200), parent) {
	setMode(TITLEMODE_COPYRIGHT);

	// Load the Origin logo
	Shared::File f("data/logo.bmp");
	Image::BitmapDecoder bmp;
	if (!bmp.loadStream(f))
		error("Couldn't load logo");
	f.close();

	const Graphics::Surface *src = bmp.getSurface();
	_logo.create(src->w, src->h);
	_logo.blitFrom(*src);

	// Load the Ultima castle bitmap
	f.open("castle.16");
	_castle.create(320, 200);
	load16(_castle, f);
	f.close();

	// Load the flags
	f.open("data/flags.bmp");
	Image::BitmapDecoder flags;
	if (!flags.loadStream(f))
		error("Could not load flags");

	src = flags.getSurface();
	for (int idx = 0; idx < 3; ++idx) {
		_flags[idx].create(8, 8);
		_flags[idx].blitFrom(*src,
			Common::Rect(idx * 8, 0, (idx + 1) * 8, 8),
			Common::Point(0, 0));
	}
	f.close();
}

void ViewTitle::draw() {
	VisualItem::draw();

	switch (_mode) {
	case TITLEMODE_COPYRIGHT:
		drawCopyrightView();
		break;

	case TITLEMODE_PRESENTS:
		drawPresentsView();
		break;

	case TITLEMODE_CASTLE:
		drawCastleView();
		break;

	case TITLEMODE_TRADEMARKS:
		drawTrademarksView();
		break;

	case TITLEMODE_MAIN_MENU:
		drawMainMenu();
		break;

	default:
		break;
	}
}

void ViewTitle::drawCopyrightView() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	Shared::Gfx::VisualSurface s = getSurface();
	s.clear();

	// Draw horizontal title lines
	for (int idx = 0; idx < 3; ++idx) {
		s.hLine(112, idx + 58, 200, 1);
		s.hLine(112, idx + 74, 200, 1);
	}

	// Write text
	s.writeString(game->_res->TITLE_MESSAGES[0], TextPoint(16, 8), game->_whiteColor);
	s.writeString(game->_res->TITLE_MESSAGES[1], TextPoint(8, 11), game->_whiteColor);
	s.writeString(game->_res->TITLE_MESSAGES[2], TextPoint(0, 21), game->_whiteColor);
}

void ViewTitle::drawPresentsView() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	Shared::Gfx::VisualSurface s = getSurface();

	switch (_counter) {
	case 0:
		s.clear();
		s.blitFrom(_logo, Point(20, 21));
		s.writeString(game->_res->TITLE_MESSAGES[3], TextPoint(14, 13));
		break;
	case 1:
		s.writeString(game->_res->TITLE_MESSAGES[4], TextPoint(5, 12));
		s.writeString(game->_res->TITLE_MESSAGES[5], TextPoint(5, 13));
		s.writeString(game->_res->TITLE_MESSAGES[6], TextPoint(5, 14));
		break;
	case 2:
		s.fillRect(Rect(0, 12 * 8, 320, 15 * 8), game->_bgColor);
		s.writeString(game->_res->TITLE_MESSAGES[7], TextPoint(6, 12));
		s.writeString(game->_res->TITLE_MESSAGES[8], TextPoint(6, 13));
		break;
	default:
		break;
	}
}

void ViewTitle::drawCastleView() {
	Shared::Gfx::VisualSurface s = getSurface();
	if (_counter == 0)
		s.blitFrom(_castle);

	drawCastleFlag(s, 123);
	drawCastleFlag(s, 196);
}

void ViewTitle::drawCastleFlag(Shared::Gfx::VisualSurface &s, int xp) {
	s.blitFrom(_flags[getGame()->getRandomNumber(0, 2)], Common::Point(xp, 55));
}

void ViewTitle::drawTrademarksView() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	Shared::Gfx::VisualSurface s = getSurface();
	if (_counter == 0)
		s.clear();

	if (_counter < 32) {
		s.blitFrom(_logo, Common::Rect(0, 0, _logo.w, _counter + 1), Point(20, 21));
		s.blitFrom(_logo, Common::Rect(0, _logo.h - _counter - 1, _logo.w, _logo.h),
			Common::Point(20, 21 + _logo.h - _counter - 1));
	} else {
		s.writeString(game->_res->TITLE_MESSAGES[9], TextPoint(1, 17));
		s.writeString(game->_res->TITLE_MESSAGES[10], TextPoint(2, 18));
		s.writeString(game->_res->TITLE_MESSAGES[11], TextPoint(11, 19));
		s.writeString(game->_res->TITLE_MESSAGES[12], TextPoint(6, 23));
	}	
}

void ViewTitle::drawMainMenu() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	Shared::Gfx::VisualSurface s = getSurface();
	DrawingSupport ds(s);
	s.clear();
	ds.drawFrame();

	s.writeString(game->_res->MAIN_MENU_TEXT[0], TextPoint(12, 6));
	s.writeString(game->_res->MAIN_MENU_TEXT[1], TextPoint(14, 9));
	s.writeString(game->_res->MAIN_MENU_TEXT[2], TextPoint(14, 10));
	s.writeString(game->_res->MAIN_MENU_TEXT[3], TextPoint(13, 11));
	s.writeString(game->_res->MAIN_MENU_TEXT[4], TextPoint(8, 14));
	s.writeString(game->_res->MAIN_MENU_TEXT[5], TextPoint(8, 15));
	s.writeString(game->_res->MAIN_MENU_TEXT[6], TextPoint(13, 18));
}

void ViewTitle::setTitlePalette() {
	const byte PALETTE[] = { 0, 1, 2, 3, 4, 5, 6, 7, 56, 57, 58, 59, 60, 61, 62, 63 };
	getGame()->setEGAPalette(PALETTE);
}

void ViewTitle::setCastlePalette() {
	const byte PALETTE[] = { 0, 24, 7, 63, 63, 34, 58, 14, 20, 7, 61, 59, 1, 57, 7, 63 };
	getGame()->setEGAPalette(PALETTE);
}

bool ViewTitle::FrameMsg(CFrameMsg &msg) {
	uint32 time = getGame()->getMillis();
	if (time < _expiryTime)
		return true;
	setDirty();
	
	switch (_mode) {
	case TITLEMODE_COPYRIGHT:
		setMode(TITLEMODE_PRESENTS);
		break;
	case TITLEMODE_PRESENTS:
		_expiryTime = time + 3000;
		if (++_counter == 3)
			setMode(TITLEMODE_CASTLE);
		break;

	case TITLEMODE_CASTLE:
		_expiryTime = time + 200;
		if (++_counter == 100)
			setMode(TITLEMODE_PRESENTS);
		break;

	case TITLEMODE_TRADEMARKS:
		_expiryTime = time + 20;
		++_counter;
		if (_counter == 32) {
			_expiryTime = time + 4000;
		} else if (_counter == 33) {
			setMode(TITLEMODE_MAIN_MENU);
		}
		break;

	default:
		break;
	}

	return true;
}

void ViewTitle::setMode(TitleMode mode) {
	_expiryTime = getGame()->getMillis();
	_counter = 0;
	_mode = mode;
	setDirty();
	setTitlePalette();

	switch (mode) {
	case TITLEMODE_COPYRIGHT:
		_expiryTime += 4000;
		break;
	case TITLEMODE_PRESENTS:
		_expiryTime += 3000;
		break;
	case TITLEMODE_CASTLE:
		setCastlePalette();
		break;
	case TITLEMODE_MAIN_MENU: {
		Shared::Gfx::TextCursor *textCursor = getGame()->_textCursor;
		textCursor->setPosition(TextPoint(25, 18));
		textCursor->setVisible(true);
		break;
	}
	default:
		break;
	}
}

bool ViewTitle::ShowMsg(CShowMsg &msg) {
	Shared::Gfx::VisualItem::ShowMsg(msg);

	if (_mode == TITLEMODE_MAIN_MENU) {
		// Returning to main menu from another screen
		setMode(TITLEMODE_MAIN_MENU);
	}

	return true;
}

bool ViewTitle::KeypressMsg(CKeypressMsg &msg) {
	uint32 time = getGame()->getMillis();

	if (_mode == TITLEMODE_MAIN_MENU) {
		if (msg._keyState.keycode == Common::KEYCODE_a || msg._keyState.keycode == Common::KEYCODE_b) {
			// Hide the cursor
			Shared::Gfx::TextCursor *textCursor = getGame()->_textCursor;
			textCursor->setVisible(false);

			if (msg._keyState.keycode == Common::KEYCODE_a) {
				setView("CharGen");
			} else {
				if (!g_vm->loadGameDialog())
					textCursor->setVisible(true);
			}
		}

	} else if (_mode != TITLEMODE_TRADEMARKS) {
		// Switch to the trademarks view
		_mode = TITLEMODE_TRADEMARKS;
		_expiryTime = time;
		_counter = -1;
	}

	return true;
}

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima
