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

#include "phoenixvr/subtitles.h"

#include "graphics/font.h"
#include "graphics/managed_surface.h"

namespace PhoenixVR {

Subtitles::Subtitles(Graphics::ManagedSurface *target) : _target(target) {
}

Subtitles::~Subtitles() {
	_outlined.free();
}

void Subtitles::setBBox(const Common::Rect &bbox) {
	Video::Subtitles::setBBox(bbox);
	_bbox = bbox;
	_outlined.create(bbox.width() + 2 * kOutlineWidth, bbox.height() + 2 * kOutlineWidth,
					 Graphics::PixelFormat::createFormatRGBA32());
	clearSubtitle();
}

void Subtitles::setColor(byte r, byte g, byte b) {
	Video::Subtitles::setColor(r, g, b);
	_textColor = _outlined.format.ARGBToColor(255, r, g, b);
}

void Subtitles::clearSubtitle() const {
	_parts = nullptr;
	_bounds.setEmpty();
	_runs.clear();
}

void Subtitles::drawSubtitleText(const Graphics::Font &font, const Common::U32String &text, int x, int y, int width) const {
	_runs.push_back(TextRun{&font, text, x, y, width});
}

void Subtitles::drawSubtitle(uint32 timestamp) {
	_runs.clear();
	Video::Subtitles::drawSubtitle(timestamp);
	if (!_runs.empty())
		renderOutline();
	if (!_parts || _parts->empty() || (*_parts)[0].tag == "sfx" || _bounds.isEmpty())
		return;

	const Common::Point position(_bbox.left - kOutlineWidth + _bounds.left,
								 _bbox.top - kOutlineWidth + _bounds.top);
	_target->simpleBlitFrom(_outlined, _bounds, position, Graphics::FLIP_NONE, true);
}

void Subtitles::renderOutline() {
	_outlined.fillRect(Common::Rect(_outlined.w, _outlined.h), 0);
	_bounds.setEmpty();
	const uint32 black = _outlined.format.ARGBToColor(255, 0, 0, 0);

	for (const auto &run : _runs) {
		const Common::Rect bounds(run.x, run.y, run.x + run.width + 2 * kOutlineWidth,
								  run.y + run.font->getFontHeight() + 2 * kOutlineWidth);
		if (_bounds.isEmpty())
			_bounds = bounds;
		else
			_bounds.extend(bounds);
		for (int y = -kOutlineWidth; y <= kOutlineWidth; ++y) {
			for (int x = -kOutlineWidth; x <= kOutlineWidth; ++x) {
				if ((x || y) && 4 * (x * x + y * y) <= (2 * kOutlineWidth + 1) * (2 * kOutlineWidth + 1))
					run.font->drawString(&_outlined, run.text, run.x + kOutlineWidth + x, run.y + kOutlineWidth + y,
										 run.width, black, Graphics::kTextAlignLeft);
			}
		}
	}
	for (const auto &run : _runs)
		run.font->drawString(&_outlined, run.text, run.x + kOutlineWidth, run.y + kOutlineWidth,
							 run.width, _textColor, Graphics::kTextAlignLeft);
	_bounds.clip(_outlined.w, _outlined.h);
}

} // namespace PhoenixVR
