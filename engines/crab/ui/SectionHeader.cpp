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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "graphics/managed_surface.h"
#include "crab/crab.h"
#include "crab/ui/SectionHeader.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;

void SectionHeader::load(rapidxml::xml_node<char> *node) {

	if (TextData::load(node, NULL, false)) {
		loadStr(_text, "text", node);
		_text.insertChar(' ', 0);
		_text += " ";

		loadImgKey(_img, "img", node);

		loadBool(_drawL, "left", node);
		loadBool(_drawR, "right", node);

		Graphics::ManagedSurface *surf = g_engine->_textManager->renderTextBlended(_font, _text, _col);

		if (_align == ALIGN_CENTER) {
			_left.x = x - surf->w / 2 - g_engine->_imageManager->getTexture(_img).w();
			_left.y = y - surf->h / 2 + g_engine->_imageManager->getTexture(_img).h() / 2;

			_right.x = x + surf->w / 2;
			_right.y = y - surf->h / 2 + g_engine->_imageManager->getTexture(_img).h() / 2;
		} else if (_align == ALIGN_LEFT) {
			_left.x = x - g_engine->_imageManager->getTexture(_img).w();
			_left.y = y + surf->h / 2 - g_engine->_imageManager->getTexture(_img).h() / 2;

			_right.x = x + surf->w;
			_right.y = y + surf->h / 2 - g_engine->_imageManager->getTexture(_img).h() / 2;
		} else {
			_left.x = x - surf->w - g_engine->_imageManager->getTexture(_img).w();
			_left.y = y + surf->h / 2 - g_engine->_imageManager->getTexture(_img).h() / 2;

			_right.x = x;
			_right.y = y + surf->h / 2 - g_engine->_imageManager->getTexture(_img).h() / 2;
		}

		delete surf;
	}

}

void SectionHeader::draw(const int &xOffset, const int &yOffset) {
	draw(_text, xOffset, yOffset);
}

void SectionHeader::draw(const Common::String &str, const int &xOffset, const int &yOffset) {
	if (_drawL)
		g_engine->_imageManager->draw(_left.x + xOffset, _left.y + yOffset, _img);

	if (_drawR)
		g_engine->_imageManager->draw(_right.x + xOffset, _right.y + yOffset, _img, (Rect*)NULL, FLIP_X);

	TextData::draw(str, xOffset, yOffset);
}

} // End of namespace Crab
