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

#ifndef PRIVATE_H
#define PRIVATE_H

#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "graphics/managed_surface.h"
#include "graphics/wincursor.h"
#include "video/smk_decoder.h"
#include "video/subtitles.h"

#include "private/grammar.h"

namespace Image {
class ImageDecoder;
}

namespace Graphics {
class ManagedSurface;
}

struct ADGameDescription;

namespace Private {

enum PRIVATEActions {
	kActionSkip,
};

// debug channels
enum {
	kPrivateDebugFunction = 1,
	kPrivateDebugCode,
	kPrivateDebugScript,
};

// sounds

const int kPaperShuffleSound[7] = {32, 33, 34, 35, 36, 37, 39};

// police

const int kPoliceBustVideos[6] = {1, 2, 4, 5, 7, 8};

// points

const int kOriginZero[2] = {0,   0};
const int kOriginOne[2]  = {64, 48};

// vm

extern Gen::VM *Gen::g_vm;

// structs

typedef struct ExitInfo {
	Common::String nextSetting;
	Common::Rect   rect;
	Common::String cursor;
} ExitInfo;

typedef struct MaskInfo {
	Graphics::Surface *surf;
	Common::String nextSetting;
	Common::Point point;
	Symbol *flag1;
	Symbol *flag2;
	Common::String cursor;

	void clear() {
		surf = nullptr;
		flag1 = nullptr;
		flag2 = nullptr;
		nextSetting.clear();
		cursor.clear();
		point = Common::Point();
	}
} MaskInfo;

typedef struct PhoneInfo {
	Common::String sound;
	Symbol *flag;
	int val;
} PhoneInfo;

typedef struct DossierInfo {
	Common::String page1;
	Common::String page2;
} DossierInfo;

typedef struct CursorInfo {
	Common::String name;
	Common::String aname;
	Graphics::Cursor *cursor;
	Graphics::WinCursorGroup *winCursorGroup;
} CursorInfo;

typedef struct MemoryInfo {
	Common::String image;
	Common::String movie;
} MemoryInfo;

typedef struct DiaryPage {
	Common::String locationName;
	Common::Array<MemoryInfo> memories;
	uint locationID;
} DiaryPage;

// funcs

typedef struct FuncTable {
	void (*func)(Private::ArgArray);
	const char *name;
} FunctTable;

typedef Common::HashMap<Common::String, void *> NameToPtr;
extern const FuncTable funcTable[];

// lists

typedef Common::List<ExitInfo> ExitList;
typedef Common::List<MaskInfo> MaskList;
typedef Common::List<Common::String> SoundList;
typedef Common::List<PhoneInfo> PhoneList;
typedef Common::List<Common::String> InvList;
typedef Common::List<Common::Rect *> RectList;

// arrays

typedef Common::Array<DossierInfo> DossierArray;
typedef Common::Array<DiaryPage> DiaryPages;

// hash tables

typedef Common::HashMap<Common::String, bool> PlayedMediaTable;


class PrivateEngine : public Engine {
private:
	Common::RandomSource *_rnd;
	Graphics::PixelFormat _pixelFormat;
	Image::ImageDecoder *_image;
	int _screenW, _screenH;

public:
	PrivateEngine(OSystem *syst, const ADGameDescription *gd);
	~PrivateEngine();

	const ADGameDescription *_gameDescription;
	bool isDemo() const;
	Common::Language _language;
	Common::Platform _platform;

	SymbolMaps maps;

	Audio::SoundHandle _fgSoundHandle;
	Audio::SoundHandle _bgSoundHandle;
	Video::SmackerDecoder *_videoDecoder;
	Video::SmackerDecoder *_pausedVideo;

	Common::Error run() override;
	void restartGame();
	void clearAreas();
	void initializePath(const Common::FSNode &gamePath) override;
	Common::SeekableReadStream *loadAssets();

	// Functions

	NameToPtr _functions;
	void initFuncs();

	// User input
	void selectPauseGame(Common::Point);
	void selectMask(Common::Point);
	void selectExit(Common::Point);
	void selectLoadGame(Common::Point);
	void selectSaveGame(Common::Point);
	void resumeGame();

