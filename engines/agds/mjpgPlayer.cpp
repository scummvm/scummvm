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

#include "agds/mjpgPlayer.h"
#include "agds/agds.h"
#include "agds/font.h"
#include "agds/object.h"
#include "agds/systemVariable.h"
#include "common/debug.h"

namespace AGDS {

MJPGPlayer::MJPGPlayer(Common::SeekableReadStream *stream, const Common::String &subtitles) :
	_stream(stream), _firstFramePos(_stream->pos()), _framesPlayed(0), _nextSubtitleIndex(0) {
	uint pos = 0;
	while(pos < subtitles.size()) {
		auto next = subtitles.find('\n', pos);
		if (next == subtitles.npos)
			next = subtitles.size();
		Common::String line = subtitles.substr(pos, next - pos);

		uint begin, end;
		int offset;
		if (sscanf(line.c_str(), "{%u}{%u}%n", &begin, &end, &offset) != 2) {
			warning("malformed subtitle line: %s", line.c_str());
			pos = next + 1;
			continue;
		}
		while(offset < static_cast<int>(line.size()) && line[offset] == ' ')
			++offset;

		line = line.substr(offset);
		debug("subtitle line: %d-%d: %s", begin, end, line.c_str());
		_subtitles.push_back({begin, end, line});
		pos = next + 1;
	}
}

MJPGPlayer::~MJPGPlayer() {
	delete _stream;
}

void MJPGPlayer::rewind() {
	_framesPlayed = 0;
	_nextSubtitleIndex = 0;
	_stream->seek(_firstFramePos);
}

const Graphics::Surface *MJPGPlayer::decodeFrame() {
	if (_stream->eos())
		return NULL;

	uint32 size = _stream->readSint32LE();
	if (size == 0)
		return NULL;

	if (_stream->eos())
		return NULL;
	Common::SeekableReadStream *stream = _stream->readStream(size);
	const Graphics::Surface *surface = _decoder.decodeFrame(*stream);
	++_framesPlayed;
	delete stream;
	return surface;
}

void MJPGPlayer::paint(AGDSEngine &engine, Graphics::Surface &backbuffer) {
	auto *surface = decodeFrame();
	if (surface) {
		Graphics::Surface *converted = surface->convertTo(engine.pixelFormat());
		Common::Point dst((backbuffer.w - converted->w) / 2, (backbuffer.h - converted->h) / 2);
		Common::Rect srcRect(converted->getRect());
		if (Common::Rect::getBlitRect(dst, srcRect, backbuffer.getRect()))
			backbuffer.copyRectToSurface(*converted, dst.x, dst.y, srcRect);
		converted->free();
		delete converted;
	}

	if (_subtitles.empty())
		return;

	while(_nextSubtitleIndex < _subtitles.size() && _framesPlayed > _subtitles[_nextSubtitleIndex].end)
		++_nextSubtitleIndex;

	if (_nextSubtitleIndex >= _subtitles.size() || _framesPlayed < _subtitles[_nextSubtitleIndex].begin)
		return;

	auto font = engine.getFont(engine.getSystemVariable("tell_font")->getInteger());
	int x = engine.getSystemVariable("subtitle_x")->getInteger();
	int y = engine.getSystemVariable("subtitle_y")->getInteger();

	auto & text = _subtitles[_nextSubtitleIndex].line;
	x -= font->getStringWidth(text) / 2;
	font->drawString(&backbuffer, text, x, y, backbuffer.w, 0);
}


} // namespace AGDS
