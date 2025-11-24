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

#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "phoenixvr/detection.h"
#include "phoenixvr/region_set.h"
#include "phoenixvr/script.h"

namespace PhoenixVR {

struct PhoenixVRGameDescription;

class PhoenixVREngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Graphics::PixelFormat _pixelFormat;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;

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

	void declareVariable(const Common::String &name);
	void setVariable(const Common::String &name, int value);
	int getVariable(const Common::String &name) const;

	void executeTest(int idx);

	Script::ConstWarpPtr getWarp(const Common::String &name);
	Script::ConstWarpPtr getCurrentWarp() { return _warp; }
	Region getRegion(int idx) const;

private:
	static Common::String removeDrive(const Common::String &path);
	static Common::String resolvePath(const Common::String &path);
	Graphics::Surface *loadSurface(const Common::String &path);
	void paint(Graphics::Surface &src, Common::Point dst);

private:
	Common::Point _mousePos;
	Common::String _nextScript;
	Common::String _nextWarp;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;
	Common::ScopedPtr<Script> _script;

	Script::ConstWarpPtr _warp;
	Common::ScopedPtr<RegionSet> _regSet;

	struct Cursor {
		Common::Rect rect;
		Graphics::Surface *surface = nullptr;
		void free();
	};
	Common::Array<Cursor> _cursors;
	Cursor _defaultCursor;
	Graphics::Surface *_static = nullptr;
};

extern PhoenixVREngine *g_engine;
#define SHOULD_QUIT ::PhoenixVR::g_engine->shouldQuit()

} // End of namespace PhoenixVR

#endif // PHOENIXVR_H
