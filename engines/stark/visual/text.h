/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_VISUAL_TEXT_H
#define STARK_VISUAL_TEXT_H

#include "engines/stark/visual/visual.h"

#include "engines/stark/services/fontprovider.h"

#include "common/rect.h"

namespace Stark {

namespace Gfx {
class Driver;
class SurfaceRenderer;
class Texture;
}

/**
 * Text renderer
 */
class VisualText : public Visual {
public:
	static const VisualType TYPE = Visual::kImageText;

	VisualText(Gfx::Driver *gfx);
	virtual ~VisualText();

	Common::Rect getRect();
	Gfx::Texture *getTexture();

	void setText(const Common::String &text);
	void setColor(uint32 color);
	void setBackgroundColor(uint32 color);
	void setTargetWidth(uint32 width);
	void setFont(FontProvider::FontType type, int32 customFontIndex = -1);

	void render(const Common::Point &position);

private:
	void createTexture();
	void freeTexture();

	Gfx::Driver *_gfx;
	Gfx::SurfaceRenderer *_surfaceRenderer;
	Gfx::Texture *_texture;

	Common::String _text;
	uint32 _color;
	uint32 _backgroundColor;
	Common::Rect _originalRect;

	FontProvider::FontType _fontType;
	int32 _fontCustomIndex;
};

} // End of namespace Stark

#endif // STARK_VISUAL_TEXT_H
