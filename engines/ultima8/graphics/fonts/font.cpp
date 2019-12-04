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

#include "ultima8/misc/pent_include.h"

#include "ultima8/graphics/fonts/font.h"

namespace Ultima8 {
namespace Pentagram {

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Font)

Font::Font() : highRes(false) {

}


Font::~Font() {
}


void Font::getTextSize(const std::string &text,
                       int &resultwidth, int &resultheight,
                       unsigned int &remaining,
                       int width, int height, TextAlign align,
                       bool u8specials) {
	std::list<PositionedText> tmp;
	tmp = typesetText<Traits>(this, text, remaining,
	                          width, height, align, u8specials,
	                          resultwidth, resultheight);
}


//static
bool Font::Traits::canBreakAfter(std::string::const_iterator &i) {
	// It's not really relevant what we do here, because this probably will
	// not be used at normal font sizes.
	return true;
}


//static
bool Font::SJISTraits::canBreakAfter(std::string::const_iterator &i) {
	std::string::const_iterator j = i;
	uint32 u1 = unicode(j);

	// See: http://www.wesnoth.org/wiki/JapaneseTranslation#Word-Wrapping
	// and: http://ja.wikipedia.org/wiki/%E7%A6%81%E5%89%87

	switch (u1) {
	case 0xff08:
	case 0x3014:
	case 0xff3b:
	case 0xff5b:
	case 0x3008:
	case 0x300a:
	case 0x300c:
	case 0x300e:
	case 0x3010:
	case 0x2018:
	case 0x201c:
		return false;
	default:
		break;
	}

	uint32 u2 = unicode(j);
	switch (u2) {
	case 0x3001:
	case 0x3002:
	case 0xff0c:
	case 0xff0e:
	case 0xff09:
	case 0x3015:
	case 0xff3d:
	case 0xff5d:
	case 0x3009:
	case 0x300b:
	case 0x300d:
	case 0x300f:
	case 0x3011:
	case 0x2019:
	case 0x201d:
	case 0x309d:
	case 0x309e:
	case 0x30fd:
	case 0x30fe:
	case 0x3005:
	case 0xff1f:
	case 0xff01:
	case 0xff1a:
	case 0xff1b:
	case 0x3041:
	case 0x3043:
	case 0x3045:
	case 0x3047:
	case 0x3049:
	case 0x3083:
	case 0x3085:
	case 0x3087:
	case 0x308e:
	case 0x30a1:
	case 0x30a3:
	case 0x30a5:
	case 0x30a7:
	case 0x30a9:
	case 0x30e3:
	case 0x30e5:
	case 0x30e7:
	case 0x30ee:
	case 0x3063:
	case 0x30f5:
	case 0x30c3:
	case 0x30f6:
	case 0x30fb:
	case 0x2026:
	case 0x30fc:
		return false;
	default:
		break;
	}

	// Also don't allow breaking between roman characters
	if (((u1 >= 'A' && u1 <= 'Z') || (u1 >= 'a' && u1 <= 'z')) &&
	        ((u2 >= 'A' && u2 <= 'Z') || (u2 >= 'a' && u2 <= 'z'))) {
		return false;
	}
	return true;
}

} // End of namespace Pentagram

template<class T>
static void findWordEnd(const std::string &text,
                        std::string::const_iterator &iter, bool u8specials) {
	while (iter != text.end()) {
		if (T::isSpace(iter, u8specials)) return;
		T::advance(iter);
	}
}

template<class T>
static void passSpace(const std::string &text,
                      std::string::const_iterator &iter, bool u8specials) {
	while (iter != text.end()) {
		if (!T::isSpace(iter, u8specials)) return;
		T::advance(iter);
	}
	return;
}




/*
  Special characters in U8:

@ = bullet for conversation options
~ = line break
% = tab
* = line break on graves and plaques, possibly page break in books
CHECKME: any others? (page breaks for books?)

*/

