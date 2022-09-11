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

#ifndef DIRECTOR_DIRECTOR_H
#define DIRECTOR_DIRECTOR_H

#include "backends/audiocd/audiocd.h"

#include "common/file.h"
#include "common/hashmap.h"
#include "common/hash-ptr.h"
#include "common/hash-str.h"
#include "common/rect.h"
#include "common/str-array.h"

#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/types.h"
#include "director/util.h"
#include "director/debugger.h"
#include "director/detection.h"

namespace Common {
class MacResManager;
class SeekableReadStream;
class SeekableReadStreamEndian;
}

namespace Graphics {
class MacWindowManager;
struct MacPlotData;
struct WinCursorGroup;
typedef Common::Array<byte *> MacPatterns;

class ManagedSurface;
}

namespace Director {

class Archive;
class Cast;
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
	kDebugDesktop		= 1 << 15,
	kDebug32bpp			= 1 << 16,
	kDebugEndVideo		= 1 << 17,
	kDebugLingoStrict	= 1 << 18,
	kDebugSound			= 1 << 19,
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

struct StartOptions {
	StartMovie startMovie;
	Common::String startupPath;
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
	uint32 foreColor;
	uint32 backColor;
	int lineSize;
	uint pattern;

	Image::ImageDecoder *tile;
	const Common::Rect *tileRect;

	Graphics::MacPlotData *pd;
};

struct PatternTile {
	Image::ImageDecoder *img;
	Common::Rect rect;
};

const int SCALE_THRESHOLD = 0x100;

class DirectorEngine : public ::Engine {
public:
	DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc);
	~DirectorEngine() override;

	// Detection related functions

	DirectorGameGID getGameGID() const;
	const char *getGameId() const;
	uint16 getDescriptionVersion() const;
	uint16 getVersion() const { return _version; }
	void setVersion(uint16 version);
	Common::Platform getPlatform() const;
	Common::Language getLanguage() const;
	Common::String getTargetName() { return _targetName; }
	const char *getExtra();
	Common::String getEXEName() const;
	StartMovie getStartMovie() const;
	void parseOptions();
	Graphics::MacWindowManager *getMacWindowManager() const { return _wm; }
	Archive *getMainArchive() const;
	Lingo *getLingo() const { return _lingo; }
	Window *getStage() const { return _stage; }
	Window *getCurrentWindow() const { return _currentWindow; }
	void setCurrentWindow(Window *window) { _currentWindow = window; };
	Window *getCursorWindow() const { return _cursorWindow; }
	void setCursorWindow(Window *window) { _cursorWindow = window; }
	Movie *getCurrentMovie() const;
	void setCurrentMovie(Movie *movie);
	Common::String getCurrentPath() const;
	Common::String getStartupPath() const;

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
	Image::ImageDecoder *getTile(int num);
	const Common::Rect &getTileRect(int num);
	uint32 transformColor(uint32 color);
	Graphics::MacPatterns &getPatterns();
	void setCursor(DirectorCursor type);
	void draw();

	Graphics::MacDrawPixPtr getInkDrawPixel();

	void loadKeyCodes();
	Common::CodePage getPlatformEncoding();

	Archive *createArchive();

	bool desktopEnabled();

	// events.cpp
	bool processEvents(bool captureClick = false);
	void processEventQUIT();
	uint32 getMacTicks();

	// game-quirks.cpp
	void gameQuirks(const char *target, Common::Platform platform);

public:
	RandomState _rnd;
	Graphics::MacWindowManager *_wm;
	Graphics::PixelFormat _pixelformat;
	AudioCDManager::Status _cdda_status;

public:
	int _colorDepth;
	Common::HashMap<int, int> _KeyCodes;
	int _machineType;
	bool _playbackPaused;
	bool _skipFrameAdvance;
	bool _centerStage;
	char _dirSeparator;
	bool _fixStageSize;
	Common::Rect _fixStageRect;
	Common::List<Common::String> _extraSearchPath;

	Common::HashMap<Common::String, Archive *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _openResFiles;
	Common::Array<Graphics::WinCursorGroup *> _winCursor;

protected:
	Common::Error run() override;

public:
	const DirectorGameDescription *_gameDescription;
	Common::FSNode _gameDataDir;
	CastMemberID *_clipBoard;
	uint32 _wmMode;
	uint16 _wmWidth;
	uint16 _wmHeight;

private:
	byte *_currentPalette;
	uint16 _currentPaletteLength;
	Lingo *_lingo;
	uint16 _version;

	Window *_stage;
	Datum *_windowList; // Lingo list
	Window *_currentWindow;
	Window *_cursorWindow;

	Graphics::MacPatterns _director3Patterns;
	Graphics::MacPatterns _director3QuickDrawPatterns;
	PatternTile _builtinTiles[kNumBuiltinTiles];

	Common::HashMap<int, PaletteV4> _loadedPalettes;

	Graphics::ManagedSurface *_surface;

	StartOptions _options;

public:
	int _tickBaseline;
	Common::String _traceLogFile;
};

// An extension of MacPlotData for interfacing with inks and patterns without
// needing extra surfaces.
struct DirectorPlotData {
	DirectorEngine *d;
	Graphics::ManagedSurface *dst;

	Common::Rect destRect;
	Common::Point srcPoint;

	Graphics::ManagedSurface *srf;
	MacShape *ms;

	SpriteType sprite;
	InkType ink;
	uint32 colorWhite;
	uint32 colorBlack;
	int alpha;

	uint32 backColor;
	uint32 foreColor;
	bool applyColor;

	// graphics.cpp
	void setApplyColor();
	uint32 preprocessColor(uint32 src);
	void inkBlitShape(Common::Rect &srcRect);
	void inkBlitSurface(Common::Rect &srcRect, const Graphics::Surface *mask);
	void inkBlitStretchSurface(Common::Rect &srcRect, const Graphics::Surface *mask);

	DirectorPlotData(DirectorEngine *d_, SpriteType s, InkType i, int a, uint32 b, uint32 f) : d(d_), sprite(s), ink(i), alpha(a), backColor(b), foreColor(f) {
		srf = nullptr;
		ms = nullptr;
		dst = nullptr;
		colorWhite = d->_wm->_colorWhite;
		colorBlack = d->_wm->_colorBlack;
		applyColor = false;
	}

	DirectorPlotData(const DirectorPlotData &old) : d(old.d), sprite(old.sprite),
	                                                ink(old.ink), alpha(old.alpha),
	                                                backColor(old.backColor), foreColor(old.foreColor),
	                                                srf(old.srf), dst(old.dst),
	                                                destRect(old.destRect), srcPoint(old.srcPoint),
	                                                colorWhite(old.colorWhite), colorBlack(old.colorBlack),
	                                                applyColor(old.applyColor) {
		if (old.ms) {
			ms = new MacShape(*old.ms);
		} else {
			ms = nullptr;
		}
	}

	DirectorPlotData &operator=(const DirectorPlotData &);

	~DirectorPlotData() {
		delete ms;
	}
};

extern DirectorEngine *g_director;
extern Debugger *g_debugger;

} // End of namespace Director

#endif
