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

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/controls/fist_control.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/video/rlf_decoder.h"

#include "common/stream.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace ZVision {

FistControl::FistControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_FIST) {
	_cursor = CursorIndex_Idle;
	_animation = NULL;
	_soundKey = 0;
	_fiststatus = 0;
	_order = 0;
	_fistnum = 0;

	_animationId = 0;

	clearFistArray(_fistsUp);
	clearFistArray(_fistsDwn);

	_numEntries = 0;
	_entries.clear();

	_anmRect = Common::Rect();

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("sound_key", true)) {
			_soundKey = atoi(values.c_str());
		} else if (param.matchString("cursor", true)) {
			_cursor = _engine->getCursorManager()->getCursorId(values);
		} else if (param.matchString("descfile", true)) {
			readDescFile(values);
		} else if (param.matchString("animation_id", true)) {
			_animationId = atoi(values.c_str());
		} else if (param.matchString("venus_id", true)) {
			_venusId = atoi(values.c_str());
		}

		line = stream.readLine();
		_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}
}

FistControl::~FistControl() {
	if (_animation)
		delete _animation;

	clearFistArray(_fistsUp);
	clearFistArray(_fistsDwn);
	_entries.clear();
}

bool FistControl::process(uint32 deltaTimeInMillis) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_animation && _animation->isPlaying()) {
		if (_animation->endOfVideo()) {
			_animation->stop();
			_engine->getScriptManager()->setStateValue(_animationId, 2);
			return false;
		}

		if (_animation->needsUpdate()) {
			const Graphics::Surface *frameData = _animation->decodeNextFrame();
			if (frameData)
				// WORKAROUND: Ignore the target frame dimensions for the finger animations.
				// The target dimensions specify an area smaller than expected, thus if we
				// scale the finger videos to fit these dimensions, they are not aligned
				// correctly. Not scaling these videos yields a result identical to the
				// original. Fixes bug #6784.
				_engine->getRenderManager()->blitSurfaceToBkg(*frameData, _anmRect.left, _anmRect.top);
		}
	}

	return false;
}

bool FistControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (mouseIn(screenSpacePos, backgroundImageSpacePos) >= 0) {
		_engine->getCursorManager()->changeCursor(_cursor);
		return true;
	}

	return false;
}

bool FistControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	int fistNumber = mouseIn(screenSpacePos, backgroundImageSpacePos);

	if (fistNumber >= 0) {
		setVenus();

		uint32 oldStatus = _fiststatus;
		_fiststatus ^= (1 << fistNumber);

		for (int i = 0; i < _numEntries; i++)
			if (_entries[i]._bitsStrt == oldStatus && _entries[i]._bitsEnd == _fiststatus) {
				if (_animation) {
					_animation->stop();
					_animation->seekToFrame(_entries[i]._anmStrt);
					_animation->setEndFrame(_entries[i]._anmEnd);
					_animation->start();
				}

				_engine->getScriptManager()->setStateValue(_animationId, 1);
				_engine->getScriptManager()->setStateValue(_soundKey, _entries[i]._sound);
				break;
			}

		_engine->getScriptManager()->setStateValue(_key, _fiststatus);
	}

	return false;
}

