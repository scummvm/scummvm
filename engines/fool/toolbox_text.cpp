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

#include "graphics/macgui/mactext-canvas.h"
#include "graphics/macgui/mactext.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/utils.h"

namespace Fool {

void Toolbox::DrawChar(Common::u32char_type_t ch) {
	warning("STUB: Toolbox::DrawChar");
}

void Toolbox::DrawString(const Common::U32String &s) {
	if (_port) {
		Common::String macString = s.encode(Common::kMacRoman);
		debugN(5, "Toolbox::DrawString: (%d, %d) %s\n", _port->pnLoc.x, _port->pnLoc.y, s.encode().c_str());
		Graphics::MacFontRun fontRun(&g_engine->_wm, _port->txFont, _port->txFace, _port->txSize, 0, 0, 0);
		const Graphics::Font *font = fontRun.getFont();
		Common::Rect bbox = font->getBoundingBox(s);
		BitMap buffer(new Graphics::ManagedSurface(bbox.width(), bbox.height(), Graphics::PixelFormat::createFormatCLUT8()));
		buffer->fillRect(buffer->getBounds(), _port->bkColor);
		font->drawString(buffer->surfacePtr(), macString, 0, 0, bbox.width(), _port->fgColor);

		BitMap mask(nullptr);

		Common::Point destPos(_port->pnLoc.x, _port->pnLoc.y-font->getFontAscent());
		// move to port coordinate space
		destPos.x += _port->portRect.left;
		destPos.y += _port->portRect.top;

		Common::Rect result = blitMono(buffer, _port->portBits, mask, destPos, _port->txMode);
		_port->pnLoc.x += bbox.width();
		if (_port->portBits == _defaultBits) {
			_defaultWindow->addDirtyRect(result);
			_defaultWindow->setDirty(true);
		}

		if (_port->picSave) {
			_port->picSave->pushOpU16(kOpTxFace, _port->txFace);
			_port->picSave->pushOpU16(kOpTxFont, _port->txFont);
			_port->picSave->pushOpU16(kOpTxMode, _port->txMode);
			_port->picSave->pushOpU16(kOpTxSize, _port->txSize);
			_port->picSave->pushOpPointStr(kOpLongText, _port->pnLoc, macString);
		}
	}
}

uint16 Toolbox::StringWidth(const Common::U32String &s) {
	if (_port) {
		Common::String macString = s.encode(Common::kMacRoman);
		Graphics::MacFontRun fontRun(&g_engine->_wm, _port->txFont, _port->txFace, _port->txSize, 0, 0, 0);
		uint16 result = fontRun.getFont()->getStringWidth(macString);
		debug(5, "Toolbox::StringWidth: %s -> %d", s.encode().c_str(), result);
		return result;
	}
	return 0;
}

void Toolbox::TextFace(uint16 style) {
	if (_port) {
		_port->txFace = style;
	}
}

void Toolbox::TextFont(uint16 font) {
	if (_port) {
		_port->txFont = font;
	}
}

void Toolbox::TextMode(SourceMode mode) {
	if (_port) {
		_port->txMode = mode;
	}
}

void Toolbox::TextSize(uint16 size) {
	if (_port) {
		_port->txSize = size;
	}
}

} // namespace Fool
