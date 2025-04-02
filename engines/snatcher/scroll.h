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

#ifndef SNATCHER_SCROLL_H
#define SNATCHER_SCROLL_H

//#include "common/platform.h"
//#include "snatcher/graphics.h"
//#include "snatcher/resource.h"
#include "common/scummsys.h"

namespace Snatcher {

class ScrollManager {
public:
	enum {
		kHorzA = 0,
		kVertA = 1,
		kHorzB = 2,
		kVertB = 3
	};

	struct ScrollState {
		int16 offsets[4];
		const int16 *hScrollTable;
		uint16 hScrollTableNumEntries;
		bool disableVScroll;
		bool lineScrollMode;
		bool hINTEnable;
		int hINTCounter;
		void *hINTProc;
	};

	ScrollManager() {
		memset(&_result, 0, sizeof(_result));
	}

	virtual ~ScrollManager() {}
	virtual void doCommand(int cmd) = 0;
	virtual void setSingleStep(int mode, int16 step) = 0;
	virtual void setDirectionAndSpeed(int mode, int16 incr) = 0;
	virtual void clear() = 0;
	virtual bool nextFrame() = 0;

	const ScrollState &getState() const {
		return _result;
	}

protected:
	ScrollState _result;

private:
	static ScrollManager *createSegaScrollManager(GraphicsEngine::GfxState &state);

public:
	static ScrollManager *create(Common::Platform platform, GraphicsEngine::GfxState &state) {
		switch (platform) {
		case Common::kPlatformSegaCD:
			return createSegaScrollManager(state);
		default:
			break;
		};
		return 0;
	}
};

} // End of namespace Snatcher

#endif // SNATCHER_RENDER_H
