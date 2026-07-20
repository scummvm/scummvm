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

#ifndef PHOENIXVR_H
#define PHOENIXVR_H

#include "audio/mixer.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/keyboard.h"
#include "common/ptr.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/framelimiter.h"
#include "graphics/screen.h"
#include "video/video_decoder.h"

#include "phoenixvr/angle.h"
#include "phoenixvr/detection.h"
#include "phoenixvr/region_set.h"
#include "phoenixvr/script.h"
#include "phoenixvr/variables.h"
#include "phoenixvr/vr.h"

namespace Common {
struct Event;
}

namespace Graphics {
class Font;
}

namespace Video {
class Subtitles;
}

namespace PhoenixVR {

class ARN;
struct PhoenixVRGameDescription;
struct GameState;

enum struct RolloverType : uint8 {
	Default,
	Malette,
	Secretaire
};

class PhoenixVREngine : public Engine {
	friend class ARN;

private:
	static constexpr uint kFPSLimit = 60;
	static constexpr float kMaxTick = 4.0f / kFPSLimit;

	Graphics::FrameLimiter _frameLimiter;
	Graphics::Screen *_screen = nullptr;
	Common::Point _screenCenter;
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Graphics::PixelFormat _pixelFormat;
	Graphics::PixelFormat _rgb565;
	Graphics::ManagedSurface _thumbnail;
	Common::ScopedPtr<ARN> _arn;

	// Engine APIs
	Common::Error run() override;

public:
	PhoenixVREngine(OSystem *syst, const ADGameDescription *gameDesc);
	~PhoenixVREngine() override;

	uint32 getFeatures() const;

	int version() const;

	bool gameIdMatches(const char *gameId) const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsSubtitleOptions) ||
			   (f == kSupportsReturnToLauncher);
	};

	// disable autosave
	int getAutosaveSlot() const override { return -1; }

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return false;
	}

	void syncSoundSettings() override;

	// Script API
	void setNextScript(const Common::String &path);
	bool goToWarp(const Common::String &warp, bool savePrev = false);
	void goToLevel(const Common::String &name);
	void returnToWarp();
	void loadCursor(int idx, const Common::String &path, int w, int h);
	void setCursorDefault(int idx, const Common::String &path);
	void setCursorDefault(int idx, int cursorIdx);
	void setCursor(const Common::String &path, const Common::String &warp, int idx);
	void hideCursor(const Common::String &warp, int idx);

	void playSound(const Common::String &sound, Audio::Mixer::SoundType type, uint8 volume, int loops, bool spatial = false, float angle = 0);
	void playRandomSound(const Common::String &sound, Audio::Mixer::SoundType type, uint8 volume, int probability, int loops);
	void stopSound(const Common::String &sound);
	void stopAllSounds();
	void playMovie(const Common::String &movie);

	void declareVariable(const Common::String &name);
	bool hasVariable(const Common::String &name) const;
	void setVariable(const Common::String &name, int value);
	int getVariable(const Common::String &name) const;

	void executeTest(int idx);
	void scheduleTest(int idx);
	void end();
	void wait(float seconds);
	void until(const Common::String &var, int value);

	const Region *getRegion(int idx) const;

	void resetLockKey();
	void lockKey(int idx, const Common::String &warp);
	void startTimer(float seconds, bool showTimer);
	void pauseTimer(bool pause, bool deactivate);
	void killTimer();
	void playAnimation(const Common::String &name, const Common::String &var, int varValue, float speed);
	void stopAnimation(const Common::String &name);
	void setZoom(float fov) {
		_fov = fov;
	}
	void interpolateAngle(float x, float y, float speed, float zoom);
	void fade(int start, int stop, int speed);
	void transFade(int speed);

	void setXMax(float max) {
		_angleY.setRange(-max, max);
	}

	// this is set to large values and effectively useless
	void setYMax(float min, float max) {
		_angleX.setRange(min, max);
	}

	void resetYMax() {
		_angleX.resetRange();
	}

	void setAngle(float x, float y) {
		_angleX.set(y);
		_angleY.set(kPi2 - x);
	}

	void setNord(float a) {
		_angleX.add(a);
	}

	bool testSaveSlot(int idx) const;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void drawSlot(int idx, int face, int x, int y);
	void drawSaveCard(int idx);
	void captureContext();

	void setContextLabel(const Common::String &contextLabel) {
		_contextLabel = contextLabel;
	}
	bool enterScript();
	bool isLoading() const { return !_loadedState.empty(); }

	bool wasRestarted() const { return _restarted; }
	bool wasLoaded() const { return _loaded; }
	uint currentLevel() const;

	void saveVariables();
	void loadVariables();

	void rollover(int textId, RolloverType type);
	void clearText();
	void showWaves();
	void restart();
	bool setNextLevel();

	void setGlobalVolume(int vol);
	void setGlobalPan(int pan);
	void drawArchiveImage(const Common::String &image, int x, int y);
	void drawArchiveText(int textId, const Common::Rect &dstRect, int size, bool bold, uint16 color);
	void clearArchiveText(const Common::Rect &dstRect);
	void showImageOverlay(const Common::String &image, int x, int y);
	void stopImageOverlay();
	void updateStage();
	void startCible(const Common::String &name, int periodSeconds, const Common::Array<int> &bounds);
	void stopCible();
	void testCible(const Common::String &insideVar, const Common::String &outsideVar);

