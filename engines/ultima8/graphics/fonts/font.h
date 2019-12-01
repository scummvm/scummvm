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

#ifndef ULTIMA8_GRAPHICS_FONTS_FONT_H
#define ULTIMA8_GRAPHICS_FONTS_FONT_H

#include "ultima8/std/containers.h"
#include "ultima8/misc/rect.h"
#include "ultima8/misc/encoding.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class RenderedText;

struct PositionedText {
	std::string text;
	Pentagram::Rect dims;
	std::string::size_type cursor;
};

namespace Pentagram {

class Font {
public:
	Font();
	virtual ~Font();

	ENABLE_RUNTIME_CLASSTYPE()

	enum TextAlign {
		TEXT_LEFT,
		TEXT_CENTER,
		TEXT_RIGHT
	};

	//! get the height of the font
	virtual int getHeight() = 0;

	//! get the baseline of the font (relative from the top)
	virtual int getBaseline() = 0;

	//! get the baselineskip of the font (distance between two baselines)
	virtual int getBaselineSkip() = 0;

	//! get the dimensions of a string (not containing any newlines)
	//! \param text The string
	//! \param width Returns the width
	//! \param height Returns the height
	virtual void getStringSize(const std::string &text,
	                           int &width, int &height) = 0;

	//! render a string
	//! \param text The text
	//! \param remaining Returns index of the first character not printed
	//! \param width The width of the target rectangle, or 0 for unlimited
	//! \param height The height of the target rectangle, or 0 for unlimited
	//! \param align Alignment of the text (left, right, center)
	//! \param u8specials If true, interpret the special characters U8 uses
	//! \return the rendered text in a RenderedText object
	virtual RenderedText *renderText(const std::string &text,
	                                 unsigned int &remaining,
	                                 int width = 0, int height = 0,
	                                 TextAlign align = TEXT_LEFT,
	                                 bool u8specials = false,
	                                 std::string::size_type cursor
	                                 = std::string::npos) = 0;

	//! get the dimensions of a rendered string
	//! \param text The text
	//! \param resultwidth Returns the resulting width
	//! \param resultheight Returns the resulting height
	//! \param remaining Returns index of the first character not printed
	//! \param width The width of the target rectangle, or 0 for unlimited
	//! \param height The height of the target rectangle, or 0 for unlimited
	//! \param u8specials If true, interpret the special characters U8 uses
	//! \param align Alignment of the text (left, right, center)
	virtual void getTextSize(const std::string &text,
	                         int &resultwidth, int &resultheight,
	                         unsigned int &remaining,
	                         int width = 0, int height = 0,
	                         TextAlign align = TEXT_LEFT, bool u8specials = false);

	void setHighRes(bool hr) {
		highRes = hr;
	}
	bool isHighRes() const {
		return highRes;
	}

protected:
	bool highRes;


protected:

	struct Traits {
		static bool isSpace(std::string::const_iterator &i, bool u8specials) {
			char c = *i;
			return (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
			        (u8specials && (c == '%' || c == '~' || c == '*')));
		}
		static bool isTab(std::string::const_iterator &i, bool u8specials) {
			char c = *i;
			return (c == '\t' ||
			        (u8specials && (c == '\t' || c == '%')));
		}
		static bool isBreak(std::string::const_iterator &i, bool u8specials) {
			char c = *i;
			return (c == '\n' ||
			        (u8specials && (c == '\n' || c == '~' || c == '*')));
		}
		static bool canBreakAfter(std::string::const_iterator &i);
		static void advance(std::string::const_iterator &i) {
			++i;
		}
		static std::string::size_type length(const std::string &t) {
			return t.size();
		}
		static uint32 unicode(std::string::const_iterator &i) {
			return Pentagram::encoding[static_cast<uint8>(*i++)];
		}
	};
	struct SJISTraits : public Traits {
		static bool canBreakAfter(std::string::const_iterator &i);
		static void advance(std::string::const_iterator &i) {
			// FIXME: this can advance past the end of a malformed string
			uint8 c = *i;
			i++;
			if (c >= 0x80) i++;
		}
		static std::string::size_type length(const std::string &t) {
			std::string::size_type l = 0;
			std::string::const_iterator iter = t.begin();
			while (iter != t.end()) {
				advance(iter);
				l++;
			}
			return l;
		}
		static uint32 unicode(std::string::const_iterator &i) {
			uint16 s = static_cast<uint8>(*i);
			i++;
			if (s >= 0x80) {
				uint16 t = static_cast<uint8>(*i++);
				s |= (t << 8);
			}
			return Pentagram::shiftjis_to_unicode(s);
		}
	};
};

}

template<class T>
std::list<PositionedText> typesetText(Pentagram::Font *font,
                                      const std::string &text,
                                      unsigned int &remaining,
                                      int width, int height,
                                      Pentagram::Font::TextAlign align,
                                      bool u8specials,
                                      int &resultwidth, int &resultheight,
                                      std::string::size_type cursor
                                      = std::string::npos);

} // End of namespace Ultima8

#endif
