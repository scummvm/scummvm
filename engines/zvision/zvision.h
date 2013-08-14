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
 *
 */

#ifndef ZVISION_H
#define ZVISION_H

#include "common/random.h"
#include "common/events.h"

#include "engines/engine.h"

#include "graphics/pixelformat.h"

#include "zvision/detection.h"
#include "zvision/clock.h"
#include "zvision/mouse_event.h"

#include "gui/debugger.h"


namespace Video {
class VideoDecoder;
}

namespace ZVision {

struct ZVisionGameDescription;
class Console;
class ScriptManager;
class RenderManager;
class CursorManager;
 
// our engine debug channels
enum {
	kZDebugExample = 1 << 0,
	kZDebugExample2 = 1 << 1
	// next new channel must be 1 << 2 (4)
	// the current limitation is 32 debug channels (1 << 31 is the last one)
};
 
class ZVision : public Engine {
public:
	ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc);
	~ZVision();

private:
	enum {
		WINDOW_WIDTH = 640,
		WINDOW_HEIGHT = 480,
		WORKING_WINDOW_WIDTH = 512,
		WORKING_WINDOW_HEIGHT = 320,

		ROTATION_SCREEN_EDGE_OFFSET = 60,
		MAX_ROTATION_SPEED = 150 // Pixels per second
	};

	Console *_console;
	const ZVisionGameDescription *_gameDescription;
	const Common::Rect _workingWindow;
	const Graphics::PixelFormat _pixelFormat;

	const int _desiredFrameTime;

	// We need random numbers
	Common::RandomSource *_rnd;

	// Managers
	ScriptManager *_scriptManager;
	RenderManager *_renderManager;
	CursorManager *_cursorManager;

	// Clock
	Clock _clock;

	// To store the current mouse events
	Common::List<MouseEvent> _mouseEvents;

	// To prevent allocation every time we process events
	Common::Event _event;

public:
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Error run();
	void pauseEngineIntern(bool pause);

	ScriptManager *getScriptManager() const;
	RenderManager *getRenderManager() const;
	CursorManager *getCursorManager() const;
	Common::RandomSource *getRandomSource() const;
	ZVisionGameId getGameId() const;

	void playVideo(Video::VideoDecoder &videoDecoder, const Common::Rect &destRect = Common::Rect(0, 0, 0, 0), bool skippable = true);

	void registerMouseEvent(const MouseEvent &event);
	void clearAllMouseEvents();

	void cycleThroughCursors();

private:
	void initialize();

	void processEvents();
	void onMouseDown(const Common::Point &pos);
	void onMouseUp(const Common::Point &pos);
	void onMouseMove(const Common::Point &pos);
	void onKeyDown(uint keyCode);
};

} // End of namespace ZVision
 
#endif
