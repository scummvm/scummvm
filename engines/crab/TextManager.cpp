#include "stdafx.h"
#include "XMLDoc.h"
#include "TextManager.h"

using namespace pyrodactyl::text;

namespace pyrodactyl
{
	namespace text
	{
		TextManager gTextManager;
	}
}

//------------------------------------------------------------------------
// Purpose: Initialize, set cache etc
//------------------------------------------------------------------------
void TextManager::Init()
{
	//First, delete everything that exists
	Quit();

	//Load the list of fonts
	XMLDoc font_list(gFilePath.font);
	if (font_list.ready())
	{
		rapidxml::xml_node<char> *node = font_list.Doc()->first_node("fonts");

		LoadNum(cache_size, "cache_size", node);
		cache.resize(cache_size);
		oldest = 0;

		if (NodeValid(node->first_node("padding")))
			pad_bg.Load(node->first_node("padding"));

		for (auto n = node->first_node("font"); n != NULL; n = n->next_sibling("font"))
		{
			rapidxml::xml_attribute<char> *id, *path, *size;
			id = n->first_attribute("id");
			path = n->first_attribute("path");
			size = n->first_attribute("size");

			if (id != NULL && path != NULL && size != NULL)
			{
				unsigned int pos = StringToNumber<unsigned int>(id->value());
				if (font.size() <= pos)
					font.resize(pos + 1);
				font.at(pos) = TTF_OpenFont(path->value(), StringToNumber<int>(size->value()));
				TTF_SetFontHinting(font.at(pos), TTF_HINTING_LIGHT);
			}
		}
	}

	colpool.Load(gFilePath.colors);
}

void TextManager::Reset()
{
	cache.clear();
	cache.resize(cache_size);
}

//------------------------------------------------------------------------
// Purpose: Search cache for rendered text
//------------------------------------------------------------------------
int TextManager::Search(const std::string &text, int col, FontKey fontid)
{
	int pos = 0;
	for (auto i = cache.begin(); i != cache.end(); ++i, ++pos)
		if (i->empty == false && i->text == text && i->EqualCol(col) && i->font == fontid)
			return pos;

	return -1;
}

int TextManager::FindFreeSlot()
{
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
SDL_Surface* TextManager::RenderTextBlended(const FontKey &font, const std::string &text, const int &color)
{
	if (text.empty())
		return TTF_RenderText_Blended(GetFont(font), " ", colpool.Get(color));

	return TTF_RenderText_Blended(GetFont(font), text.c_str(), colpool.Get(color));
}

//------------------------------------------------------------------------
// Purpose: Draw text
//------------------------------------------------------------------------
void TextManager::Draw(const int &x, const int &y, const std::string &text, const int &color,
	const FontKey &font, const Align &align, const bool &background)
{
	int pos = Search(text, color, font);
	if (pos == -1)
	{
		pos = FindFreeSlot();
		SDL_Surface *surf = RenderTextBlended(font, text, color);

		cache[pos].img.Delete();
		cache[pos].empty = false;

		cache[pos].text = text;
		cache[pos].col = color;
		cache[pos].font = font;

		cache[pos].img.Load(surf);
		SDL_FreeSurface(surf);
	}

	if (background)
	{
		rect.w = cache[pos].img.W() + (2 * pad_bg.x);
		rect.h = cache[pos].img.H() + (2 * pad_bg.y);

		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 128);

		if (align == ALIGN_LEFT)
		{
			rect.x = x - pad_bg.x;
			rect.y = y - pad_bg.y;

			SDL_RenderFillRect(gRenderer, &rect);
			cache[pos].img.Draw(x, y);
		}
		else if (align == ALIGN_CENTER)
		{
			rect.x = x - cache[pos].img.W() / 2 - pad_bg.x;
			rect.y = y - cache[pos].img.H() / 2 - pad_bg.y;

			SDL_RenderFillRect(gRenderer, &rect);
			cache[pos].img.Draw(x - cache[pos].img.W() / 2, y - cache[pos].img.H() / 2);
		}
		else
		{
			rect.x = x - cache[pos].img.W() - pad_bg.x;
			rect.y = y - pad_bg.y;

			SDL_RenderFillRect(gRenderer, &rect);
			cache[pos].img.Draw(x - cache[pos].img.W(), y);
		}
	}
	else
	{
		if (align == ALIGN_LEFT) cache[pos].img.Draw(x, y);
		else if (align == ALIGN_CENTER) cache[pos].img.Draw(x - cache[pos].img.W() / 2, y - cache[pos].img.H() / 2);
		else cache[pos].img.Draw(x - cache[pos].img.W(), y);
	}
}

void TextManager::Draw(const int &x, int y, const std::string &text, const int &color, const FontKey &font, const Align &align,
	const unsigned int &line_width, const unsigned int &line_height, const bool &background)
{
	for (int start_pos = 0, len = text.length(); start_pos < len; y += line_height)
	{
		int end_pos = start_pos + 1, last_interrupt = -1;
		std::string word;

		while (end_pos - start_pos <= line_width)
		{
			if (end_pos == len || text[end_pos] == '`')
			{
				last_interrupt = end_pos;
				break;
			}

			if (text[end_pos] == ' ' || text[end_pos] == ',' || text[end_pos] == '.')
				last_interrupt = end_pos;

			end_pos++;
		}

		if (last_interrupt >= 0) //wrap a word around
		{
			for (int i = 0; i < last_interrupt - start_pos; i++)
				word += text[start_pos + i];

			start_pos = last_interrupt + 1;
		}
		else //word bigger than line, just thunk
		{
			for (int i = 0; i < end_pos - start_pos; i++)
				word += text[start_pos + i];

			start_pos += line_width;
		}

		Draw(x, y, word, color, font, align, background);
	}
}

//------------------------------------------------------------------------
// Purpose: Quit
//------------------------------------------------------------------------
void TextManager::Quit()
{
	for (auto i = font.begin(); i != font.end(); ++i)
		TTF_CloseFont(*i);

	for (auto i = cache.begin(); i != cache.end(); ++i)
	{
		if (i->empty == false)
		{
			i->img.Delete();
			i->empty = true;
		}
	}
}