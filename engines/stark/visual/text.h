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

#ifndef STARK_VISUAL_TEXT_H
#define STARK_VISUAL_TEXT_H

#include "engines/stark/visual/visual.h"

#include "engines/stark/services/fontprovider.h"
#include "engines/stark/gfx/color.h"

#include "common/rect.h"
#include "graphics/font.h"

namespace Stark {

namespace Gfx {
class Driver;
class SurfaceRenderer;
class Bitmap;
}

/**
 * Text renderer
 */
class VisualText : public Visual {
public:
	static const VisualType TYPE = Visual::kImageText;

	explicit VisualText(Gfx::Driver *gfx);
	~VisualText() override;

	Common::Rect getRect();

	void setText(const Common::String &text);
	void setColor(const Gfx::Color &color);
	void setBackgroundColor(const Gfx::Color &color);
	void setAlign(Graphics::TextAlign align);
	void setTargetWidth(uint32 width);
	void setTargetHeight(uint32 height);
	void setFont(FontProvider::FontType type, int32 customFontIndex = -1);

	uint getTargetWidth() { return _targetWidth; }
	uint getTargetHeight() { return _targetHeight; }

	void render(const Common::Point &position);
	void reset();

private:
	void createBitmap();
	void freeBitmap();

	/** Check whether the text is blank */
	bool isBlank();

	Gfx::Driver *_gfx;
	Gfx::SurfaceRenderer *_surfaceRenderer;
	Gfx::Bitmap *_bitmap;

	Common::String _text;
	Gfx::Color _color;
	Gfx::Color _backgroundColor;
	Graphics::TextAlign _align;
	uint32 _targetWidth;
	uint32 _targetHeight;
	Common::Rect _originalRect;

	FontProvider::FontType _fontType;
	int32 _fontCustomIndex;
};

} // End of namespace Stark

#endif // STARK_VISUAL_TEXT_H