template<class T>
std::list<PositionedText> typesetText(Pentagram::Font *font,
                                      const std::string &text,
                                      unsigned int &remaining,
                                      int width, int height,
                                      Pentagram::Font::TextAlign align,
                                      bool u8specials,
                                      int &resultwidth, int &resultheight,
                                      std::string::size_type cursor) {
#if 0
	pout << "typeset (" << width << "," << height << ") : "
	     << text << std::endl;
#endif

	// be optimistic and assume everything will fit
	remaining = text.size();

	std::string curline;

	int totalwidth = 0;
	int totalheight = 0;

	std::list<PositionedText> lines;
	PositionedText line;

	std::string::const_iterator iter = text.begin();
	std::string::const_iterator cursoriter = text.begin();
	if (cursor != std::string::npos) cursoriter += cursor;
	std::string::const_iterator curlinestart = text.begin();

	bool breakhere = false;
	while (true) {
		if (iter == text.end() || breakhere || T::isBreak(iter, u8specials)) {
			// break here
			int stringwidth = 0, stringheight = 0;
			font->getStringSize(curline, stringwidth, stringheight);
			line.dims.x = 0;
			line.dims.y = totalheight;
			line.dims.w = stringwidth;
			line.dims.h = stringheight;
			line.text = curline;
			line.cursor = std::string::npos;
			if (cursor != std::string::npos && cursoriter >= curlinestart &&
			        (cursoriter < iter || (!breakhere && cursoriter == iter))) {
				line.cursor = cursoriter - curlinestart;
				if (line.dims.w == 0) {
					stringwidth = line.dims.w = 2;
				}
			}
			lines.push_back(line);

			if (stringwidth > totalwidth) totalwidth = stringwidth;
			totalheight += font->getBaselineSkip();

			curline = "";

			if (iter == text.end())
				break; // done

			if (breakhere) {
				breakhere = false;
				curlinestart = iter;
			} else {
				T::advance(iter);
				curlinestart = iter;
			}

			if (height != 0 && totalheight + font->getHeight() > height) {
				// next line won't fit
				remaining = curlinestart - text.begin();
				break;
			}

		} else {

			// see if next word still fits on the current line
			std::string::const_iterator nextword = iter;
			passSpace<T>(text, nextword, u8specials);

			// process spaces
			bool foundLF = false;
			std::string spaces;
			for (; iter < nextword; T::advance(iter)) {
				if (T::isBreak(iter, u8specials)) {
					foundLF = true;
					break;
				} else if (T::isTab(iter, u8specials)) {
					spaces.append("    ");
				} else if (!curline.empty()) {
					spaces.append(" ");
				}
			}
			if (foundLF) continue;

			// process word
			std::string::const_iterator endofnextword = iter;
			findWordEnd<T>(text, endofnextword, u8specials);
			int stringwidth = 0, stringheight = 0;
			std::string newline = curline + spaces +
			                      text.substr(nextword - text.begin(), endofnextword - nextword);
			font->getStringSize(newline, stringwidth, stringheight);

			// if not, break line before this word
			if (width != 0 && stringwidth > width) {
				if (!curline.empty()) {
					iter = nextword;
				} else {
					// word is longer than the line; have to break in mid-word
					// FIXME: this is rather inefficient; binary search?
					// FIXME: clean up...
					iter = nextword;
					std::string::const_iterator saveiter = nextword;	// Dummy initialization
					std::string::const_iterator saveiter_fail;
					std::string curline_fail;
					newline = spaces;
					bool breakok = true;
					int breakcount = -1;
					do {
						if (breakok) {
							curline = newline;
							saveiter = iter;
							breakcount++;
						}
						curline_fail = newline;
						saveiter_fail = iter;

						if (iter == text.end()) break;

						breakok = T::canBreakAfter(iter);

						// try next character
						T::advance(iter);
						newline = spaces + text.substr(nextword - text.begin(),
						                               iter - nextword);
						font->getStringSize(newline, stringwidth, stringheight);
					} while (stringwidth <= width);
					if (breakcount > 0) {
						iter = saveiter;
					} else {
						iter = saveiter_fail;
						curline = curline_fail;
					}
				}
				breakhere = true;
				continue;
			} else {
				// copy next word into curline
				curline = newline;
				iter = endofnextword;
			}
		}
	}

	if (lines.size() == 1 && align == Pentagram::Font::TEXT_LEFT) {
		// only one line, so use the actual text width
		width = totalwidth;
	}

	if (width != 0) totalwidth = width;

	// adjust total height
	totalheight -= font->getBaselineSkip();
	totalheight += font->getHeight();

	// fixup x coordinates of lines
	std::list<PositionedText>::iterator lineiter;
	for (lineiter = lines.begin(); lineiter != lines.end(); ++lineiter) {
		switch (align) {
		case Pentagram::Font::TEXT_LEFT:
			break;
		case Pentagram::Font::TEXT_RIGHT:
			lineiter->dims.x = totalwidth - lineiter->dims.w;
			break;
		case Pentagram::Font::TEXT_CENTER:
			lineiter->dims.x = (totalwidth - lineiter->dims.w) / 2;
			break;
		}
#if 0
		pout << lineiter->dims.x << "," << lineiter->dims.y << " "
		     << lineiter->dims.w << "," << lineiter->dims.h << ": "
		     << lineiter->text << std::endl;
#endif
	}

	resultwidth = totalwidth;
	resultheight = totalheight;

	return lines;
}


// explicit instantiations
template
std::list<PositionedText> typesetText<Pentagram::Font::Traits>
(Pentagram::Font *font, const std::string &text,
 unsigned int &remaining, int width, int height,
 Pentagram::Font::TextAlign align, bool u8specials,
 int &resultwidth, int &resultheight, std::string::size_type cursor);

template
std::list<PositionedText> typesetText<Pentagram::Font::SJISTraits>
(Pentagram::Font *font, const std::string &text,
 unsigned int &remaining, int width, int height,
 Pentagram::Font::TextAlign align, bool u8specials,
 int &resultwidth, int &resultheight, std::string::size_type cursor);

} // End of namespace Ultima8
