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

#ifndef MACS2_H
#define MACS2_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "common/file.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "macs2/detection.h"
#include "macs2/events.h"
#include <common/memstream.h>

namespace Macs2 {

struct Macs2GameDescription;

struct GlyphData {
	byte* Data;
	char ASCII;
	uint16 Width;
	uint16 Height;

	void ReadFromeFile(Common::File &file);
};

class Macs2Engine : public Engine, public Events {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
protected:
	// Engine APIs
	Common::Error run() override;

	/**
	 * Returns true if the game should quit
	 */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

	Graphics::ManagedSurface readRLEImage(int64 offs, Common::File& file);

	void readResourceFile();

public:
	Macs2Engine(OSystem *syst, const ADGameDescription *gameDesc);
	~Macs2Engine() override;

	struct Graphics::ManagedSurface _bgImageShip;
	Graphics::ManagedSurface _map;
	byte _pal[256 * 3] = { 0 };

	byte* _charData;
	char _charASCII;
	uint16 _charWidth;
	uint16 _charHeight;

	GlyphData _glyphs[256];
	// TODO: THis count could be read from the file as well
	uint16 numGlyphs = 79;

	bool FindGlyph(char c, GlyphData &out) const;

	byte* _cursorData;
	uint16 _cursorWidth;
	uint16 _cursorHeight;

	// TODO: Need a data structure for this by now or check if a bitmap with transparent pixels for blitting exists in ScummVM
	byte* _guyData;
	uint16 _guyWidth;
	uint16 _guyHeight;

	byte* _borderData;
	uint16 _borderWidth;
	uint16 _borderHeight;

	byte* _shadingTable;

	byte* _borderHighlightData;
	uint16 _borderHighlightWidth;
	uint16 _borderHighlightHeight;

	byte** _flagData;
	uint16* _flagWidths;
	uint16* _flagHeights;

	byte* mapData;

	Common::MemoryReadStream* _scriptStream;
	byte* _scriptData;

	void ExecuteScript(Common::MemoryReadStream* stream);


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
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
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
};

extern Macs2Engine *g_engine;
#define SHOULD_QUIT ::Macs2::g_engine->shouldQuit()

} // End of namespace Macs2

#endif // MACS2_H
