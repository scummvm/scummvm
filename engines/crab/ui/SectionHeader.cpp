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

#include "crab/crab.h"
#include "crab/ui/SectionHeader.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;

void SectionHeader::load(rapidxml::xml_node<char> *node) {

	if (TextData::load(node, NULL, false)) {
		loadStr(text, "text", node);
		text.insertChar(' ', 0);
		text += " ";

		loadImgKey(img, "img", node);

		loadBool(draw_l, "left", node);
		loadBool(draw_r, "right", node);

		Graphics::ManagedSurface *surf = g_engine->_textManager->RenderTextBlended(font, text, col);

		if (align == ALIGN_CENTER) {
			left.x = x - surf->w / 2 - g_engine->_imageManager->GetTexture(img).W();
			left.y = y - surf->h / 2 + g_engine->_imageManager->GetTexture(img).H() / 2;

			right.x = x + surf->w / 2;
			right.y = y - surf->h / 2 + g_engine->_imageManager->GetTexture(img).H() / 2;
		} else if (align == ALIGN_LEFT) {
			left.x = x - g_engine->_imageManager->GetTexture(img).W();
			left.y = y + surf->h / 2 - g_engine->_imageManager->GetTexture(img).H() / 2;

			right.x = x + surf->w;
			right.y = y + surf->h / 2 - g_engine->_imageManager->GetTexture(img).H() / 2;
		} else {
			left.x = x - surf->w - g_engine->_imageManager->GetTexture(img).W();
			left.y = y + surf->h / 2 - g_engine->_imageManager->GetTexture(img).H() / 2;

			right.x = x;
			right.y = y + surf->h / 2 - g_engine->_imageManager->GetTexture(img).H() / 2;
		}

		delete surf;
	}

}

void SectionHeader::Draw(const int &XOffset, const int &YOffset) {
	Draw(text, XOffset, YOffset);
}

void SectionHeader::Draw(const Common::String &str, const int &XOffset, const int &YOffset) {
	if (draw_l)
		g_engine->_imageManager->Draw(left.x + XOffset, left.y + YOffset, img);

	if (draw_r)
		g_engine->_imageManager->Draw(right.x + XOffset, right.y + YOffset, img, (Rect*)NULL, FLIP_X);

	TextData::Draw(str, XOffset, YOffset);
}

} // End of namespace Crab
