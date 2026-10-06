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

#ifndef PHOENIXVR_SUBTITLES_H
#define PHOENIXVR_SUBTITLES_H

#include "common/path.h"
#include "video/subtitles.h"

namespace Graphics {
class ManagedSurface;
}

namespace PhoenixVR {

class Subtitles : public Video::Subtitles {
public:
	explicit Subtitles(Graphics::ManagedSurface *target);
	~Subtitles() override;

	void setBBox(const Common::Rect &bbox);
	void setColor(byte r, byte g, byte b);
	void drawSubtitle(uint32 timestamp);
	void clearSubtitle() const override;

protected:
	void drawSubtitleText(const Graphics::Font &font, const Common::U32String &text, int x, int y, int width) const override;
	void updateSubtitleOverlay() const override {}

private:
	struct TextRun {
		const Graphics::Font *font;
		Common::U32String text;
		int x, y, width;
	};
	void renderOutline();

	static const int kOutlineWidth = 3;
	Graphics::ManagedSurface *_target;
	mutable Graphics::Surface _outlined;
	Common::Rect _bbox;
	mutable Common::Rect _bounds;
	mutable Common::Array<TextRun> _runs;
	uint32 _textColor = 0;
};

} // namespace PhoenixVR

#endif