private:
	struct ArchiveImage {
		Common::String image;
		Common::Point pos;
	};

	struct TextState {
		TextState() : textId(-1), size(0), bold(false), color(0) {}
		TextState(int textId_, const Common::Rect &rect_, int size_, bool bold_, uint16 color_) : textId(textId_), rect(rect_), size(size_), bold(bold_), color(color_) {}

		int textId;
		Common::Rect rect;
		int size;
		bool bold;
		uint16 color;
	};

	struct Level {
		Common::String path;
		Common::String name;
	};

	static Common::String removeDrive(const Common::String &path);
	Common::SeekableReadStream *open(const Common::String &name, Common::String *origName = nullptr);
	Common::SeekableReadStream *tryOpen(const Common::Path &name, Common::String *origName);

	Graphics::ManagedSurface *loadSurface(const Common::String &path);
	Graphics::ManagedSurface *loadCursor(const Common::String &path, int w = 0, int h = 0);
	PointF currentVRPos() const {
		return RectF::transform(_angleX.angle(), _angleY.angle(), _fov);
	}
	void tick(float dt);
	void tickTimer(float dt);
	void loadNextScript();
	const Graphics::Surface *findArchiveImage(const Common::String &image) const;
	void renderVR(float dt);
	void renderArchiveImages();
	void renderArchiveTexts();
	void paintText(const TextState &textState);
	void renderImageOverlay();
	void renderTimer();
	void renderFade(int color);
	void resetState();
	const Graphics::Font *getFont(int size, bool bold) const;
	Common::Path getSubtitlePath(const Common::String &path) const;
	Common::SharedPtr<Video::Subtitles> loadSubtitles(const Common::String &path) const;
	void setupSubtitles(Video::Subtitles &subtitles) const;
	void drawAudioSubtitles();

	void processGenericEvents(const Common::Event &event);
	void pauseEngineIntern(bool pause) override;
	Common::String getLevelLabel(const Common::String &script) const;
	Common::String getLevelScript(const Level &level) const;
	void saveThumbnail();

private:
	bool _hasFocus = true;
	Common::Point _mousePos, _mouseRel;
	Common::String _nextScript;
	Common::Path _currentScriptPath;
	int _warpIdx = -1;
	Script::ConstWarpPtr _warp;
	int _nextWarp = -1;
	int _prevWarp = -1;
	int _hoverIndex = -1;
	int _messengerInventoryHover = -1;
	int _nextTest = -1;

	struct KeyCodeHash : public Common::UnaryFunction<Common::KeyCode, uint> {
		uint operator()(Common::KeyCode val) const { return static_cast<uint>(val); }
	};

	Common::Array<Common::String> _lockKey;

	Variables _variables;

	struct Sound {
		Audio::SoundHandle handle;
		bool spatial;
		float angle;
		uint8 volume;
		int loops;
		Common::SharedPtr<Video::Subtitles> subtitles;
	};
	Common::HashMap<Common::String, Sound, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _sounds;

	struct RandomSound {
		Common::String sound;
		Audio::Mixer::SoundType type;
		int volume;
		int probability;
		int loops;
	};
	Common::Array<RandomSound> _randomSounds;

	Common::ScopedPtr<Script> _script;

	Common::ScopedPtr<RegionSet> _regSet;

	struct PreloadedCursor {
		Common::String path;
	};
	Common::Array<PreloadedCursor> _loadedCursors;

	Common::HashMap<Common::String, Graphics::ManagedSurface *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _cursorCache;

	Common::Array<Common::Array<Common::String>> _cursors;
	Common::String _defaultCursor[2];
	Common::String _currentMusic;
	int _currentMusicVolume = 0;
	int _globalPan = 128;

	VR _vr;
	float _fov;
	AngleX _angleX;
	AngleY _angleY;
	Audio::Mixer *_mixer;
	bool _showRegions = false;

	static constexpr byte kPaused = 2;
	static constexpr byte kActive = 4;
	byte _timerFlags = 0;
	bool _showTimer = false;
	float _timer = 0, _initialTimer = 0;

	Common::String _contextScript;
	Common::String _contextLabel;
	Common::Array<byte> _capturedState;
	Common::Array<byte> _loadedState;

	Common::HashMap<int, Common::U32String> _textes;

	static const int kFontSizeCount = 6;
	Common::ScopedPtr<Graphics::Font> _regularFonts[kFontSizeCount];
	Common::ScopedPtr<Graphics::Font> _boldFonts[kFontSizeCount];

	TextState _rolloverText;
	Common::ScopedPtr<Graphics::ManagedSurface> _imageOverlay;
	Common::Point _imageOverlayPos;
	Common::Array<ArchiveImage> _archiveImages;
	Common::Array<TextState> _archiveTexts;
	bool _cibleActive = false;
	uint32 _cibleStartMillis = 0;
	int _ciblePeriodSeconds = 0;
	Common::Array<int> _cibleBounds;

	Common::Array<Level> _levels;
	uint _nextLevel = 0;

	bool _restarted = false;
	bool _loaded = false;
};

extern PhoenixVREngine *g_engine;
#define SHOULD_QUIT ::PhoenixVR::g_engine->shouldQuit()

} // End of namespace PhoenixVR

#endif // PHOENIXVR_H
