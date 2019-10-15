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
	if (!_hasSubtitles || _index > _last) {
		return;
	}

	_subSurface->copyFrom(frame);
	// char strf[384] = {0};
	// sprintf(strf, "Time passed: %d", frameNumber);
	// _vm->drawCostumeLine(0, 0, strf, _subSurface);
	// _vm->_system->copyRectToScreen(_subSurface->getBasePtr(0, 0), _subSurface->pitch, 0, 0, _subSurface->w,  _subSurface->h);

	if (frameNumber > _tw[_index].fend) {
		_index++;
		if (_index > _last) {
			return;
		}
		_currentLine = (char *)_fileData + _tw[_index].foffset;
	}

	if (frameNumber < _tw[_index].fstart) {
		return;
	}

	_vm->drawCustomText(TOON_SCREEN_WIDTH / 2, TOON_SCREEN_HEIGHT, _currentLine, _subSurface, color);
	_vm->_system->copyRectToScreen(_subSurface->getBasePtr(0, 0), _subSurface->pitch, 0, 0, _subSurface->w,  _subSurface->h);
}

bool SubtitleRenderer::load(const Common::String &video) {
	// warning(video.c_str());

	_hasSubtitles = false;
	_index = 0;

	Common::String subfile(video);
	Common::String ext("tss");
	subfile.replace(subfile.size() - ext.size(), ext.size(), ext);

	uint32 fileSize = 0;
	uint8 *fileData = _vm->resources()->getFileData(subfile, &fileSize);
	if (!fileData) {
		return false;
	}

	uint32 numOflines = *((uint32 *) fileData);
	uint32 idx_size = numOflines * sizeof(TimeWindow);
	memcpy(_tw, sizeof(numOflines) + fileData, idx_size);
	_fileData = sizeof(numOflines) + fileData + idx_size;
	_last = numOflines - 1;

	_currentLine = (char *)_fileData + _tw[0].foffset;
	_hasSubtitles = true;
	return _hasSubtitles;
}

}
