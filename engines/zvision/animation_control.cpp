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

#include "video/video_decoder.h"

#include "zvision/animation_control.h"
#include "zvision/zvision.h"
#include "zvision/render_manager.h"
#include "zvision/script_manager.h"
#include "zvision/rlf_animation.h"
#include "zvision/zork_avi_decoder.h"


namespace ZVision {

AnimationControl::AnimationControl(ZVision *engine, uint32 controlKey, const Common::String &fileName)
		: Control(engine, controlKey), 
		  _fileType(RLF),
		  _loopCount(1),
		  _currentLoop(0),
		  _accumulatedTime(0) {
	if (fileName.hasSuffix(".rlf")) {
		_fileType = RLF;
		_animation.rlf = new RlfAnimation(fileName, false);
	} else if (fileName.hasSuffix(".avi")) {
		_fileType = AVI;
		_animation.avi = new ZorkAVIDecoder();
		_animation.avi->loadFile(fileName);
	} else {
		warning("Unrecognized animation file type: %s", fileName.c_str());
	}
}

AnimationControl::~AnimationControl() {
	if (_fileType == RLF) {
		delete _animation.rlf;
	} else if (_fileType == AVI) {
		delete _animation.avi;
	}
}

bool AnimationControl::process(uint32 deltaTimeInMillis) {
	if (!_enabled) {
		return false;
	}

	bool finished = false;

	if (_fileType == RLF) {
		_accumulatedTime += deltaTimeInMillis;

		uint32 frameTime = _animation.rlf->frameTime();
		while (_accumulatedTime >= frameTime) {
			_accumulatedTime -= frameTime;

			RenderManager *renderManager = _engine->getRenderManager();
			Common::Point workingWindowPoint = renderManager->imageSpaceToWorkingWindowSpace(Common::Point(_x, _y));
			Common::Rect subRect(workingWindowPoint.x, workingWindowPoint.y, workingWindowPoint.x + _animation.rlf->width(), workingWindowPoint.y + _animation.rlf->height());

			// If the clip returns false, it means the animation is outside the working window
			if (!renderManager->clipRectToWorkingWindow(subRect)) {
				return false;
			}

			const Graphics::Surface *frame = _animation.rlf->getNextFrame();

			RenderTable::RenderState state = renderManager->getRenderTable()->getRenderState();
			if (state == RenderTable::PANORAMA) {
				Graphics::Surface *tranposedFrame = RenderManager::tranposeSurface(frame);

				renderManager->copyRectToWorkingWindow((uint16 *)tranposedFrame->getBasePtr(tranposedFrame->w - subRect.width(), tranposedFrame->h - subRect.height()), subRect.left, subRect.top, _animation.rlf->width(), subRect.width(), subRect.height());

				tranposedFrame->free();
				delete tranposedFrame;
			} else {
				renderManager->copyRectToWorkingWindow((uint16 *)frame->getBasePtr(frame->w - subRect.width(), frame->h - subRect.height()), subRect.left, subRect.top, _animation.rlf->width(), subRect.width(), subRect.height());
			}

			if (_animation.rlf->endOfAnimation()) {
				_animation.rlf->seekToFrame(-1);
				if (_loopCount > 0) {
					_currentLoop++;
					if (_currentLoop >= _loopCount) {
						finished = true;
					}
				}
			}
		}
	} else if (_fileType == AVI) {
		if (!_animation.avi->isPlaying()) {
			_animation.avi->start();
		}

		if (_animation.avi->needsUpdate()) {
			const Graphics::Surface *frame = _animation.avi->decodeNextFrame();

			if (frame) {
				RenderManager *renderManager = _engine->getRenderManager();
				Common::Point workingWindowPoint = renderManager->imageSpaceToWorkingWindowSpace(Common::Point(_x, _y));
				Common::Rect subRect(workingWindowPoint.x, workingWindowPoint.y, workingWindowPoint.x + frame->w, workingWindowPoint.y + frame->h);

				// If the clip returns false, it means the animation is outside the working window
				if (!renderManager->clipRectToWorkingWindow(subRect)) {
					return false;
				}

				RenderTable::RenderState state = renderManager->getRenderTable()->getRenderState();
				if (state == RenderTable::PANORAMA) {
					Graphics::Surface *tranposedFrame = RenderManager::tranposeSurface(frame);

					renderManager->copyRectToWorkingWindow((uint16 *)tranposedFrame->getBasePtr(tranposedFrame->w - subRect.width(), tranposedFrame->h - subRect.height()), subRect.left, subRect.top, frame->w, subRect.width(), subRect.height());

					tranposedFrame->free();
					delete tranposedFrame;
				} else {
					renderManager->copyRectToWorkingWindow((uint16 *)frame->getBasePtr(frame->w - subRect.width(), frame->h - subRect.height()), subRect.left, subRect.top, frame->w, subRect.width(), subRect.height());
				}

				
			}
		}

		if (_animation.avi->endOfVideo()) {
			_animation.avi->rewind();
			if (_loopCount > 0) {
				_currentLoop++;
				if (_currentLoop >= _loopCount) {
					_animation.avi->stop();
					finished = true;
				}
			}
		}
	}

	if (finished) {
		_engine->getScriptManager()->setStateValue(_animationKey, 2);
		disable();
		_currentLoop = 0;
	}

	return false;
}

} // End of namespace ZVision
