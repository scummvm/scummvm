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
void TextManager::init() {
	// First, delete everything that exists
	quit();

	// Load the list of fonts
	XMLDoc fontList(g_engine->_filePath->_font);
	if (fontList.ready()) {
		rapidxml::xml_node<char> *node = fontList.doc()->first_node("fonts");

		loadNum(_cacheSize, "cache_size", node);
		_cache.resize(_cacheSize);
		_oldest = 0;

		if (nodeValid(node->first_node("padding")))
			_padBg.load(node->first_node("padding"));

		for (auto n = node->first_node("font"); n != NULL; n = n->next_sibling("font")) {
			rapidxml::xml_attribute<char> *id, *path, *size;
			id = n->first_attribute("id");
			path = n->first_attribute("path");
			size = n->first_attribute("size");

			if (id != NULL && path != NULL && size != NULL) {
				unsigned int pos = stringToNumber<unsigned int>(id->value());
				if (_font.size() <= pos)
					_font.resize(pos + 1);
#if 0
				font[pos] = TTF_OpenFont(path->value(), StringToNumber<int>(size->value()));
				TTF_SetFontHinting(font[pos], TTF_HINTING_LIGHT);
#endif
				Common::File file;
				fileOpen(path->value(), &file);
				_font[pos] = Graphics::loadTTFFont(file, stringToNumber<int>(size->value()));
			}
		}
	}

	_colpool.load(g_engine->_filePath->_colors);
}

void TextManager::reset() {
	_cache.clear();
	_cache.resize(_cacheSize);
}

//------------------------------------------------------------------------
// Purpose: Search cache for rendered text
//------------------------------------------------------------------------
int TextManager::search(const Common::String &text, int col, FontKey fontid) {
	int pos = 0;
	for (auto i = _cache.begin(); i != _cache.end(); ++i, ++pos)
		if (i->_empty == false && i->_text == text && i->EqualCol(col) && i->_font == fontid)
			return pos;

	return -1;
}

