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

namespace PhoenixVR {

struct PhoenixVRGameDescription;

class PhoenixVREngine : public Engine {
private:
	Graphics::Screen *_screen = nullptr;
	Common::Point _screenCenter;
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Graphics::PixelFormat _pixelFormat;

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

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	// Script API
	void setNextScript(const Common::String &path);
	void goToWarp(const Common::String &warp);
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
	void end();

	Script::ConstWarpPtr getWarp(const Common::String &name);
	Script::ConstWarpPtr getCurrentWarp() { return _warp; }
	const Region *getRegion(int idx) const;

	uint numCursors() const {
		return _cursors.size();
	}
	const Region *getCursorRegion(uint idx) const {
		return idx < _cursors.size() ? &_cursors[idx].region : nullptr;
	}

	void resetLockKey();
	void lockKey(Common::KeyCode code, const Common::String &warp);
	void startTimer(float seconds);
	void pauseTimer(bool pause, bool deactivate);
	void killTimer();

private:
	static Common::String removeDrive(const Common::String &path);
	static Common::String resolvePath(const Common::String &path);
	Graphics::Surface *loadSurface(const Common::String &path);
	void paint(Graphics::Surface &src, Common::Point dst);
	PointF currentVRPos() const {
		return RectF::transform(_angleX.angle(), _angleY.angle(), _fov);
	}
	void tick(float dt);
	void tickTimer(float dt);

private:
	Common::Point _mousePos;
	Common::String _nextScript;
	Common::String _nextWarp;

	struct KeyCodeHash : public Common::UnaryFunction<Common::KeyCode, uint> {
		uint operator()(Common::KeyCode val) const { return static_cast<uint>(val); }
	};

	Common::HashMap<Common::KeyCode, Common::String, KeyCodeHash> _keys;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;
	struct Sound {
		Audio::SoundHandle handle;
		bool spatial;
		float angle;
	};
	Common::HashMap<Common::String, Sound, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _sounds;
	Common::ScopedPtr<Script> _script;

	Script::ConstWarpPtr _warp;
	Common::ScopedPtr<RegionSet> _regSet;

	Common::ScopedPtr<Video::VideoDecoder> _movie;

	struct Cursor {
		Region region;
		Graphics::Surface *surface = nullptr;
		void free();
	};
	Common::Array<Cursor> _cursors;
	Cursor _defaultCursor[2];
	VR _vr;
	float _fov;
	AngleX _angleX;
	AngleY _angleY;
	Audio::Mixer *_mixer;

	static constexpr byte kPaused = 2;
	static constexpr byte kActive = 4;
	byte _timerFlags = 0;
	float _timer = 0;
};

extern PhoenixVREngine *g_engine;
#define SHOULD_QUIT ::PhoenixVR::g_engine->shouldQuit()

} // End of namespace PhoenixVR

#endif // PHOENIXVR_H
