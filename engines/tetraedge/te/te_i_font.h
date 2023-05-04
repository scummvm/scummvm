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

#ifndef TETRAEDGE_TE_TE_I_FONT_H
#define TETRAEDGE_TE_TE_I_FONT_H

#include "common/str.h"
#include "graphics/font.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

/**
 * A wrapper to provide a TeFont-like interface using ScummVM'S font class.
 */
class TeIFont : public TeResource {
public:
	enum AlignStyle {
		AlignLeft,
		AlignRight,
		AlignJustify,
		AlignCenter
	};

	struct GlyphData {
		uint32 _charcode;
		Common::Rect _bitmapSize;
		TeIntrusivePtr<TeImage> _img;
	};

	TeIFont();
	virtual ~TeIFont();
	virtual Graphics::Font *getAtSize(uint size) = 0;

	virtual float ascender(uint pxSize);
	virtual float descender(uint pxSize);
	virtual float height(uint pxSize);
	virtual TeVector3f32 kerning(uint pxSize, uint isocode1, uint isocode2);

	virtual void draw(TeImage &destImage, const Common::String &str, int fontSize, int yoff, const TeColor &col, AlignStyle alignMode);
	virtual Common::Rect getBBox(const Common::String &str, int fontSize);
	virtual int getHeight(int fontSize);
	virtual int wordWrapText(const Common::String &str, int fontSize, int maxWidth, Common::Array<Common::String> &lines);

	virtual TeIFont::GlyphData glyph(uint pxSize, uint charcode);

protected:
	Common::CodePage _codePage;

private:
	Common::CodePage codePage() const;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_I_FONT_H
