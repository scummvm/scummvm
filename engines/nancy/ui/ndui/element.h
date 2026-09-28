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

#ifndef NANCY_UI_NDUI_ELEMENT_H
#define NANCY_UI_NDUI_ELEMENT_H

#include "common/rect.h"
#include "engines/nancy/ui/ndui/common.h"

namespace Graphics {
class Font;
class ManagedSurface;
}

namespace Nancy {
namespace UI {
namespace NDUI {

// One visual part of a control: a region of a texture and a font, each with a color
// for every control state. A control holds one element or more (a button has its face,
// a scroll bar its track, arrows and thumb), and tells them which state it is in.
// Each element then fades its colors toward the ones of that state, and draws its
// texture tinted with them, and text in its font. Its control gets the texture by name
// from the Manager, which keeps them loaded.
class Element {
public:
	void read(Common::SeekableReadStream &stream);
	void setTexture(const Common::String &textureName, const Common::Rect &src);

	// Hands the text settings over to another element, keeping only a disabled color
	void moveFontTo(Element &other);

	// Moves the current colors toward the ones for the given state; returns true
	// when the colors that get drawn have changed
	bool blend(ControlState state, float seconds, float rate);

	void drawSprite(Graphics::ManagedSurface &surface, const Common::Rect &src, const Common::Rect &dest, const Graphics::ManagedSurface &texture) const;
	void drawText(Graphics::ManagedSurface &surface, const Common::String &text, const Common::Rect &dest, bool shadow) const;
	void drawText(Graphics::ManagedSurface &surface, const Common::String &text, const Common::Rect &dest, const float *color, ControlState state, bool shadow) const;

	uint32 getFontColor(ControlState state) const { return _fontColors[state]; }
	int getFontHeight() const;

	// The height the text takes up when it gets wrapped to the given width
	int measureTextHeight(const Common::String &text, int width) const;
	const Graphics::Font *getSystemFont() const;
	void setTextureName(const Common::String &textureName) { _textureName = textureName; }

	Common::String _textureName;
	Common::Rect _src;
	uint32 _colors[kNumStates] = {};

private:
	enum VerticalAlignment {
		kAlignVCenter	= 2,
		kAlignTop		= 3,
		kAlignBottom	= 4
	};

	void setFontColors(uint32 normalColor);

	Common::String _fontName;
	int32 _horizontalAlignment = 0;		// left, right or centered
	int32 _verticalAlignment = kAlignTop;
	bool _wordWrap = false;
	uint32 _fontColors[kNumStates] = {};

	// ARGB, 0 to 1; both start at the hidden color
	float _current[4] = {};
	float _currentFont[4] = {};

	// The state the colors are moving toward; color tags in the text follow it
	ControlState _state = kStateHidden;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_ELEMENT_H
