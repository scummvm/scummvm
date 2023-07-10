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
#include "crab/text/TextManager.h"
#include "crab/XMLDoc.h"

namespace Crab {

using namespace pyrodactyl::text;

//------------------------------------------------------------------------
// Purpose: Initialize, set cache etc
//------------------------------------------------------------------------
void TextManager::Init() {
	// First, delete everything that exists
	Quit();

	// Load the list of fonts
	XMLDoc font_list(g_engine->_filePath->font);
	if (font_list.ready()) {
		rapidxml::xml_node<char> *node = font_list.doc()->first_node("fonts");

		loadNum(cache_size, "cache_size", node);
		cache.resize(cache_size);
		oldest = 0;

		if (nodeValid(node->first_node("padding")))
			pad_bg.Load(node->first_node("padding"));

		for (auto n = node->first_node("font"); n != NULL; n = n->next_sibling("font")) {
			rapidxml::xml_attribute<char> *id, *path, *size;
			id = n->first_attribute("id");
			path = n->first_attribute("path");
			size = n->first_attribute("size");

			if (id != NULL && path != NULL && size != NULL) {
				unsigned int pos = StringToNumber<unsigned int>(id->value());
				if (font.size() <= pos)
					font.resize(pos + 1);
#if 0
				font[pos] = TTF_OpenFont(path->value(), StringToNumber<int>(size->value()));
				TTF_SetFontHinting(font[pos], TTF_HINTING_LIGHT);
#endif
				Common::File file;
				FileOpen(path->value(), &file);
				font[pos] = Graphics::loadTTFFont(file, StringToNumber<int>(size->value()));
			}
		}
	}

	colpool.Load(g_engine->_filePath->colors);
}

void TextManager::Reset() {
	cache.clear();
	cache.resize(cache_size);
}

//------------------------------------------------------------------------
// Purpose: Search cache for rendered text
//------------------------------------------------------------------------
int TextManager::Search(const Common::String &text, int col, FontKey fontid) {
	int pos = 0;
	for (auto i = cache.begin(); i != cache.end(); ++i, ++pos)
		if (i->empty == false && i->text == text && i->EqualCol(col) && i->font == fontid)
			return pos;

	return -1;
}

int TextManager::FindFreeSlot() {
	int pos = 0;
	for (auto i = cache.begin(); i != cache.end(); ++i, ++pos)
		if (i->empty)
			return pos;

	int ret = oldest;
	oldest = (oldest + 1) % cache.size();
	return ret;
}

//------------------------------------------------------------------------
// Purpose: Render the SDL surface for text
//------------------------------------------------------------------------
#if 0
SDL_Surface *TextManager::RenderTextBlended(const FontKey &font, const Common::String &text, const int &color) {
	if (text.empty())
		return TTF_RenderText_Blended(GetFont(font), " ", colpool.Get(color));

	return TTF_RenderText_Blended(GetFont(font), text.c_str(), colpool.Get(color));
}
#endif

Graphics::ManagedSurface *TextManager::RenderTextBlended(const FontKey &fKey, const Common::String &text, const int &color) {
	SDL_Color sdlcolor = colpool.Get(color);
	uint32 col = g_engine->_format->ARGBToColor(255, sdlcolor.r, sdlcolor.g, sdlcolor.b);

	Graphics::ManagedSurface *surf = nullptr;

	if (text.empty()) {
		Common::Rect rec = GetFont(fKey)->getBoundingBox(" ");
		int h = rec.height();
		surf = new Graphics::ManagedSurface(rec.width(), h + (h / 2), *g_engine->_format);
		GetFont(fKey)->drawString(surf, " ", 0, 0, rec.width(), col);
	} else {
		Common::Rect rec = GetFont(fKey)->getBoundingBox(text);
		int h = rec.height();
		surf = new Graphics::ManagedSurface(rec.width(), h + (h / 2), *g_engine->_format);
		GetFont(fKey)->drawString(surf, text, 0, 0, rec.width(), col);
	}

	return surf;
}

//------------------------------------------------------------------------
// Purpose: Draw text
//------------------------------------------------------------------------
void TextManager::Draw(const int &x, const int &y, const Common::String &text, const int &color,
					   const FontKey &fontk, const Align &align, const bool &background) {
	//warning("STUB: TextManager::Draw()");

	if (text == " ") return;

	int pos = Search(text, color, fontk);
	if (pos == -1) {
		pos = FindFreeSlot();
#if 0
		SDL_Surface *surf = RenderTextBlended(font, text, color);
#endif
		Graphics::ManagedSurface *surf = RenderTextBlended(fontk, text, color);
		cache[pos].img.Delete();
		cache[pos].empty = false;

		cache[pos].text = text;
		cache[pos].col = color;
		cache[pos].font = fontk;

		cache[pos].img.Load(surf);

		delete surf;
#if 0
		SDL_FreeSurface(surf);
#endif
	}

	if (background) {
		rect.w = cache[pos].img.W() + (2 * pad_bg.x);
		rect.h = cache[pos].img.H() + (2 * pad_bg.y);

		uint32 col = g_engine->_format->ARGBToColor(128, 0, 0, 0);
		Graphics::Surface surf;
		surf.create(rect.w, rect.h, *g_engine->_format);
		surf.fillRect(Common::Rect(rect.w, rect.h), col);

#if 0
		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 128);
#endif

		if (align == ALIGN_LEFT) {
			rect.x = x - pad_bg.x;
			rect.y = y - pad_bg.y;
			g_engine->_screen->blitFrom(surf, Common::Point(rect.x, rect.y));

#if 0
			SDL_RenderFillRect(gRenderer, &rect);
#endif
			cache[pos].img.Draw(x, y);
		} else if (align == ALIGN_CENTER) {
			rect.x = x - cache[pos].img.W() / 2 - pad_bg.x;
			rect.y = y - cache[pos].img.H() / 2 - pad_bg.y;
			g_engine->_screen->blitFrom(surf, Common::Point(rect.x, rect.y));

#if 0
			SDL_RenderFillRect(gRenderer, &rect);
#endif
			cache[pos].img.Draw(x - cache[pos].img.W() / 2, y - cache[pos].img.H() / 2);
		} else {
			rect.x = x - cache[pos].img.W() - pad_bg.x;
			rect.y = y - pad_bg.y;
			g_engine->_screen->blitFrom(surf, Common::Point(rect.x, rect.y));

#if 0
			SDL_RenderFillRect(gRenderer, &rect);
#endif
			cache[pos].img.Draw(x - cache[pos].img.W(), y);
		}

		surf.free();

	} else {
		if (align == ALIGN_LEFT)
			cache[pos].img.Draw(x, y);
		else if (align == ALIGN_CENTER)
			cache[pos].img.Draw(x - cache[pos].img.W() / 2, y - cache[pos].img.H() / 2);
		else
			cache[pos].img.Draw(x - cache[pos].img.W(), y);
	}
}

