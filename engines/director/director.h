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

#include "common/hash-ptr.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/types.h"
#include "director/util.h"
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
typedef Common::Array<const byte *> MacPatterns;

class ManagedSurface;
}

namespace Director {

class Archive;
class MacArchive;
class Cast;
class Debugger;
class DirectorSound;
class Lingo;
class Movie;
class Window;
struct Picture;
class Score;
class Channel;
class CastMember;
class Stxt;

enum {
	kDebugLingoExec	= 1,
	kDebugCompile,
	kDebugLoading,
	kDebugImages,
	kDebugText,
	kDebugEvents,
	kDebugParse,
	kDebugCompileOnly,
	kDebugSlow,
	kDebugFast,
	kDebugNoLoop,
	kDebugNoBytecode,
	kDebugFewFramesOnly,
	kDebugPreprocess,
	kDebugScreenshot,
	kDebugDesktop,
	kDebug32bpp,
	kDebugEndVideo,
	kDebugLingoStrict,
	kDebugSound,
	kDebugConsole,
	kDebugXObj,
	kDebugLingoThe,
	kDebugImGui,
	kDebugPaused,
	kDebugPauseOnLoad,
};

enum {
	GF_DESKTOP = 1 << 0,
	GF_640x480 = 1 << 1,
	GF_32BPP   = 1 << 2,
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
	CastMemberID id;
	const byte *palette;
	int length;

	PaletteV4(CastMemberID i, const byte *p, int l) : id(i), palette(p), length(l) {}
	PaletteV4() : id(), palette(nullptr), length(0) {}
};

struct MacShape {
	InkType ink;
	byte spriteType;
	uint32 foreColor;
	uint32 backColor;
	int lineSize;
	uint pattern;

	Picture *tile;
	const Common::Rect *tileRect;

	Graphics::MacPlotData *pd;
};

struct PatternTile {
	Picture *img = nullptr;
	Common::Rect rect;

	~PatternTile();
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
	uint32 getGameFlags() const;
	Common::String getTargetName() { return _targetName; }
	const char *getExtra();
	Common::String getRawEXEName() const;
	Common::String getEXEName() const;
	StartMovie getStartMovie() const;
	void parseOptions();
	Graphics::MacWindowManager *getMacWindowManager() const { return _wm; }
	Archive *getMainArchive() const;
	Lingo *getLingo() const { return _lingo; }
	Window *getStage() const { return _stage; }
	Window *getCurrentWindow() const { return _currentWindow; }
	void setCurrentWindow(Window *window);
	Window *getCursorWindow() const { return _cursorWindow; }
	void setCursorWindow(Window *window) { _cursorWindow = window; }
	Movie *getCurrentMovie() const;
	void setCurrentMovie(Movie *movie);
	Common::String getCurrentPath() const;
	Common::String getCurrentAbsolutePath();
	Common::Path getStartupPath() const;

	// graphics.cpp
	bool hasFeature(EngineFeature f) const override;

	void addPalette(CastMemberID &id, const byte *palette, int length);
	bool setPalette(const CastMemberID &id);
	void setPalette(const byte *palette, uint16 count);
	void shiftPalette(int startIndex, int endIndex, bool reverse);
	void clearPalettes();
	PaletteV4 *getPalette(const CastMemberID &id);
	bool hasPalette(const CastMemberID &id);
	void loadDefaultPalettes();

	const Common::HashMap<CastMemberID, PaletteV4> &getLoadedPalettes() { return _loadedPalettes; }
	const Common::HashMap<CastMemberID, PaletteV4> &getLoaded16Palettes() { return _loaded16Palettes; }
	const PaletteV4 &getLoaded4Palette() { return _loaded4Palette; }

	const Common::FSNode *getGameDataDir() const { return &_gameDataDir; }
	const byte *getPalette() const { return _currentPalette; }
	uint16 getPaletteColorCount() const { return _currentPaletteLength; }

	void loadPatterns();
	Picture *getTile(int num);
	const Common::Rect &getTileRect(int num);
	uint32 transformColor(uint32 color);
	Graphics::MacPatterns &getPatterns();
	void setCursor(DirectorCursor type);
	void draw();