	// Cursors
	bool cursorPauseMovie(Common::Point);
	bool cursorExit(Common::Point);
	bool cursorMask(Common::Point);

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveAutosaveCurrently() override  {
		return false;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	void ignoreEvents();
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	Common::Path convertPath(const Common::String &);
	void playVideo(const Common::String &);
	void skipVideo();

	void loadSubtitles(const Common::Path &path);
	void adjustSubtitleSize();
	Video::Subtitles *_subtitles;
	bool _useSubtitles;
	bool _sfxSubtitles;

	Graphics::Surface *decodeImage(const Common::String &file, byte **palette, bool *isNewPalette);
	//byte *decodePalette(const Common::String &name);
	void remapImage(uint16 ncolors, const Graphics::Surface *oldImage, const byte *oldPalette, Graphics::Surface *newImage, const byte *currentPalette);
	static uint32 findMaskTransparentColor(const byte *palette, uint32 defaultColor);
	static void swapImageColors(Graphics::Surface *image, byte *palette, uint32 a, uint32 b);
	void loadImage(const Common::String &file, int x, int y);
	void drawScreenFrame(const byte *videoPalette);

	// Cursors
	Graphics::Cursor *_defaultCursor;
	Common::Array<CursorInfo> _cursors;
	Common::String _currentCursor;
	void changeCursor(const Common::String &);
	Common::String getInventoryCursor();
	Common::String getExitCursor();
	void loadCursors();

	// Rendering
	Graphics::ManagedSurface *_compositeSurface;
	Graphics::Surface *loadMask(const Common::String &, int, int, bool);
	void drawMask(Graphics::Surface *);
	void fillRect(uint32, Common::Rect);
	bool inMask(Graphics::Surface *, Common::Point);
	uint32 _transparentColor;
	Common::Rect _screenRect;
	Common::String _framePath;
	Graphics::Surface *_frameImage;
	Graphics::Surface *_mframeImage;
	byte *_framePalette;
	Common::String _nextVS;
	Common::String _currentVS;
	Common::Point _origin;
	void drawScreen();
	bool _needToDrawScreenFrame;

	// settings
	Common::String _nextSetting;
	Common::String _pausedSetting;
	Common::String _currentSetting;
	Common::String getPauseMovieSetting();
	Common::String getGoIntroSetting();
	Common::String getMainDesktopSetting();
	Common::String getPOGoBustMovieSetting();
	Common::String getPoliceBustFromMOSetting();
	Common::String getAlternateGameVariable();
	Common::String getPoliceIndexVariable();
	Common::String getWallSafeValueVariable();

	// movies
	Common::String _nextMovie;
	Common::String _currentMovie;

	// Dossiers
	DossierArray _dossiers;
	uint _dossierSuspect;
	uint _dossierPage;
	MaskInfo _dossierNextSuspectMask;
	MaskInfo _dossierPrevSuspectMask;
	MaskInfo _dossierNextSheetMask;
	MaskInfo _dossierPrevSheetMask;
	bool selectDossierNextSuspect(Common::Point);
	bool selectDossierPrevSuspect(Common::Point);
	bool selectDossierNextSheet(Common::Point);
	bool selectDossierPrevSheet(Common::Point);
	void addDossier(Common::String &page1, Common::String &page2);
	void loadDossier();

	// Police Bust
	bool _policeBustEnabled;
	void startPoliceBust();
	void checkPoliceBust();
	int _numberClicks;
	int _maxNumberClicks;
	int _sirenWarning;
	Common::String _policeBustSetting;

	// Diary
	InvList inventory;
	Common::String _diaryLocPrefix;
	void loadLocations(const Common::Rect &);
	void loadInventory(uint32, const Common::Rect &, const Common::Rect &);
	bool _toTake;
	DiaryPages _diaryPages;
	int _currentDiaryPage;
	ExitInfo _diaryNextPageExit;
	ExitInfo _diaryPrevPageExit;
	bool selectDiaryNextPage(Common::Point mousePos);
	bool selectDiaryPrevPage(Common::Point mousePos);
	void addMemory(const Common::String &path);
	void loadMemories(const Common::Rect &rect, uint rightPageOffset, uint verticalOffset);
	bool selectLocation(const Common::Point &mousePos);
	Common::Array<MaskInfo> _locationMasks;
	Common::Array<MaskInfo> _memoryMasks;
	bool selectMemory(const Common::Point &mousePos);

	// Save/Load games
	MaskInfo _saveGameMask;
	MaskInfo _loadGameMask;

	int _mode;
	bool _modified;

	PlayedMediaTable _playedMovies;
	PlayedMediaTable _playedPhoneClips;
	Common::String _repeatedMovieExit;

	// Masks/Exits
	ExitList _exits;
	MaskList _masks;

	// Sounds
	void playSound(const Common::String &, uint, bool, bool);
	void stopSound(bool);
	bool isSoundActive();
	bool _noStopSounds;

	Common::String getPaperShuffleSound();
	Common::String _globalAudioPath;

	Common::String getTakeSound();
	Common::String getTakeLeaveSound();
	Common::String getLeaveSound();
	Common::String _sirenSound;

	// Radios
	Common::String _infaceRadioPath;
	MaskInfo _AMRadioArea;
	MaskInfo _policeRadioArea;
	MaskInfo _phoneArea;
	Common::String _phonePrefix;
	Common::String _phoneCallSound;
	SoundList _AMRadio;
	SoundList _policeRadio;
	PhoneList _phone;

	Common::String getRandomPhoneClip(const char *, int, int);
	void selectAMRadioArea(Common::Point);
	void selectPoliceRadioArea(Common::Point);
	void selectPhoneArea(Common::Point);
	void checkPhoneCall();

	// Safe
	uint32 _safeColor;
	Common::String _safeNumberPath;
	MaskInfo _safeDigitArea[3];
	Common::Rect _safeDigitRect[3];
	uint32 _safeDigit[3];

	bool selectSafeDigit(Common::Point);
	void addSafeDigit(uint32, Common::Rect*);
	void renderSafeDigit(uint32);

	// Random values
	bool getRandomBool(uint);

	// Timers
	bool installTimer(uint32, Common::String *);
	void removeTimer();

	// VM objects
	RectList _rects; // created by fCRect
};

extern PrivateEngine *g_private;

} // End of namespace Private

#endif
