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

#include "common/scummsys.h"

#include "zvision/scripting/controls/hotmov_control.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"

#include "common/stream.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace ZVision {

HotMovControl::HotMovControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_HOTMOV) {
	_animation = NULL;
	_cycle = 0;
	_frames.clear();
	_cyclesCount = 0;
	_framesCount = 0;

	_engine->getScriptManager()->setStateValue(_key, 0);

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("hs_frame_list", true)) {
			readHsFile(values);
		} else if (param.matchString("rectangle", true)) {
			int x;
			int y;
			int width;
			int height;

			sscanf(values.c_str(), "%d %d %d %d", &x, &y, &width, &height);

			_rectangle = Common::Rect(x, y, width, height);
		} else if (param.matchString("num_frames", true)) {
			_framesCount = atoi(values.c_str());
		} else if (param.matchString("num_cycles", true)) {
			_cyclesCount = atoi(values.c_str());
		} else if (param.matchString("animation", true)) {
			char filename[64];
			sscanf(values.c_str(), "%s", filename);
			values = Common::String(filename);
			_animation = _engine->loadAnimation(values);
			_animation->start();
		} else if (param.matchString("venus_id", true)) {
			_venusId = atoi(values.c_str());
		}

		line = stream.readLine();
		_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}
}

HotMovControl::~HotMovControl() {
	if (_animation)
		delete _animation;

	_frames.clear();
}

bool HotMovControl::process(uint32 deltaTimeInMillis) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_cycle < _cyclesCount) {
		if (_animation && _animation->endOfVideo()) {
			_cycle++;

			if (_cycle == _cyclesCount) {
				_engine->getScriptManager()->setStateValue(_key, 2);
				return false;
			}

			_animation->rewind();
		}

		if (_animation && _animation->needsUpdate()) {
			const Graphics::Surface *frameData = _animation->decodeNextFrame();
			if (frameData)
				_engine->getRenderManager()->blitSurfaceToBkgScaled(*frameData, _rectangle);
		}
	}

	return false;
}

bool HotMovControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (!_animation)
		return false;

	if (_cycle < _cyclesCount) {
		if (_frames[_animation->getCurFrame()].contains(backgroundImageSpacePos)) {
			_engine->getCursorManager()->changeCursor(CursorIndex_Active);
			return true;
		}
	}

	return false;
}

bool HotMovControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (!_animation)
		return false;

	if (_cycle < _cyclesCount) {
		if (_frames[_animation->getCurFrame()].contains(backgroundImageSpacePos)) {
			setVenus();
			_engine->getScriptManager()->setStateValue(_key, 1);
			return true;
		}
	}

	return false;
}

void HotMovControl::readHsFile(const Common::String &fileName) {
	if (_framesCount == 0)
		return;

	Common::File file;
	if (!_engine->getSearchManager()->openFile(file, fileName)) {
		warning("HS file %s could could be opened", fileName.c_str());
		return;
	}

	Common::String line;

	_frames.resize(_framesCount);

	while (!file.eos()) {
		line = file.readLine();

		int frame;
		int x;
		int y;
		int width;
		int height;

		sscanf(line.c_str(), "%d:%d %d %d %d~", &frame, &x, &y, &width, &height);

		if (frame >= 0 && frame < _framesCount)
			_frames[frame] = Common::Rect(x, y, width, height);
	}
	file.close();
}

} // End of namespace ZVision
