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

#include "graphics/font.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/font.h"
#include "engines/nancy/util.h"

#include "engines/nancy/misc/hypertext.h"

#include "engines/nancy/ui/ndui/element.h"

namespace Nancy {
namespace UI {
namespace NDUI {

static const uint32 kDefaultColor = 0xFFFFFFFF;
static const uint32 kDefaultDisabledColor = 0xC8808080;

// Lays out text, with its markup, in the font and color of an element
class ElementText : public Misc::HypertextParser {
public:
	enum Alignment {
		kAlignLeft		= 0,
		kAlignRight		= 1,
		kAlignCenter	= 2
	};

	const Graphics::ManagedSurface &render(const Common::String &text, int width, int height, uint fontID, byte r, byte g, byte b, int32 alignment, ControlState state) {
		_fullSurface.create(width, height, g_nancy->_graphics->getTransparentPixelFormat());
		_fullSurface.clear(0);

		// The control states and the font color states come in the same order
		_fontColorState = (FontColorState)state;

		if (alignment == kAlignCenter) {
			_defaultJustification = kJustifyCenter;
		} else if (alignment == kAlignRight) {
			_defaultJustification = kJustifyRight;
		}

		setDefaultTextRGB(r, g, b);
		addTextLine(text);
		drawAllText(Common::Rect(width, height), 0, fontID, fontID);
		return _fullSurface;
	}

