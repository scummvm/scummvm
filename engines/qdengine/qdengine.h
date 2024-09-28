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

#ifndef QDENGINE_H
#define QDENGINE_H

#include "common/error.h"
#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "graphics/pixelformat.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_screen_text.h"

struct ADGameDescription;

namespace Graphics {
class Screen;
}

namespace QDEngine {

struct QDEngineGameDescription;
class grDispatcher;
class qdGameDispatcher;

enum QDEngineDebugChannels {
	kDebugGraphics = 1,
	kDebugImGui,
	kDebugInput,
	kDebugLoad,
	kDebugLog,
	kDebugSave,
	kDebugSound,
	kDebugTemp,
	kDebugQuant,
	kDebugMinigames,
	kDebugMovement,
	kDebugText,
};

class QDEngineEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	grDispatcher *_grD = nullptr;
	qdGameDispatcher *_gameD = nullptr;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;
	Graphics::PixelFormat _pixelformat;
	int _tagMap[QDSCR_MAX_KEYWORD_ID];

	int _screenW;
	int _screenH;

	bool _forceFullRedraw = false;

public:
	QDEngineEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~QDEngineEngine() override;

	uint32 getFeatures() const;
	Common::Language getLanguage() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	const char *getExeName() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	void setSeed(uint32 seed) {
		_randomSource.setSeed(seed);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	void syncSoundSettings() override;

	int engineMain();

	const Vect2i &screen_offset() {
		return _screen_offset;
	}
	void set_screen_offset(const Vect2i &offset) {
		_screen_offset = offset;
	}

public:
	int _thumbSizeX = 0, _thumbSizeY = 0;
	bool _debugDraw = false;
	bool _debugDrawGrid = false;
	int _gameVersion = 0;

	Vect2i _screen_offset = Vect2i(0, 0);
	grScreenRegion _fps_region = grScreenRegion_EMPTY;
	grScreenRegion _fps_region_last = grScreenRegion_EMPTY;
	Std::vector<qdGameObject *> _visible_objects;

	// Default text format
	qdScreenTextFormat _default_format;
	// Global format
	qdScreenTextFormat _global_text_format;
	// Global format for dialog topics
	qdScreenTextFormat _global_topic_format;

private:
	void init_graphics();
};

extern QDEngineEngine *g_engine;
#define SHOULD_QUIT ::QDEngine::g_engine->shouldQuit()

} // namespace QDEngine

byte *transCyrillic(const Common::String &str);

#endif // QDENGINE_H
