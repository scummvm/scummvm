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

#ifndef BCB_H
#define BCB_H

#include "audio/mixer.h"
#include "common/archive.h"
#include "common/array.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/rect.h"
#include "common/str-enc.h"
#include "common/ustr.h"
#include "engines/engine.h"
#include "harold/detection.h"
#include "harold/bcb/ui.h"

namespace Graphics {
class Font;
class FontSJIS;
struct Surface;
}

namespace Video {
class AVIDecoder;
}

namespace BCB {

enum {
	kBCBDebugGeneral = 1
};

class Debugger;

class BCBEngine : public Engine {
	friend class Debugger;

public:
	BCBEngine(OSystem *syst, const HaroldGameDescription *desc);
	~BCBEngine();

	Common::Error run() override;

private:
	void handleLeftButtonDown(const Common::Point &mousePosition);
	bool pollEvents();
	bool pollEvents(bool handleClicks);
	void serviceCursorRefresh(bool screenUpdated = false);
	void delayWithEventPolling(uint32 millis);
	void delayWithEventPolling(uint32 millis, bool handleClicks);
	void mainLoop();

	const HaroldGameDescription *_gameDescription;
	uint32 _lastCursorRefreshMillis = 0;
	Common::Point _mousePosition;
	bool _suppressClickHandling = false;
};

} // End of namespace BCB

#endif // BCB_H
