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

#include "ultima/ultima8/graphics/fonts/font.h"

namespace Ultima {
namespace Ultima8 {

Font::Font() : _highRes(false) {
}


Font::~Font() {
}


void Font::getTextSize(const Std::string &text,
                       int32 &resultwidth, int32 &resultheight,
                       unsigned int &remaining,
                       int32 width, int32 height, TextAlign align,
                       bool u8specials) {
	Std::list<PositionedText> tmp;
	tmp = typesetText<Traits>(this, text, remaining,
	                          width, height, align, u8specials,
	                          resultwidth, resultheight);
}


//static
bool Font::Traits::canBreakAfter(Std::string::const_iterator &i) {
	// It's not really relevant what we do here, because this probably will
	// not be used at normal font sizes.
	return true;
}


//static
bool Font::SJISTraits::canBreakAfter(Std::string::const_iterator &i) {
	Std::string::const_iterator j = i;
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

template<class T>
static void findWordEnd(const Std::string &text,
                        Std::string::const_iterator &iter, bool u8specials) {
	while (iter != text.end()) {
		if (T::isSpace(iter, u8specials)) return;
		T::advance(iter);
	}
}

template<class T>
static void passSpace(const Std::string &text,
                      Std::string::const_iterator &iter, bool u8specials) {
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
Std::list<PositionedText> typesetText(Font *font,
	const Std::string &text, unsigned int &remaining, int32 width, int32 height,
	Font::TextAlign align, bool u8specials, int32 &resultwidth,
	int32 &resultheight, Std::string::size_type cursor) {
#if 0
	pout << "typeset (" << width << "," << height << ") : "
	     << text << Std::endl;
#endif

	// be optimistic and assume everything will fit
	remaining = text.size();

	Std::string curline;

	int totalwidth = 0;
	int totalheight = 0;

	Std::list<PositionedText> lines;
	PositionedText line;

	Std::string::const_iterator iter = text.begin();
	Std::string::const_iterator cursoriter = text.begin();
	if (cursor != Std::string::npos) cursoriter += cursor;
	Std::string::const_iterator curlinestart = text.begin();

	bool breakhere = false;
	while (true) {
		if (iter == text.end() || breakhere || T::isBreak(iter, u8specials)) {
			// break here
			int32 stringwidth = 0, stringheight = 0;
			font->getStringSize(curline, stringwidth, stringheight);
			line._dims.left = 0;
			line._dims.top = totalheight;
			line._dims.setWidth(stringwidth);
			line._dims.setHeight(stringheight);
			line._text = curline;
			line._cursor = Std::string::npos;
			if (cursor != Std::string::npos && cursoriter >= curlinestart &&
			        (cursoriter < iter || (!breakhere && cursoriter == iter))) {
				line._cursor = cursoriter - curlinestart;
				if (line._dims.width() == 0) {
					stringwidth = 2;
					line._dims.setWidth(stringwidth);
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
			Std::string::const_iterator nextword = iter;
			passSpace<T>(text, nextword, u8specials);

			// process spaces
			bool foundLF = false;
			Std::string spaces;
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
			Std::string::const_iterator endofnextword = iter;
			findWordEnd<T>(text, endofnextword, u8specials);
			int32 stringwidth = 0, stringheight = 0;
			Std::string newline = curline + spaces +
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
					Std::string::const_iterator saveiter = nextword;	// Dummy initialization
					Std::string::const_iterator saveiter_fail;
					Std::string curline_fail;
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

	if (lines.size() == 1 && align == Font::TEXT_LEFT) {
		// only one line, so use the actual text width
		width = totalwidth;
	}

	if (width != 0) totalwidth = width;

	// adjust total height
	totalheight -= font->getBaselineSkip();
	totalheight += font->getHeight();

	// fixup x coordinates of lines
	Std::list<PositionedText>::iterator lineiter;
	for (lineiter = lines.begin(); lineiter != lines.end(); ++lineiter) {
		switch (align) {
		case Font::TEXT_LEFT:
			break;
		case Font::TEXT_RIGHT:
			lineiter->_dims.moveTo(totalwidth - lineiter->_dims.width(), lineiter->_dims.top);
			break;
		case Font::TEXT_CENTER:
			lineiter->_dims.moveTo((totalwidth - lineiter->_dims.width()) / 2, lineiter->_dims.top);
			break;
		}
#if 0
		pout << lineiter->_dims.x << "," << lineiter->_dims.y << " "
		     << lineiter->_dims.width() << "," << lineiter->_dims.height() << ": "
		     << lineiter->text << Std::endl;
#endif
	}

	resultwidth = totalwidth;
	resultheight = totalheight;

	return lines;
}


// explicit instantiations
template
Std::list<PositionedText> typesetText<Font::Traits>
(Font *font, const Std::string &text,
 unsigned int &remaining, int32 width, int32 height,
 Font::TextAlign align, bool u8specials,
 int32 &resultwidth, int32 &resultheight, Std::string::size_type cursor);

template
Std::list<PositionedText> typesetText<Font::SJISTraits>
(Font *font, const Std::string &text,
 unsigned int &remaining, int32 width, int32 height,
 Font::TextAlign align, bool u8specials,
 int32 &resultwidth, int32 &resultheight, Std::string::size_type cursor);

} // End of namespace Ultima8
} // End of namespace Ultima