	Graphics::MacDrawPixPtr getInkDrawPixel();
	uint32 getColorBlack();
	uint32 getColorWhite();

	void loadKeyCodes();
	void setMachineType(int machineType);
	Common::CodePage getPlatformEncoding();

	Archive *createArchive();
	Archive *openArchive(const Common::Path &movie);
	void addArchiveToOpenList(const Common::Path &path);
	Archive *loadEXE(const Common::Path &movie);
	Archive *loadEXEv3(Common::SeekableReadStream *stream);
	Archive *loadEXEv4(Common::SeekableReadStream *stream);
	Archive *loadEXEv5(Common::SeekableReadStream *stream);
	Archive *loadEXEv7(Common::SeekableReadStream *stream);
	Archive *loadEXERIFX(Common::SeekableReadStream *stream, uint32 offset);
	Archive *loadMac(const Common::Path &movie);

	bool desktopEnabled();

	// events.cpp
	bool pollEvent(Common::Event &event);
	bool processEvents(bool captureClick = false, bool skipWindowManager = false);
	void processEventQUIT();
	uint32 getMacTicks();
	Common::Array<Common::Event> _injectedEvents;

	// game-quirks.cpp
	void gameQuirks(const char *target, Common::Platform platform);
	void loadSlowdownCooloff(uint32 delay = 2000);

	void delayMillis(uint32 delay);

public:
	RandomState _rnd;
	Graphics::MacWindowManager *_wm;
	Graphics::PixelFormat _pixelformat;

	uint32 _debugDraw = 0;
	int _defaultVolume = 255;

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

	// Owner of all Archive objects.
	Common::HashMap<Common::Path, Archive *, Common::Path::IgnoreCaseAndMac_Hash, Common::Path::IgnoreCaseAndMac_EqualTo> _allSeenResFiles;
	// Handles to resource files that were opened by OpenResFile.
	Common::HashMap<Common::Path, Archive *, Common::Path::IgnoreCaseAndMac_Hash, Common::Path::IgnoreCaseAndMac_EqualTo> _openResFiles;
	// List of all currently open resource files
	Common::List<Common::Path> _allOpenResFiles;

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
	CastMemberID _lastPalette;

	// used for quirks
	byte _fpsLimit;
	TimeDate _forceDate;
	uint32 _loadSlowdownFactor;
	uint32 _loadSlowdownCooldownTime;

private:
	byte _currentPalette[768];
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

	Common::HashMap<CastMemberID, PaletteV4> _loadedPalettes;
	Common::HashMap<CastMemberID, PaletteV4> _loaded16Palettes;
	PaletteV4 _loaded4Palette;

	Graphics::ManagedSurface *_surface;

	StartOptions _options;

public:
	int _tickBaseline;
	Common::Path _traceLogFile;

	uint16 _framesRan = 0; // used by kDebugFewFramesOnly
	bool _noFatalLingoError = false;

	bool _firstMovie = true;
};

// An extension of MacPlotData for interfacing with inks and patterns without
// needing extra surfaces.
struct DirectorPlotData {
	DirectorEngine *d = nullptr;
	Graphics::ManagedSurface *dst = nullptr;

	Common::Rect destRect;
	Common::Point srcPoint;

	Graphics::ManagedSurface *srf = nullptr;
	MacShape *ms = nullptr;

	SpriteType sprite = kInactiveSprite;
	bool oneBitImage = false;
	InkType ink = kInkTypeCopy;
	uint32 colorWhite;
	uint32 colorBlack;
	int alpha = 0;

	uint32 backColor;
	uint32 foreColor;
	bool applyColor = false;

	// graphics.cpp
	void setApplyColor();
	uint32 preprocessColor(uint32 src);
	void inkBlitShape(Common::Rect &srcRect);
	void inkBlitSurface(Common::Rect &srcRect, const Graphics::Surface *mask);

	DirectorPlotData(DirectorEngine *d_, SpriteType s, InkType i, int a, uint32 b, uint32 f) : d(d_), sprite(s), ink(i), alpha(a), backColor(b), foreColor(f) {
		colorWhite = d->_wm->_colorWhite;
		colorBlack = d->_wm->_colorBlack;
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
