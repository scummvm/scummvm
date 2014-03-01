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

#ifndef ZVISION_ZVISION_H
#define ZVISION_ZVISION_H

#include "zvision/core/console.h"
#include "zvision/detection.h"
#include "zvision/utility/clock.h"

#include "common/random.h"
#include "common/events.h"

#include "engines/engine.h"

#include "graphics/pixelformat.h"

#include "gui/debugger.h"


namespace Video {
class VideoDecoder;
}

namespace ZVision {

struct ZVisionGameDescription;
class ScriptManager;
class RenderManager;
class CursorManager;
class StringManager;
class SaveManager;
class RlfAnimation;

class ZVision : public Engine {
public:
	ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc);
	~ZVision();

public:
	/**
	 * A Rectangle centered inside the actual window. All in-game coordinates
	 * are given in this coordinate space. Also, all images are clipped to the
	 * edges of this Rectangle
	 */
	const Common::Rect _workingWindow;
	const Graphics::PixelFormat _pixelFormat;

private:
	enum {
		WINDOW_WIDTH = 640,
		WINDOW_HEIGHT = 480,

		//Zork nemesis working window sizes
		ZNEM_WORKING_WINDOW_WIDTH = 512,
		ZNEM_WORKING_WINDOW_HEIGHT = 320,

		//ZGI(and default) working window sizes
		ZGI_WORKING_WINDOW_WIDTH = 640,
		ZGI_WORKING_WINDOW_HEIGHT = 344,

		ROTATION_SCREEN_EDGE_OFFSET = 60,
		MAX_ROTATION_SPEED = 400 // Pixels per second
	};

	Console *_console;
	const ZVisionGameDescription *_gameDescription;

	const int _desiredFrameTime;

	// We need random numbers
	Common::RandomSource *_rnd;

	// Managers
	ScriptManager *_scriptManager;
	RenderManager *_renderManager;
	CursorManager *_cursorManager;
	SaveManager *_saveManager;
	StringManager *_stringManager;

	// Clock
	Clock _clock;

	// To prevent allocation every time we process events
	Common::Event _event;

public:
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Error run();
	void pauseEngineIntern(bool pause);

	ScriptManager *getScriptManager() const { return _scriptManager; }
	RenderManager *getRenderManager() const { return _renderManager; }
	CursorManager *getCursorManager() const { return _cursorManager; }
	SaveManager *getSaveManager() const { return _saveManager; }
	StringManager *getStringManager() const { return _stringManager; }
	Common::RandomSource *getRandomSource() const { return _rnd; }
	ZVisionGameId getGameId() const { return _gameDescription->gameId; }
	GUI::Debugger *getDebugger() { return _console; }

	/**
	 * Play a video until it is finished. This is a blocking call. It will call
	 * _clock.stop() when the video starts and _clock.start() when the video finishes.
	 * It will also consume all events during video playback.
	 *
	 * @param videoDecoder    The video to play
	 * @param destRect        Where to put the video. (In working window coords)
	 * @param skippable       If true, the video can be skipped at any time using [Spacebar]
	 */
	void playVideo(Video::VideoDecoder &videoDecoder, const Common::Rect &destRect = Common::Rect(0, 0, 0, 0), bool skippable = true);

	Common::String generateSaveFileName(uint slot);
	Common::String generateAutoSaveFileName();

private:
	void initialize();
	void initFonts();

	void parseStrFile(const Common::String fileName);

	/** Called every frame from ZVision::run() to process any events from EventMan */
	void processEvents();

	void onMouseDown(const Common::Point &pos);
	void onMouseUp(const Common::Point &pos);
	void onMouseMove(const Common::Point &pos);
};

} // End of namespace ZVision

#endif
