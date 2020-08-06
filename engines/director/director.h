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

#include "common/rect.h"
#include "common/str-array.h"
#include "common/file.h"

#include "common/hashmap.h"
#include "engines/engine.h"

#include "common/hash-ptr.h"
#include "common/hash-str.h"

#include "director/types.h"
#include "director/util.h"

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
class Window;
class Score;
class Channel;
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
	kDebugNoBytecode	= 1 << 11,
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

struct StartMovie {
	Common::String startMovie;
	int16 startFrame;
};

struct PaletteV4 {
	int id;
	byte *palette;
	int length;

	PaletteV4(int i, byte *p, int l) : id(i), palette(p), length(l) {}
	PaletteV4() : id(0), palette(nullptr), length(0) {}
};

struct MacShape {
	InkType ink;
	byte spriteType;
	byte foreColor;
	byte backColor;
	int lineSize;
	uint pattern;

	Graphics::MacPlotData *pd;
};

// An extension of MacPlotData for interfacing with inks and patterns without
// needing extra surfaces.
struct DirectorPlotData {
	Graphics::MacWindowManager *_wm;
	Graphics::ManagedSurface *dst;

	Common::Rect destRect;
	Common::Point srcPoint;

	Graphics::ManagedSurface *srf;
	MacShape *ms;

	SpriteType sprite;
	InkType ink;
	int colorWhite;
	int colorBlack;
	int alpha;

	int backColor;
	int foreColor;
	bool applyColor;

	void setApplyColor(); // graphics.cpp

	DirectorPlotData(Graphics::MacWindowManager *w, SpriteType s, InkType i, int a, uint b, uint f) : _wm(w), sprite(s), ink(i), alpha(a), backColor(b), foreColor(f) {
		srf = nullptr;
		ms = nullptr;
		dst = nullptr;
		colorWhite = 255;
		colorBlack = 0;
		applyColor = false;
	}

	~DirectorPlotData() {
		delete ms;
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
	uint16 getDescriptionVersion() const;
	uint16 getVersion() const { return _version; }
	void setVersion(uint16 version) { _version = version; }
	Common::Platform getPlatform() const;
	Common::Language getLanguage() const;
	const char *getExtra();
	Common::String getEXEName() const;
	StartMovie getStartMovie() const;
	DirectorSound *getSoundManager() const { return _soundManager; }
	Graphics::MacWindowManager *getMacWindowManager() const { return _wm; }
	Archive *getMainArchive() const;
	Lingo *getLingo() const { return _lingo; }
	Window *getStage() const { return _stage; }
	Window *getCurrentWindow() const { return _currentWindow; }
	void setCurrentWindow(Window *window) { _currentWindow = window; };
	Movie *getCurrentMovie() const;
	void setCurrentMovie(Movie *movie);
	Common::String getCurrentPath() const;

	// graphics.cpp
	bool hasFeature(EngineFeature f) const override;

	void addPalette(int id, byte *palette, int length);
	bool setPalette(int id);
	void setPalette(byte *palette, uint16 count);
	void clearPalettes();
	PaletteV4 *getPalette(int id);
	void loadDefaultPalettes();

	const Common::HashMap<int, PaletteV4> &getLoadedPalettes() { return _loadedPalettes; }

	const Common::FSNode *getGameDataDir() const { return &_gameDataDir; }
	const byte *getPalette() const { return _currentPalette; }
	uint16 getPaletteColorCount() const { return _currentPaletteLength; }

	void loadPatterns();
	uint32 transformColor(uint32 color);
	Graphics::MacPatterns &getPatterns();
	void setCursor(int type);
	void draw();

	void loadKeyCodes();

	Archive *createArchive();

	// events.cpp
	void processEvents();
	uint32 getMacTicks();

public:
	RandomState _rnd;
	Graphics::ManagedSurface *_surface;
	Graphics::MacWindowManager *_wm;

public:
	int _colorDepth;
	Common::HashMap<int, int> _macKeyCodes;
	int _machineType;
	bool _playbackPaused;
	bool _skipFrameAdvance;
	bool _centerStage;

	Common::HashMap<Common::String, Archive *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _openResFiles;
	Common::String _sharedCastFile;

protected:
	Common::Error run() override;

private:
	const DirectorGameDescription *_gameDescription;
	Common::FSNode _gameDataDir;

	DirectorSound *_soundManager;
	byte *_currentPalette;
	uint16 _currentPaletteLength;
	Lingo *_lingo;
	uint16 _version;

	Window *_stage;
	Datum *_windowList; // Lingo list
	Window *_currentWindow;

	Graphics::MacPatterns _director3Patterns;
	Graphics::MacPatterns _director3QuickDrawPatterns;

	Common::HashMap<int, PaletteV4> _loadedPalettes;
};

extern DirectorEngine *g_director;
extern uint32 wmMode;

} // End of namespace Director

#endif
