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
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"
#include "video/video_decoder.h"

#include "phoenixvr/angle.h"
#include "phoenixvr/detection.h"
#include "phoenixvr/region_set.h"
#include "phoenixvr/script.h"
#include "phoenixvr/vr.h"

namespace Graphics {
class Font;
}

namespace PhoenixVR {

struct PhoenixVRGameDescription;
struct GameState;

class PhoenixVREngine : public Engine {
private:
	static constexpr uint kFPSLimit = 60;

	Graphics::Screen *_screen = nullptr;
	Common::Point _screenCenter;
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Graphics::PixelFormat _pixelFormat;
	Graphics::PixelFormat _rgb565;
	Graphics::ManagedSurface _thumbnail;

	// Engine APIs
	Common::Error run() override;

public:
	PhoenixVREngine(OSystem *syst, const ADGameDescription *gameDesc);
	~PhoenixVREngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
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

	// Script API
	void setNextScript(const Common::String &path);
	void goToWarp(const Common::String &warp, bool savePrev = false);
	void returnToWarp();
	void setCursorDefault(int idx, const Common::String &path);
	void setCursor(const Common::String &path, const Common::String &warp, int idx);
	void hideCursor(const Common::String &warp, int idx);

	void playSound(const Common::String &sound, uint8 volume, int loops, bool spatial = false, float angle = 0);
	void stopSound(const Common::String &sound);
	void playMovie(const Common::String &movie);

	void declareVariable(const Common::String &name);
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
	void startTimer(float seconds);
	void pauseTimer(bool pause, bool deactivate);
	void killTimer();
	void playAnimation(const Common::String &name, const Common::String &var, int varValue, float speed);
	void setZoom(int fov) {
		_fov = kPi * fov / 180;
	}

	void setXMax(float max) {
		static const float baseX = -kPi2;
		_angleY.setRange(baseX - max, baseX + max);
	}

	// this is set to large values and effectively useless
	void setYMax(float min, float max) {
		_angleX.setRange(min, max);
	}

	void setAngle(float x, float y) {
		_angleX.set(y);
		static const float baseX = -kPi2;
		_angleY.set(baseX + x);
	}

	bool testSaveSlot(int idx) const;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void drawSlot(int idx, int face, int x, int y);
	void captureContext();

	void setContextLabel(const Common::String &contextLabel) {
		_contextLabel = contextLabel;
	}

	bool isLoading() const {
		return _loading;
	}

	void saveVariables();
	void loadVariables();

	void rollover(Common::Rect dstRect, int textId, int size, bool bold, uint16_t color);

private:
	static Common::String removeDrive(const Common::String &path);
	Common::Path resolve(const Common::String &name);

	Graphics::Surface *loadSurface(const Common::String &path);
	Graphics::Surface *loadCursor(const Common::String &path);
	void paint(Graphics::Surface &src, Common::Point dst);
	PointF currentVRPos() const {
		return RectF::transform(_angleX.angle(), _angleY.angle(), _fov);
	}
	void tick(float dt);
	void tickTimer(float dt);
	void loadNextScript();
	void renderVR(float dt);

private:
	Common::Point _mousePos, _mouseRel;
	Common::String _nextScript;
	Common::Path _currentScriptPath;
	int _warpIdx = -1;
	Script::ConstWarpPtr _warp;
	int _nextWarp = -1;
	int _prevWarp = -1;
	int _hoverIndex = -1;
	int _nextTest = -1;

	struct KeyCodeHash : public Common::UnaryFunction<Common::KeyCode, uint> {
		uint operator()(Common::KeyCode val) const { return static_cast<uint>(val); }
	};

	Common::Array<Common::String> _lockKey;
	Common::Array<Common::String> _variableOrder;
	Common::Array<int> _variableSnapshot;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;
	struct Sound {
		Audio::SoundHandle handle;
		bool spatial;
		float angle;
		uint8 volume;
		int loops;
	};
	Common::HashMap<Common::String, Sound, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _sounds;
	Common::ScopedPtr<Script> _script;

	Common::ScopedPtr<RegionSet> _regSet;

	Common::HashMap<Common::String, Graphics::Surface *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _cursorCache;

	Common::Array<Common::Array<Common::String>> _cursors;
	Common::String _defaultCursor[2];

	VR _vr;
	float _fov;
	AngleX _angleX;
	AngleY _angleY;
	Audio::Mixer *_mixer;
	bool _loading = false;
	bool _showRegions = false;

	static constexpr byte kPaused = 2;
	static constexpr byte kActive = 4;
	byte _timerFlags = 0;
	float _timer = 0;

	Common::String _contextScript;
	Common::String _contextLabel;
	Common::Array<byte> _capturedState;

	Common::HashMap<int, Common::String> _textes;

	Common::ScopedPtr<Graphics::Font> _font12;
	Common::ScopedPtr<Graphics::Font> _font14;
	Common::ScopedPtr<Graphics::Font> _font18;

	Common::ScopedPtr<Graphics::ManagedSurface> _text;
	Common::Rect _textRect;
};

extern PhoenixVREngine *g_engine;
#define SHOULD_QUIT ::PhoenixVR::g_engine->shouldQuit()

} // End of namespace PhoenixVR

#endif // PHOENIXVR_H