void FistControl::readDescFile(const Common::String &fileName) {
	Common::File file;
	if (!_engine->getSearchManager()->openFile(file, fileName)) {
		warning("Desc file %s could could be opened", fileName.c_str());
		return;
	}

	Common::String line;
	Common::String param;
	Common::String values;

	while (!file.eos()) {
		line = file.readLine();
		getFistParams(line, param, values);

		if (param.matchString("animation_id", true)) {
			// Not used
		} else if (param.matchString("animation", true)) {
			_animation = _engine->loadAnimation(values);
		} else if (param.matchString("anim_rect", true)) {
			int left, top, right, bottom;
			sscanf(values.c_str(), "%d %d %d %d", &left, &top, &right, &bottom);
			_anmRect = Common::Rect(left, top, right, bottom);
		} else if (param.matchString("num_fingers", true)) {
			sscanf(values.c_str(), "%d", &_fistnum);
			_fistsUp.resize(_fistnum);
			_fistsDwn.resize(_fistnum);
		} else if (param.matchString("entries", true)) {
			sscanf(values.c_str(), "%d", &_numEntries);
			_entries.resize(_numEntries);
		} else if (param.matchString("eval_order_ascending", true)) {
			sscanf(values.c_str(), "%d", &_order);
		} else if (param.matchString("up_hs_num_*", true)) {
			int fist, num;
			num = atoi(values.c_str());

			sscanf(param.c_str(), "up_hs_num_%d", &fist);
			_fistsUp[fist].resize(num);
		} else if (param.matchString("up_hs_*", true)) {
			int16 fist, box, x1, y1, x2, y2;
			sscanf(param.c_str(), "up_hs_%hd_%hd", &fist, &box);
			sscanf(values.c_str(), "%hd %hd %hd %hd", &x1, &y1, &x2, &y2);
			(_fistsUp[fist])[box] = Common::Rect(x1, y1, x2, y2);
		} else if (param.matchString("down_hs_num_*", true)) {
			int fist, num;
			num = atoi(values.c_str());

			sscanf(param.c_str(), "down_hs_num_%d", &fist);
			_fistsDwn[fist].resize(num);
		} else if (param.matchString("down_hs_*", true)) {
			int16 fist, box, x1, y1, x2, y2;
			sscanf(param.c_str(), "down_hs_%hd_%hd", &fist, &box);
			sscanf(values.c_str(), "%hd %hd %hd %hd", &x1, &y1, &x2, &y2);
			(_fistsDwn[fist])[box] = Common::Rect(x1, y1, x2, y2);
		} else {
			int  entry, start, end, sound;
			char bitsStart[33];
			char bitsEnd[33];
			entry = atoi(param.c_str());
			if (sscanf(values.c_str(), "%s %s %d %d (%d)", bitsStart, bitsEnd, &start, &end, &sound) == 5) {
				_entries[entry]._bitsStrt = readBits(bitsStart);
				_entries[entry]._bitsEnd = readBits(bitsEnd);
				_entries[entry]._anmStrt = start;
				_entries[entry]._anmEnd = end;
				_entries[entry]._sound = sound;
			}
		}
	}
	file.close();
}

void FistControl::clearFistArray(Common::Array< Common::Array<Common::Rect> > &arr) {
	for (uint i = 0; i < arr.size(); i++)
		arr[i].clear();

	arr.clear();
}

uint32 FistControl::readBits(const char *str) {
	uint32 bfield = 0;
	int len = strlen(str);
	for (int i = 0; i < len; i++)
		if (str[i] != '0')
			bfield |= (1 << i);
	return bfield;
}

int FistControl::mouseIn(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_order) {
		for (int i = 0; i < _fistnum; i++) {
			if (((_fiststatus >> i) & 1) == 1) {
				for (uint j = 0; j < _fistsDwn[i].size(); j++)
					if ((_fistsDwn[i])[j].contains(backgroundImageSpacePos))
						return i;
			} else {
				for (uint j = 0; j < _fistsUp[i].size(); j++)
					if ((_fistsUp[i])[j].contains(backgroundImageSpacePos))
						return i;
			}
		}
	} else {
		for (int i = _fistnum - 1; i >= 0; i--) {
			if (((_fiststatus >> i) & 1) == 1) {
				for (uint j = 0; j < _fistsDwn[i].size(); j++)
					if ((_fistsDwn[i])[j].contains(backgroundImageSpacePos))
						return i;
			} else {
				for (uint j = 0; j < _fistsUp[i].size(); j++)
					if ((_fistsUp[i])[j].contains(backgroundImageSpacePos))
						return i;
			}
		}
	}
	return -1;
}

void FistControl::getFistParams(const Common::String &inputStr, Common::String &parameter, Common::String &values) {
	const char *chrs = inputStr.c_str();
	uint lbr;

	for (lbr = 0; lbr < inputStr.size(); lbr++)
		if (chrs[lbr] == ':')
			break;

	if (lbr >= inputStr.size())
		return;

	uint rbr;

	for (rbr = lbr + 1; rbr < inputStr.size(); rbr++)
		if (chrs[rbr] == '~')
			break;

	if (rbr >= inputStr.size())
		return;

	parameter = Common::String(chrs, chrs + lbr);
	values = Common::String(chrs + lbr + 1, chrs + rbr);
}

} // End of namespace ZVision