int TextManager::findFreeSlot() {
	int pos = 0;
	for (auto i = _cache.begin(); i != _cache.end(); ++i, ++pos)
		if (i->_empty)
			return pos;

	int ret = _oldest;
	_oldest = (_oldest + 1) % _cache.size();
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

Graphics::ManagedSurface *TextManager::renderTextBlended(const FontKey &fKey, const Common::String &text, const int &color) {
	SDL_Color sdlcolor = _colpool.get(color);
	uint32 col = g_engine->_format->ARGBToColor(255, sdlcolor.r, sdlcolor.g, sdlcolor.b);

	Graphics::ManagedSurface *surf = nullptr;

	if (text.empty()) {
		Common::Rect rec = getFont(fKey)->getBoundingBox(" ");
		int h = rec.height();
		surf = new Graphics::ManagedSurface(rec.width(), h + (h / 2), *g_engine->_format);
		getFont(fKey)->drawString(surf, " ", 0, 0, rec.width(), col);
	} else {
		Common::Rect rec = getFont(fKey)->getBoundingBox(text);
		int h = rec.height();
		surf = new Graphics::ManagedSurface(rec.width(), h + (h / 2), *g_engine->_format);
		getFont(fKey)->drawString(surf, text, 0, 0, rec.width(), col);
	}

	return surf;
}

//------------------------------------------------------------------------
// Purpose: Draw text
//------------------------------------------------------------------------
void TextManager::draw(const int &x, const int &y, const Common::String &text, const int &color,
					   const FontKey &fontk, const Align &align, const bool &background) {
	//warning("STUB: TextManager::draw()");

	if (text == " ") return;

	int pos = search(text, color, fontk);
	if (pos == -1) {
		pos = findFreeSlot();
#if 0
		SDL_Surface *surf = RenderTextBlended(font, text, color);
#endif
		Graphics::ManagedSurface *surf = renderTextBlended(fontk, text, color);
		_cache[pos]._img.deleteImage();
		_cache[pos]._empty = false;

		_cache[pos]._text = text;
		_cache[pos]._col = color;
		_cache[pos]._font = fontk;

		_cache[pos]._img.load(surf);

		delete surf;
#if 0
		SDL_FreeSurface(surf);
#endif
	}

	if (background) {
		_rect.w = _cache[pos]._img.w() + (2 * _padBg.x);
		_rect.h = _cache[pos]._img.h() + (2 * _padBg.y);

		uint32 col = g_engine->_format->ARGBToColor(128, 0, 0, 0);
		Graphics::Surface surf;
		surf.create(_rect.w, _rect.h, *g_engine->_format);
		surf.fillRect(Common::Rect(_rect.w, _rect.h), col);

#if 0
		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 128);
#endif

		if (align == ALIGN_LEFT) {
			_rect.x = x - _padBg.x;
			_rect.y = y - _padBg.y;
			g_engine->_screen->blitFrom(surf, Common::Point(_rect.x, _rect.y));

#if 0
			SDL_RenderFillRect(gRenderer, &rect);
#endif
			_cache[pos]._img.draw(x, y);
		} else if (align == ALIGN_CENTER) {
			_rect.x = x - _cache[pos]._img.w() / 2 - _padBg.x;
			_rect.y = y - _cache[pos]._img.h() / 2 - _padBg.y;
			g_engine->_screen->blitFrom(surf, Common::Point(_rect.x, _rect.y));

#if 0
			SDL_RenderFillRect(gRenderer, &rect);
#endif
			_cache[pos]._img.draw(x - _cache[pos]._img.w() / 2, y - _cache[pos]._img.h() / 2);
		} else {
			_rect.x = x - _cache[pos]._img.w() - _padBg.x;
			_rect.y = y - _padBg.y;
			g_engine->_screen->blitFrom(surf, Common::Point(_rect.x, _rect.y));

#if 0
			SDL_RenderFillRect(gRenderer, &rect);
#endif
			_cache[pos]._img.draw(x - _cache[pos]._img.w(), y);
		}

		surf.free();

	} else {
		if (align == ALIGN_LEFT)
			_cache[pos]._img.draw(x, y);
		else if (align == ALIGN_CENTER)
			_cache[pos]._img.draw(x - _cache[pos]._img.w() / 2, y - _cache[pos]._img.h() / 2);
		else
			_cache[pos]._img.draw(x - _cache[pos]._img.w(), y);
	}
}

void TextManager::draw(const int &x, int y, const Common::String &text, const int &color, const FontKey &fKey, const Align &align,
					   const unsigned int &lineWidth, const unsigned int &lineHeight, const bool &background) {
	for (unsigned int startPos = 0, len = text.size(); startPos < len; y += lineHeight) {
		unsigned int endPos = startPos + 1;
		int lastInterrupt = -1;
		Common::String word;

		while (endPos - startPos <= lineWidth) {
			if (endPos == len || text[endPos] == '`') {
				lastInterrupt = endPos;
				break;
			}

			if (text[endPos] == ' ' || text[endPos] == ',' || text[endPos] == '.')
				lastInterrupt = endPos;

			endPos++;
		}

		if (lastInterrupt >= 0) // wrap a word around
		{
			for (unsigned int i = 0; i < lastInterrupt - startPos; i++)
				word += text[startPos + i];

			startPos = lastInterrupt + 1;
		} else // word bigger than line, just thunk
		{
			for (unsigned int i = 0; i < endPos - startPos; i++)
				word += text[startPos + i];

			startPos += lineWidth;
		}

		draw(x, y, word, color, fKey, align, background);
	}
}

//------------------------------------------------------------------------
// Purpose: Quit
//------------------------------------------------------------------------
void TextManager::quit() {
	for (auto i = _font.begin(); i != _font.end(); ++i)
		delete *i;

	for (auto i = _cache.begin(); i != _cache.end(); ++i) {
		if (i->_empty == false) {
			i->_img.deleteImage();
			i->_empty = true;
		}
	}
}

} // End of namespace Crab
