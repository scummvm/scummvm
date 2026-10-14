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

#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "hollywood/runaway/video.h"

namespace Hollywood {

bool RunawayVideo::load(const char *name) {
	close();
	Common::File file;
	if (!file.open(Common::Path(name)) || file.size() < 1024 || file.size() > 256 * 1024 * 1024)
		return false;
	_data.resize(file.size());
	if (file.read(_data.data(), _data.size()) != _data.size())
		return false;
	if (READ_BE_UINT32(_data.data()) != MKTAG('R', 'I', 'F', 'F')) {
		// The native player (0x41ac70) temporarily rewrites the first 1024
		// bytes using DATAVC00.001. Restore them only in our memory copy.
		// Match the stored prefix: the Spanish codebook uses different indices.
		Common::File codebook;
		if (!codebook.open("DATAVC00.001"))
			return false;
		byte entry[2048];
		bool found = false;
		while (codebook.read(entry, sizeof(entry)) == sizeof(entry)) {
			if (memcmp(entry + 1024, _data.data(), 1024))
				continue;
			for (uint i = 0; i < 1024; ++i)
				_data[i] = entry[i] - entry[1024 + i];
			found = true;
			break;
		}
		if (!found)
			return false;
	}
	if (READ_BE_UINT32(_data.data()) != MKTAG('R', 'I', 'F', 'F') ||
			READ_BE_UINT32(_data.data() + 8) != MKTAG('A', 'V', 'I', ' '))
		return false;
	if (!_decoder.loadStream(new Common::MemoryReadStream(_data.data(), _data.size())) ||
			_decoder.getAudioTrackCount() || !_decoder.getFrameCount()) {
		close();
		return false;
	}
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	if (!_decoder.setOutputPixelFormat(format) || !decodeToFrame(0)) {
		close();
		return false;
	}
	return true;
}

void RunawayVideo::close() {
	_decoder.close();
	_frame = nullptr;
	_data.clear();
}

bool RunawayVideo::decodeToFrame(uint frame) {
	if (frame >= _decoder.getFrameCount())
		return false;
	while (_decoder.getCurFrame() < (int)frame && !_decoder.endOfVideo()) {
		const Graphics::Surface *decoded = _decoder.decodeNextFrame();
		if (decoded)
			_frame = decoded;
		else {
			warning("Runaway: unable to decode video frame %d", _decoder.getCurFrame());
			return false;
		}
	}
	return _frame != nullptr && _decoder.getCurFrame() == (int)frame;
}

void RunawayVideo::draw(Graphics::ManagedSurface &surface, const Common::Rect &destination) const {
	if (_frame)
		surface.blitFrom(*_frame, Common::Rect(_frame->w, _frame->h), destination);
}

} // namespace Hollywood
