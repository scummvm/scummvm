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

#ifndef DIRECTOR_DIRECTOR_H
#define DIRECTOR_DIRECTOR_H

#include "common/random.h"
#include "common/rect.h"
#include "common/str-array.h"

#include "common/hashmap.h"
#include "engines/engine.h"

#include "director/types.h"

namespace Common {
class MacResManager;
class SeekableReadStream;
class SeekableSubReadStreamEndian;
}

namespace Graphics {
class MacWindowManager;
struct MacPlotData;
typedef Common::Array<byte *> MacPatterns;

class ManagedSurface;
}

namespace Director {

enum DirectorGameGID {
	GID_GENERIC,
	GID_TEST,
	GID_TESTALL
};

class Archive;
class Cast;
struct DirectorGameDescription;
class DirectorSound;
class Lingo;
class Movie;
class Stage;
class Score;
class CastMember;
class Stxt;

enum {
	kDebugLingoExec		= 1 << 0,
	kDebugCompile		= 1 << 1,
	kDebugLoading		= 1 << 2,
	kDebugImages		= 1 << 3,
	kDebugText			= 1 << 4,
	kDebugEvents		= 1 << 5,
	kDebugParse			= 1 << 6,
	kDebugCompileOnly	= 1 << 7,
	kDebugSlow			= 1 << 8,
	kDebugFast			= 1 << 9,
	kDebugNoLoop		= 1 << 10,
	kDebugBytecode		= 1 << 11,
	kDebugFewFramesOnly	= 1 << 12,
	kDebugPreprocess	= 1 << 13,
	kDebugScreenshot	= 1 << 14,
	kDebugDesktop		= 1 << 15
};

struct MovieReference {
	Common::String movie;
	Common::String frameS;
	int frameI;

	MovieReference() { frameI = -1; }
};

struct PaletteV4 {
	int id;
	byte *palette;
	int length;
};

// An extension of MacPlotData for interfacing with inks and patterns without
// needing extra surfaces.
struct DirectorPlotData {
	Graphics::ManagedSurface *src;
	Graphics::ManagedSurface *dst;
	Graphics::MacPlotData *macPlot;
	Common::Rect destRect;
	Common::Point srcPoint;

	InkType ink;
	int numColors;
	uint backColor;

	Graphics::MacWindowManager *_wm;

	DirectorPlotData(Graphics::MacWindowManager *wm, Graphics::ManagedSurface *s, Graphics::ManagedSurface *d, InkType i, uint b, uint n) :
		src(s), dst(d), ink(i), backColor(b), macPlot(nullptr), numColors(n), _wm(wm) {
	}
};

void inkDrawPixel(int x, int y, int color, void *data);

class DirectorEngine : public ::Engine {
public:
	DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc);
	~DirectorEngine() override;

	// Detection related functions

	DirectorGameGID getGameGID() const;
	const char *getGameId() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	Common::Language getLanguage() const;
	Common::String getEXEName() const;
	DirectorSound *getSoundManager() const { return _soundManager; }
	Graphics::MacWindowManager *getMacWindowManager() const { return _wm; }
	Archive *getMainArchive() const;
	Lingo *getLingo() const { return _lingo; }
	Stage *getStage() const { return _currentStage; }
	Movie *getCurrentMovie() const;
	Common::String getCurrentPath() const;
	void setPalette(int id);
	void setPalette(byte *palette, uint16 count);
	bool hasFeature(EngineFeature f) const override;
	void loadPalettes();
	const byte *getPalette() const { return _currentPalette; }
	uint16 getPaletteColorCount() const { return _currentPaletteLength; }
	void loadPatterns();
	uint32 transformColor(uint32 color);
	Graphics::MacPatterns &getPatterns();
	void setCursor(int type); // graphics.cpp

	void loadKeyCodes();

	Archive *createArchive();

	// events.cpp
	void processEvents(bool bufferLingoEvents = false);
	void setDraggedSprite(uint16 id);
	void releaseDraggedSprite();
	uint32 getMacTicks();
	void waitForClick();

public:
	Common::RandomSource _rnd;
	Graphics::ManagedSurface *_surface;
	Graphics::MacWindowManager *_wm;

public:
	int _colorDepth;
	unsigned char _key;
	int _keyCode;
	Common::HashMap<int, int> _macKeyCodes;
	int _machineType;
	bool _playbackPaused;
	bool _skipFrameAdvance;

	Common::String _sharedCastFile;

protected:
	Common::Error run() override;

private:
	const DirectorGameDescription *_gameDescription;

	DirectorSound *_soundManager;
	byte *_currentPalette;
	uint16 _currentPaletteLength;
	Lingo *_lingo;

	Stage *_currentStage;

	Graphics::MacPatterns _director3Patterns;
	Graphics::MacPatterns _director3QuickDrawPatterns;

	Common::HashMap<int, PaletteV4 *> _director4Palettes;

	bool _draggingSprite;
	uint16 _draggingSpriteId;
	Common::Point _draggingSpritePos;
};

extern DirectorEngine *g_director;
extern uint32 wmMode;

} // End of namespace Director

#endif
