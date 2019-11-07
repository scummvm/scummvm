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

#include "common/debug.h"
#include "common/rect.h"
#include "common/system.h"

#include "toon/subtitles.h"

namespace Toon {
SubtitleRenderer::SubtitleRenderer(ToonEngine *vm) : _vm(vm) {
	_subSurface = new Graphics::Surface();
	_subSurface->create(TOON_SCREEN_WIDTH, TOON_SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_hasSubtitles = false;
}

SubtitleRenderer::~SubtitleRenderer() {
}


void SubtitleRenderer::render(const Graphics::Surface &frame, uint32 frameNumber, byte color) {
	if (!_hasSubtitles || _tw.empty()) {
		return;
	}

	_subSurface->copyFrom(frame);
	// char strf[384] = {0};
	// sprintf(strf, "Time passed: %d", frameNumber);
	// _vm->drawCostumeLine(0, 0, strf, _subSurface);
	// _vm->_system->copyRectToScreen(_subSurface->getBasePtr(0, 0), _subSurface->pitch, 0, 0, _subSurface->w,  _subSurface->h);

	if (frameNumber > _tw.front()._endFrame) {
		_tw.pop_front();
		if (_tw.empty()) {
			return;
		}
	}

	if (frameNumber < _tw.front()._startFrame) {
		return;
	}

	_vm->drawCustomText(TOON_SCREEN_WIDTH / 2, TOON_SCREEN_HEIGHT, _tw.front()._text.c_str(), _subSurface, color);
	_vm->_system->copyRectToScreen(_subSurface->getBasePtr(0, 0), _subSurface->pitch, 0, 0, _subSurface->w,  _subSurface->h);
}

bool SubtitleRenderer::load(const Common::String &video) {
	// warning(video.c_str());

	_hasSubtitles = false;

	Common::String subfile(video);
	Common::String ext("tss");
	subfile.replace(subfile.size() - ext.size(), ext.size(), ext);

	Common::SeekableReadStream *file = _vm->resources()->openFile(subfile);
	if (!file) {
		return false;
	}

	Common::String line;
	int lineNo = 0;

	_tw.clear();
	while (!file->eos() && !file->err()) {
		line = file->readLine();

		lineNo++;
		if (line.empty() || line[0] == '#') {
			continue;
		}

		const char *ptr = line.c_str();

		int startFrame = strtoul(ptr, const_cast<char **>(&ptr), 10);
		int endFrame = strtoul(ptr, const_cast<char **>(&ptr), 10);

		while (*ptr && Common::isSpace(*ptr))
			ptr++;

		if (startFrame > endFrame) {
			warning("%s:%d: startFrame (%d) > endFrame (%d)", subfile.c_str(), lineNo, startFrame, endFrame);
			continue;
		}

		_tw.push_back(TimeWindow(startFrame, endFrame, ptr));
	}

	_hasSubtitles = true;
	return _hasSubtitles;
}

}
