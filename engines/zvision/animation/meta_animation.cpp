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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/animation/meta_animation.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/animation/rlf_animation.h"
#include "zvision/video/zork_avi_decoder.h"

#include "video/video_decoder.h"

#include "graphics/surface.h"

namespace ZVision {

MetaAnimation::MetaAnimation(const Common::String &fileName, ZVision *engine)
	: _fileType(RLF),
	  _curFrame(NULL) {
	Common::String tmpFileName = fileName;
	tmpFileName.toLowercase();
	if (tmpFileName.hasSuffix(".rlf")) {
		_fileType = RLF;
		Common::File *_file = engine->getSearchManager()->openFile(tmpFileName);
		_animation.rlf = new RLFDecoder(_file, false);
		_frmDelay = _animation.rlf->frameTime();
	} else if (tmpFileName.hasSuffix(".avi")) {
		_fileType = AVI;
		Common::File *_file = engine->getSearchManager()->openFile(tmpFileName);
		_animation.avi = new ZorkAVIDecoder();
		_animation.avi->loadStream(_file);
		_frmDelay = 1000.0 / _animation.avi->getDuration().framerate();
	} else {
		warning("Unrecognized animation file type: %s", fileName.c_str());
	}
}

MetaAnimation::~MetaAnimation() {
	if (_fileType == RLF) {
		delete _animation.rlf;
	} else if (_fileType == AVI) {
		delete _animation.avi;
	}
}

uint MetaAnimation::frameCount() {
	if (_fileType == RLF) {
		return _animation.rlf->frameCount();
	} else
		return _animation.avi->getFrameCount();

}

uint MetaAnimation::width() {
	if (_fileType == RLF) {
		return _animation.rlf->width();
	} else
		return _animation.avi->getWidth();
}
uint MetaAnimation::height() {
	if (_fileType == RLF) {
		return _animation.rlf->height();
	} else
		return _animation.avi->getHeight();
}
uint32 MetaAnimation::frameTime() {
	return _frmDelay;
}

void MetaAnimation::seekToFrame(int frameNumber) {
	if (frameNumber >= (int)frameCount())
		frameNumber = frameCount() - 1;

	if (_fileType == RLF) {
		_animation.rlf->seekToFrame(frameNumber);
	} else
		_animation.avi->seekToFrame(frameNumber);
}

const Graphics::Surface *MetaAnimation::decodeNextFrame() {
	if (_fileType == RLF)
		_curFrame = _animation.rlf->decodeNextFrame();
	else
		_curFrame = _animation.avi->decodeNextFrame();

	return _curFrame;
}

const Graphics::Surface *MetaAnimation::getFrameData(uint frameNumber) {
	if (frameNumber >= frameCount())
		frameNumber = frameCount() - 1;

	if (_fileType == RLF) {
		_curFrame = _animation.rlf->getFrameData(frameNumber);
		return _curFrame;
	} else {
		_animation.avi->seekToFrame(frameNumber);
		_curFrame = _animation.avi->decodeNextFrame();
		return _curFrame;
	}
}

bool MetaAnimation::endOfAnimation() {
	if (_fileType == RLF) {
		return _animation.rlf->endOfAnimation();
	} else
		return _animation.avi->endOfVideo();
}

} // End of namespace ZVision
