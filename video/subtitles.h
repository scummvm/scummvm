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

#ifndef VIDEO_SUBTITLES_H
#define VIDEO_SUBTITLES_H

#include "common/str.h"
#include "common/array.h"
#include "common/rect.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Video {

struct SRTEntry {
	uint seq;
	uint32 start;
	uint32 end;

	Common::String text;

	SRTEntry(uint seq_, uint32 start_, uint32 end_, Common::String text_) {
		seq = seq_; start = start_; end = end_; text = text_;
	}
};

class SRTParser {
public:
	SRTParser();
	~SRTParser();

	void cleanup();
	bool parseFile(const char *fname);
	Common::String getSubtitle(uint32 timestamp) const;

private:
	Common::Array<SRTEntry *> _entries;
};

class Subtitles {
public:
	Subtitles();
	~Subtitles();

	void loadSRTFile(const char *fname);
	void close() { _loaded = false; _subtitle.clear(); _fname.clear(); _srtParser.cleanup(); }
	void setFont(const char *fontname, int height = 18);
	void setBBox(const Common::Rect bbox);
	void setColor(byte r, byte g, byte b);
	void setPadding(uint16 horizontal, uint16 vertical);
	bool drawSubtitle(uint32 timestamp, bool force = false) const;
	bool isLoaded() const { return _loaded || _subtitleDev; }

private:
	void renderSubtitle() const;

	SRTParser _srtParser;
	bool _loaded;
	bool _subtitleDev;
	bool _overlayHasAlpha;

	const Graphics::Font *_font;
	int _fontHeight;

	Graphics::Surface *_surface;

	mutable Common::Rect _drawRect;
	Common::Rect _requestedBBox;
	mutable Common::Rect _realBBox;
	mutable int16 _lastOverlayWidth, _lastOverlayHeight;

	Common::String _fname;
	mutable Common::String _subtitle;
	uint32 _color;
	uint32 _blackColor;
	uint32 _transparentColor;
	uint16 _hPad;
	uint16 _vPad;
};

} // End of namespace Video

#endif
