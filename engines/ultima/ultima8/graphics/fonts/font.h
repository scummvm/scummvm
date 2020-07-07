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

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/rect.h"
#include "ultima/ultima8/misc/encoding.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class RenderedText;

struct PositionedText {
	Std::string _text;
	Rect _dims;
	Std::string::size_type _cursor;
};

class Font {
public:
	Font();
	virtual ~Font();

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
	virtual void getStringSize(const Std::string &text,
		int32 &width, int32 &height) = 0;

	//! render a string
	//! \param text The text
	//! \param remaining Returns index of the first character not printed
	//! \param width The width of the target rectangle, or 0 for unlimited
	//! \param height The height of the target rectangle, or 0 for unlimited
	//! \param align Alignment of the text (left, right, center)
	//! \param u8specials If true, interpret the special characters U8 uses
	//! \return the rendered text in a RenderedText object
	virtual RenderedText *renderText(const Std::string &text,
	    unsigned int &remaining, int32 width = 0, int32 height = 0,
		TextAlign align = TEXT_LEFT, bool u8specials = false,
		Std::string::size_type cursor = Std::string::npos) = 0;

	//! get the dimensions of a rendered string
	//! \param text The text
	//! \param resultwidth Returns the resulting width
	//! \param resultheight Returns the resulting height
	//! \param remaining Returns index of the first character not printed
	//! \param width The width of the target rectangle, or 0 for unlimited
	//! \param height The height of the target rectangle, or 0 for unlimited
	//! \param u8specials If true, interpret the special characters U8 uses
	//! \param align Alignment of the text (left, right, center)
	virtual void getTextSize(const Std::string &text,
		int32 &resultwidth, int32 &resultheight, unsigned int &remaining,
		int32 width = 0, int32 height = 0, TextAlign align = TEXT_LEFT,
		bool u8specials = false);

	void setHighRes(bool hr) {
		_highRes = hr;
	}
	bool isHighRes() const {
		return _highRes;
	}

protected:
	bool _highRes;

protected:

	struct Traits {
		static bool isSpace(Std::string::const_iterator &i, bool u8specials) {
			char c = *i;
			return (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
			        (u8specials && (c == '%' || c == '~' || c == '*' || c == '^')));
		}
		static bool isTab(Std::string::const_iterator &i, bool u8specials) {
			char c = *i;
			return (c == '\t' ||
			        (u8specials && (c == '\t' || c == '%')));
		}
		static bool isBreak(Std::string::const_iterator &i, bool u8specials) {
			char c = *i;
			return (c == '\n' ||
			        (u8specials && (c == '\n' || c == '~' || c == '*')));
		}
		static bool canBreakAfter(Std::string::const_iterator &i);
		static void advance(Std::string::const_iterator &i) {
			++i;
		}
		static Std::string::size_type length(const Std::string &t) {
			return t.size();
		}
		static uint32 unicode(Std::string::const_iterator &i) {
			return encoding[static_cast<uint8>(*i++)];
		}
	};
	struct SJISTraits : public Traits {
		static bool canBreakAfter(Std::string::const_iterator &i);
		static void advance(Std::string::const_iterator &i) {
			// FIXME: this can advance past the end of a malformed string
			uint8 c = *i;
			i++;
			if (c >= 0x80) i++;
		}
		static Std::string::size_type length(const Std::string &t) {
			Std::string::size_type l = 0;
			Std::string::const_iterator iter = t.begin();
			while (iter != t.end()) {
				advance(iter);
				l++;
			}
			return l;
		}
		static uint32 unicode(Std::string::const_iterator &i) {
			uint16 s = static_cast<uint8>(*i);
			i++;
			if (s >= 0x80) {
				uint16 t = static_cast<uint8>(*i++);
				s |= (t << 8);
			}
			return shiftjis_to_unicode(s);
		}
	};
};

template<class T>
Std::list<PositionedText> typesetText(Font *font,
	const Std::string &text, unsigned int &remaining,
	int32 width, int32 height, Font::TextAlign align,
	bool u8specials, int32 &resultwidth, int32 &resultheight,
	Std::string::size_type cursor = Std::string::npos);

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
