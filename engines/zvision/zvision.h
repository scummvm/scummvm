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
 *
 */

#ifndef ZVISION_ZVISION_H
#define ZVISION_ZVISION_H

#include "common/random.h"
#include "common/events.h"
#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include "gui/debugger.h"
#include "zvision/detection.h"
#include "zvision/core/clock.h"

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

struct ZvisionGameDescription;

class Console;
class ScriptManager;
class RenderManager;
class CursorManager;
class StringManager;
class FileManager;
class SaveManager;
class RLFDecoder;
class MenuManager;
class SubtitleManager;
class TextRenderer;
class Subtitle;
class MidiManager;
class VolumeManager;

struct ScreenLayout {
	Common::Rect screenArea;  // Original screen resolution
	Common::Rect menuArea;  // Menu display area, relative to original screen
	Common::Rect workingArea;   // Playfield & video playback area, relative to original screen
	Common::Rect textArea;  // Subtitle & message area, relative to original screen
};

// NB Footage of original DOS Nemesis engine indicates playfield was centrally placed on screen.
// Subtitle scripts, however, suggest playfield was higher up, otherwise they run off the bottom of the screen.  
// This could just be an error in the scripts or an artefact of the original game's development, so we will continue to use as-released central placement.

static const ScreenLayout nemesisLayout {
	Common::Rect(640, 480), // Screen
	Common::Rect(Common::Point(64, 0), 512, 32), // Menu
	Common::Rect(Common::Point(64, 80), 512, 320), // Working; aspect ratio 1.6
	Common::Rect(Common::Point(64, 420), 512, 60) // Text
};

static const ScreenLayout zgiLayout {
	Common::Rect(640, 480), // Screen
	Common::Rect(Common::Point(0, 0), 640, 32), // Menu
	Common::Rect(Common::Point(0, 68), 640, 344), // Working; aspect ratio 1.86
	Common::Rect(Common::Point(0, 412), 640, 68) // Text
};

enum {

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
	const Graphics::PixelFormat _resourcePixelFormat;
	const Graphics::PixelFormat _screenPixelFormat;

private:
	const ZVisionGameDescription *_gameDescription;


	// We need random numbers
	Common::RandomSource *_rnd;

	// Managers
	ScriptManager *_scriptManager;
	RenderManager *_renderManager;
	CursorManager *_cursorManager;
	StringManager *_stringManager;
	TextRenderer *_textRenderer;
	MidiManager *_midiManager;
	FileManager *_fileManager;
	SaveManager *_saveManager;
	MenuManager *_menu;
	SubtitleManager *_subtitleManager;
	VolumeManager *_volumeManager;

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
	bool _widescreen;
	bool _videoIsPlaying;
	bool _breakMainLoop = false;

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
	FileManager *getFileManager() const {
		return _fileManager;
	}
	SaveManager *getSaveManager() const {
		return _saveManager;
	}
	StringManager *getStringManager() const {
		return _stringManager;
	}
	TextRenderer *getTextRenderer() const {
		return _textRenderer;
	}
	MidiManager *getMidiManager() const {
		return _midiManager;
	}
	MenuManager *getMenuManager() const {
		return _menu;
	}
	SubtitleManager *getSubtitleManager() const {
		return _subtitleManager;
	}
	VolumeManager *getVolumeManager() const {
		return _volumeManager;
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

	void initScreen(bool hiRes = false);

	/**
	 * Play a video until it is finished. This is a blocking call. It will call
	 * _clock.stop() when the video starts and _clock.start() when the video finishes.
	 * It will also consume all events during video playback.
	 *
	 * @param videoDecoder    The video to play
	 * @param destRect        Where to put the video. (In working window coords)
	 * @param srcRect         What subset of video to blit to destRect (In video frame coords)  // TODO implement
	 * @param skippable       If true, the video can be skipped at any time using [Spacebar]
	 */
	void playVideo(Video::VideoDecoder &videoDecoder, Common::Rect destRect = Common::Rect(0, 0, 0, 0), bool skippable = true, uint16 sub = 0, Common::Rect srcRect = Common::Rect(0, 0, 0, 0));
	Video::VideoDecoder *loadAnimation(const Common::Path &fileName);

	void setRenderDelay(uint);
	bool canRender();
	static void fpsTimerCallback(void *refCon);
	void fpsTimer();
	int getFPS() const {
		return _fps;
	}

	bool isWidescreen() {
		return _widescreen;
	}

	void syncSoundSettings() override;

	void loadSettings();
	void saveSettings();

	bool quit(bool askFirst = true, bool streaming = false);
	
	// Engine features
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

private:
	void initialize();
	void initFonts();
	
	void initializePath(const Common::FSNode &gamePath) override;

	void parseStrFile(const Common::String &fileName);

	/** Called every frame from ZVision::run() to process any events from EventMan */
	void processEvents();

	void onMouseMove(const Common::Point &pos);

	void registerDefaultSettings();

	void cheatCodes(uint8 key);
	void pushKeyToCheatBuf(uint8 key);
	bool checkCode(const char *code);
	uint8 getBufferedKey(uint8 pos);
};

} // End of namespace ZVision

#endif