	// The number of lines the text gets wrapped into
	uint countLines(const Common::String &text, int width, uint fontID) {
		_fullSurface.create(width, 1, g_nancy->_graphics->getTransparentPixelFormat());
		addTextLine(text);
		drawAllText(Common::Rect(width, 1), 0, fontID, fontID);

		// Every text gets an empty line after it
		return _numDrawnLines - 1;
	}
};

void Element::read(Common::SeekableReadStream &stream) {
	Common::String textureName;
	Common::Rect src;
	readFilename(stream, textureName);
	readExclusiveRect(stream, src);
	setTexture(textureName, src);

	_colors[kStateNormal] = stream.readUint32BE();
	_colors[kStateDisabled] = stream.readUint32BE();
	_colors[kStateMouseOver] = stream.readUint32BE();
	_colors[kStatePressed] = stream.readUint32BE();

	readFilename(stream, _fontName);
	if (stream.readByte()) {
		_fontName += "_Bold";
	}

	_horizontalAlignment = stream.readSint32LE();
	_verticalAlignment = stream.readSint32LE();
	_wordWrap = stream.readByte();

	setFontColors(stream.readUint32BE());
	_fontColors[kStateDisabled] = stream.readUint32BE();
	_fontColors[kStateMouseOver] = stream.readUint32BE();
	_fontColors[kStatePressed] = stream.readUint32BE();

	if (g_nancy->getGameType() >= kGameTypeNancy17) {
		stream.skip(1);	// A flag of the control, taken from its first element
	}
}

void Element::setTexture(const Common::String &textureName, const Common::Rect &src) {
	_textureName = textureName;
	_src = src;

	for (uint i = 0; i < kNumStates; ++i) {
		_colors[i] = kDefaultColor;
	}

	_colors[kStateDisabled] = kDefaultDisabledColor;
	_colors[kStateHidden] = 0;
}

void Element::setFontColors(uint32 normalColor) {
	for (uint i = 0; i < kNumStates; ++i) {
		_fontColors[i] = normalColor;
	}

	_fontColors[kStateDisabled] = kDefaultDisabledColor;
	_fontColors[kStateHidden] = 0;
}

void Element::moveFontTo(Element &other) {
	other._fontName = _fontName;
	other._horizontalAlignment = _horizontalAlignment;
	other._verticalAlignment = _verticalAlignment;
	other._wordWrap = _wordWrap;
	for (uint i = 0; i < kNumStates; ++i) {
		other._fontColors[i] = _fontColors[i];
	}

	setFontColors(0);
}

bool Element::blend(ControlState state, float seconds, float rate) {
	_state = state;
	float amount = getBlendAmount(seconds, rate);
	bool changed = blendColor(_current, _colors[state], amount);
	changed |= blendColor(_currentFont, _fontColors[state], amount);
	return changed;
}

// The texture is stretched to fill the destination, and tinted by the current color
void Element::drawSprite(Graphics::ManagedSurface &surface, const Common::Rect &src, const Common::Rect &dest, const Graphics::ManagedSurface &texture) const {
	byte tintA = colorChannelToByte(_current[0]);
	if (tintA == 0 || src.isEmpty() || dest.isEmpty()) {
		return;
	}

	byte tintR = colorChannelToByte(_current[1]);
	byte tintG = colorChannelToByte(_current[2]);
	byte tintB = colorChannelToByte(_current[3]);

	Common::Rect clipped = dest.findIntersectingRect(Common::Rect(surface.w, surface.h));
	for (int y = clipped.top; y < clipped.bottom; ++y) {
		int srcY = src.top + (y - dest.top) * src.height() / dest.height();
		for (int x = clipped.left; x < clipped.right; ++x) {
			int srcX = src.left + (x - dest.left) * src.width() / dest.width();
			if (srcX < 0 || srcY < 0 || srcX >= texture.w || srcY >= texture.h) {
				continue;
			}

			byte a, r, g, b;
			texture.format.colorToARGB(texture.getPixel(srcX, srcY), a, r, g, b);
			a = a * tintA / 255;
			if (a != 0) {
				blendPixel(surface, x, y, a, r * tintR / 255, g * tintG / 255, b * tintB / 255);
			}
		}
	}
}

int Element::measureTextHeight(const Common::String &text, int width) const {
	int fontID = g_nancy->_graphics->getFontID(_fontName);
	if (text.empty() || width <= 0 || fontID < 0 || !g_nancy->_graphics->getFont((uint)fontID)->getSystemFont()) {
		return 0;
	}

	ElementText layout;
	return layout.countLines(text, width, (uint)fontID) * getFontHeight();
}

const Graphics::Font *Element::getSystemFont() const {
	const Font *font = g_nancy->_graphics->getFont(_fontName);
	return font ? font->getSystemFont() : nullptr;
}

int Element::getFontHeight() const {
	const Font *font = g_nancy->_graphics->getFont(_fontName);
	return font && font->getSystemFont() ? font->getSystemFont()->getFontHeight() : 0;
}

void Element::drawText(Graphics::ManagedSurface &surface, const Common::String &text, const Common::Rect &dest, bool shadow) const {
	drawText(surface, text, dest, _currentFont, _state, shadow);
}

void Element::drawText(Graphics::ManagedSurface &surface, const Common::String &text, const Common::Rect &dest, const float *color, ControlState state, bool shadow) const {
	byte alpha = colorChannelToByte(color[0]);
	int fontID = g_nancy->_graphics->getFontID(_fontName);
	if (text.empty() || alpha == 0 || dest.isEmpty() || fontID < 0 || !g_nancy->_graphics->getFont((uint)fontID)->getSystemFont()) {
		return;
	}

	ElementText layout;
	const Graphics::ManagedSurface &rendered = layout.render(text, dest.width(), dest.height(), (uint)fontID,
		colorChannelToByte(color[1]), colorChannelToByte(color[2]), colorChannelToByte(color[3]), _horizontalAlignment, state);

	// Only text kept on a single line gets aligned vertically
	int top = dest.top;
	if (!_wordWrap) {
		int lineHeight = getFontHeight();
		if (_verticalAlignment == kAlignVCenter) {
			top += (dest.height() - lineHeight) / 2;
		} else if (_verticalAlignment == kAlignBottom) {
			top += dest.height() - lineHeight;
		}
	}

	// The text gets blended in with the color's alpha. The shadow is the same shape
	// in black, one pixel down and to the right.
	for (uint pass = shadow ? 0 : 1; pass < 2; ++pass) {
		int offset = pass == 0 ? 1 : 0;
		for (int y = 0; y < rendered.h; ++y) {
			int destY = top + y + offset;
			if (destY < 0 || destY >= surface.h) {
				continue;
			}

			for (int x = 0; x < rendered.w; ++x) {
				int destX = dest.left + x + offset;
				if (destX < 0 || destX >= surface.w) {
					continue;
				}

				byte a, r, g, b;
				rendered.format.colorToARGB(rendered.getPixel(x, y), a, r, g, b);
				a = a * alpha / 255;
				if (a == 0) {
					continue;
				}

				if (pass == 0) {
					blendPixel(surface, destX, destY, a, 0, 0, 0);
				} else {
					blendPixel(surface, destX, destY, a, r, g, b);
				}
			}
		}
	}
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
