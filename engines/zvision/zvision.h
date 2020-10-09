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
 *
 */

#ifndef ZVISION_ZVISION_H
#define ZVISION_ZVISION_H

#include "zvision/core/clock.h"
#include "zvision/file/search_manager.h"
#include "zvision/detection.h"

#include "common/random.h"
#include "common/events.h"

#include "engines/engine.h"

#include "graphics/pixelformat.h"

#include "gui/debugger.h"

namespace Common {
class Keymap;
}

namespace Video {
class VideoDecoder;
}

/**
 * This is the namespace of the ZVision engine.
 *
 * Status of this engine: complete
 *
 * Games using this engine:
 * - Zork Nemesis: The Forbidden Lands
 * - Zork: Grand Inquisitor
 *
 */
namespace ZVision {

class Console;
class ScriptManager;
class RenderManager;
class CursorManager;
class StringManager;
class SaveManager;
class RLFDecoder;
class MenuHandler;
class TextRenderer;
class Subtitle;
class MidiManager;

enum {
	WINDOW_WIDTH = 640,
	WINDOW_HEIGHT = 480,

	HIRES_WINDOW_WIDTH = 800,
	HIRES_WINDOW_HEIGHT = 600,

	// Zork Nemesis working window sizes
	ZNM_WORKING_WINDOW_WIDTH = 512,
	ZNM_WORKING_WINDOW_HEIGHT = 320,

	// ZGI working window sizes
	ZGI_WORKING_WINDOW_WIDTH = 640,
	ZGI_WORKING_WINDOW_HEIGHT = 344,

	ROTATION_SCREEN_EDGE_OFFSET = 60,
	MAX_ROTATION_SPEED = 400, // Pixels per second

	KEYBUF_SIZE = 20
};

enum ZVisionAction {
	kZVisionActionNone,
	kZVisionActionUp,
	kZVisionActionDown,
	kZVisionActionLeft,
	kZVisionActionRight,
	kZVisionActionSave,
	kZVisionActionRestore,
	kZVisionActionQuit,
	kZVisionActionPreferences,
	kZVisionActionShowFPS,
	kZVisionActionSkipCutscene,

	kZVisionActionCount
};

extern const char *mainKeymapId;
extern const char *gameKeymapId;
extern const char *cutscenesKeymapId;

class ZVision : public Engine {
public:
	ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc);
	~ZVision() override;

public:
	/**
	 * A Rectangle centered inside the actual window. All in-game coordinates
	 * are given in this coordinate space. Also, all images are clipped to the
	 * edges of this Rectangle
	 */
	Common::Rect _workingWindow;
	const Graphics::PixelFormat _resourcePixelFormat;
	const Graphics::PixelFormat _screenPixelFormat;

private:
	const ZVisionGameDescription *_gameDescription;

	const int _desiredFrameTime;

	// We need random numbers
	Common::RandomSource *_rnd;

	// Managers
	ScriptManager *_scriptManager;
	RenderManager *_renderManager;
	CursorManager *_cursorManager;
	StringManager *_stringManager;
	SearchManager *_searchManager;
	TextRenderer *_textRenderer;
	MidiManager *_midiManager;
	SaveManager *_saveManager;
	MenuHandler *_menu;

	// Clock
	Clock _clock;

	// Audio ID
	int _audioId;

	// To prevent allocation every time we process events
	Common::Event _event;

	Common::Keymap *_gameKeymap, *_cutscenesKeymap;

	int _frameRenderDelay;
	int _renderedFrameCount;
	int _fps;
	int16 _mouseVelocity;
	int16 _keyboardVelocity;
	bool _doubleFPS;
	bool _videoIsPlaying;

	uint8 _cheatBuffer[KEYBUF_SIZE];

public:
	Common::Error run() override;
	void pauseEngineIntern(bool pause) override;

	ZVisionGameId getGameId() const;
	Common::Language getLanguage() const;
	uint32 getFeatures() const;

	ScriptManager *getScriptManager() const {
		return _scriptManager;
	}
	RenderManager *getRenderManager() const {
		return _renderManager;
	}
	CursorManager *getCursorManager() const {
		return _cursorManager;
	}
	SaveManager *getSaveManager() const {
		return _saveManager;
	}
	StringManager *getStringManager() const {
		return _stringManager;
	}
	SearchManager *getSearchManager() const {
		return _searchManager;
	}
	TextRenderer *getTextRenderer() const {
		return _textRenderer;
	}
	MidiManager *getMidiManager() const {
		return _midiManager;
	}
	MenuHandler *getMenuHandler() const {
		return _menu;
	}

	Common::Keymap *getGameKeymap() const {
		return _gameKeymap;
	}
	Common::RandomSource *getRandomSource() const {
		return _rnd;
	}
	int16 getKeyboardVelocity() const {
		return _keyboardVelocity;
	}
	int16 getMouseVelocity() const {
		return _mouseVelocity;
	}

	uint8 getZvisionKey(Common::KeyCode scummKeyCode);

	void startClock() {
		_clock.start();
	}

	void stopClock() {
		_clock.stop();
	}

	void initScreen();
	void initHiresScreen();

	/**
	 * Play a video until it is finished. This is a blocking call. It will call
	 * _clock.stop() when the video starts and _clock.start() when the video finishes.
	 * It will also consume all events during video playback.
	 *
	 * @param videoDecoder    The video to play
	 * @param destRect        Where to put the video. (In working window coords)
	 * @param skippable       If true, the video can be skipped at any time using [Spacebar]
	 */
	void playVideo(Video::VideoDecoder &videoDecoder, const Common::Rect &destRect = Common::Rect(0, 0, 0, 0), bool skippable = true, Subtitle *sub = NULL);
	Video::VideoDecoder *loadAnimation(const Common::String &fileName);

	void setRenderDelay(uint);
	bool canRender();
	static void fpsTimerCallback(void *refCon);
	void fpsTimer();
	int getFPS() const {
		return _fps;
	}

	void syncSoundSettings() override;

	void loadSettings();
	void saveSettings();

	bool ifQuit();

	// Engine features
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

private:
	void initialize();
	void initFonts();

	void parseStrFile(const Common::String fileName);

	/** Called every frame from ZVision::run() to process any events from EventMan */
	void processEvents();

	void onMouseMove(const Common::Point &pos);

	void registerDefaultSettings();

	void cheatCodes(uint8 key);
	void pushKeyToCheatBuf(uint8 key);
	bool checkCode(const char *code);
	uint8 getBufferedKey(uint8 pos);

	double getVobAmplification(Common::String fileName) const;
};

} // End of namespace ZVision

#endif