void TextManager::Draw(const int &x, int y, const Common::String &text, const int &color, const FontKey &fKey, const Align &align,
					   const unsigned int &line_width, const unsigned int &line_height, const bool &background) {
	for (unsigned int start_pos = 0, len = text.size(); start_pos < len; y += line_height) {
		unsigned int end_pos = start_pos + 1;
		int last_interrupt = -1;
		Common::String word;

		while (end_pos - start_pos <= line_width) {
			if (end_pos == len || text[end_pos] == '`') {
				last_interrupt = end_pos;
				break;
			}

			if (text[end_pos] == ' ' || text[end_pos] == ',' || text[end_pos] == '.')
				last_interrupt = end_pos;

			end_pos++;
		}

		if (last_interrupt >= 0) // wrap a word around
		{
			for (unsigned int i = 0; i < last_interrupt - start_pos; i++)
				word += text[start_pos + i];

			start_pos = last_interrupt + 1;
		} else // word bigger than line, just thunk
		{
			for (unsigned int i = 0; i < end_pos - start_pos; i++)
				word += text[start_pos + i];

			start_pos += line_width;
		}

		Draw(x, y, word, color, fKey, align, background);
	}
}

//------------------------------------------------------------------------
// Purpose: Quit
//------------------------------------------------------------------------
void TextManager::Quit() {
	for (auto i = font.begin(); i != font.end(); ++i)
		delete *i;

	for (auto i = cache.begin(); i != cache.end(); ++i) {
		if (i->empty == false) {
			i->img.Delete();
			i->empty = true;
		}
	}
}

} // End of namespace Crab
