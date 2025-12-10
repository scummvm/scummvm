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

namespace Common {
class Archive;
}

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

	void clear() {
		nextSetting.clear();
		rect.setEmpty();
		cursor.clear();
	}
} ExitInfo;

typedef struct MaskInfo {
	Graphics::Surface *surf;
	Common::String nextSetting;
	Common::Point point;
	Symbol *flag1;
	Symbol *flag2;
	Common::String cursor;
	Common::String inventoryItem;
	bool useBoxCollision;
	Common::Rect box;

	MaskInfo() {
		clear();
	}

	void clear() {
		surf = nullptr;
		useBoxCollision = false;
		box = Common::Rect();
		flag1 = nullptr;
		flag2 = nullptr;
		nextSetting.clear();
		cursor.clear();
		point = Common::Point();
		inventoryItem.clear();
	}
} MaskInfo;

enum PhoneStatus : byte {
	kPhoneStatusWaiting,
	kPhoneStatusAvailable,
	kPhoneStatusCalling,
	kPhoneStatusMissed,
	kPhoneStatusAnswered
};

typedef struct PhoneInfo {
	Common::String name;
	bool once;
	int startIndex;
	int endIndex;
	Common::String flagName;
	int flagValue;
	PhoneStatus status;
	int callCount;
	uint32 soundIndex;
	Common::Array<Common::String> sounds;
} PhoneInfo;

typedef struct RadioClip {
	Common::String name;
	bool played;
	int priority;
	int disabledPriority1; // 0 == none
	bool exactPriorityMatch1;
	int disabledPriority2; // 0 == none
	bool exactPriorityMatch2;
	Common::String flagName;
	int flagValue;
} RadioClip;

typedef struct Radio {
	Common::String path;
	Common::Array<RadioClip> clips;
	int channels[3];

	Radio() {
		clear();
	}

	void clear() {
		clips.clear();
		for (uint i = 0; i < ARRAYSIZE(channels); i++) {
			channels[i] = -1;
		}
	}
} Radio;

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
	int locationID;
} DiaryPage;

typedef struct InventoryItem {
	Common::String diaryImage;
	Common::String flag;
} InventoryItem;

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
typedef Common::List<PhoneInfo> PhoneList;
typedef Common::List<InventoryItem> InvList;
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
	bool _shouldHighlightMasks;
	bool _highlightMasks;
	PrivateEngine(OSystem *syst, const ADGameDescription *gd);
	~PrivateEngine();

	const ADGameDescription *_gameDescription;
	bool isDemo() const;
	Common::Language _language;
	Common::Platform _platform;

	SymbolMaps maps;

	Audio::SoundHandle _fgSoundHandle;
	Audio::SoundHandle _bgSoundHandle;
	Audio::SoundHandle _phoneCallSoundHandle;
	Video::SmackerDecoder *_videoDecoder;
	Video::SmackerDecoder *_pausedVideo;

	Common::Error run() override;
	void restartGame();
	void clearAreas();
	void initializePath(const Common::FSNode &gamePath) override;
	Common::SeekableReadStream *loadAssets();
	Common::Archive *loadMacInstaller();

	// Functions

	NameToPtr _functions;
	void initFuncs();

	// User input
	void selectPauseGame(Common::Point);
	bool selectMask(Common::Point);
	bool selectExit(Common::Point);
	bool selectLoadGame(Common::Point);
	bool selectSaveGame(Common::Point);
	void resumeGame();

	// Cursors
	void updateCursor(Common::Point);
	bool cursorPauseMovie(Common::Point);
	bool cursorExit(Common::Point);
	bool cursorSafeDigit(Common::Point);
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

	static Common::Path convertPath(const Common::String &name);
	static Common::String getVideoViewScreen(Common::String video);
	void playVideo(const Common::String &);
	void skipVideo();
	void destroyVideo();

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
	void loadMaskAndInfo(MaskInfo *m, const Common::String &name, int x, int y, bool drawn);
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
	Common::String getDiaryTOCSetting();
	Common::String getDiaryMiddleSetting();
	Common::String getDiaryLastPageSetting();
	Common::String getPOGoBustMovieSetting();
	Common::String getPoliceBustFromMOSetting();
	Common::String getListenToPhoneSetting();
	Common::String getAlternateGameVariable();
	Common::String getPoliceIndexVariable();
	Common::String getWallSafeValueVariable();
	Common::String getPoliceArrivedVariable();
	Common::String getBeenDowntownVariable();
	Common::String getPoliceStationLocation();
	const char *getSymbolName(const char *name, const char *strippedName, const char *demoName = nullptr);

	// movies
	Common::String _nextMovie;
	Common::String _currentMovie;

	// Dossiers
	DossierArray _dossiers;
	uint _dossierSuspect;
	uint _dossierPage;
	MaskInfo _dossierPageMask;
	MaskInfo _dossierNextSuspectMask;
	MaskInfo _dossierPrevSuspectMask;
	MaskInfo _dossierNextSheetMask;
	MaskInfo _dossierPrevSheetMask;
	bool selectDossierPage(Common::Point);
	bool selectDossierNextSuspect(Common::Point);
	bool selectDossierPrevSuspect(Common::Point);
	bool selectDossierNextSheet(Common::Point);
	bool selectDossierPrevSheet(Common::Point);
	void addDossier(Common::String &page1, Common::String &page2);
	void loadDossier();

	// Police Bust
	bool _policeBustEnabled;
	bool _policeSirenPlayed;
	int _numberOfClicks;
	int _numberClicksAfterSiren;
	int _policeBustMovieIndex;
	Common::String _policeBustMovie;
	Common::String _policeBustPreviousSetting;
	void resetPoliceBust();
	void startPoliceBust();
	void stopPoliceBust();
	void wallSafeAlarm();
	void completePoliceBust();
	void checkPoliceBust();

	// Diary
	InvList inventory;
	bool inInventory(const Common::String &bmp) const;
	void addInventory(const Common::String &bmp, Common::String &flag);
	void removeInventory(const Common::String &bmp);
	void removeRandomInventory();
	Common::String _diaryLocPrefix;
	void loadLocations(const Common::Rect &);
	void loadInventory(uint32, const Common::Rect &, const Common::Rect &);
	bool _toTake;
	bool _haveTakenItem;
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
	void setLocationAsVisited(Symbol *location);
	int getMaxLocationValue();

	// Save/Load games
	MaskInfo _saveGameMask;
	MaskInfo _loadGameMask;

	int _mode;
	bool _modified;

	PlayedMediaTable _playedMovies;
	Common::String _repeatedMovieExit;

	// Masks/Exits
	ExitList _exits;
	MaskList _masks;

	// Sounds
	void playSound(const Common::String &, uint, bool, bool);
	void playPhoneCallSound();
	void stopSound(bool);
	bool isSoundActive();
	void waitForSoundToStop();
	bool _noStopSounds;
	Common::String _backgroundSound;
	Common::String _pausedBackgroundSound;

	Common::String getPaperShuffleSound();
	Common::String _globalAudioPath;

	Common::String getTakeSound();
	Common::String getTakeLeaveSound();
	Common::String getLeaveSound();
	Common::String _sirenSound;

	// Radios
	MaskInfo _AMRadioArea;
	MaskInfo _policeRadioArea;
	Radio _AMRadio;
	Radio _policeRadio;
	void addRadioClip(
		Radio &radio, const Common::String &name, int priority,
		int disabledPriority1, bool exactPriorityMatch1,
		int disabledPriority2, bool exactPriorityMatch2,
		const Common::String &flagName, int flagValue);
	void initializeAMRadioChannels(uint clipCount);
	void initializePoliceRadioChannels();
	void disableRadioClips(Radio &radio, int priority);
	void playRadio(Radio &radio, bool randomlyDisableClips);
	bool selectAMRadioArea(Common::Point);
	bool selectPoliceRadioArea(Common::Point);

	// Phone
	MaskInfo _phoneArea;
	Common::String _phonePrefix;
	Common::String _phoneCallSound;
	PhoneList _phones;
	void addPhone(const Common::String &name, bool once, int startIndex, int endIndex, const Common::String &flagName, int flagValue);
	void initializePhoneOnDesktop();
	void checkPhoneCall();
	bool cursorPhoneArea(Common::Point mousePos);
	bool selectPhoneArea(Common::Point mousePos);

	// Safe
	Common::String _safeNumberPath;
	MaskInfo _safeDigitArea[3];
	Common::Rect _safeDigitRect[3];

	void initializeWallSafeValue();
	bool selectSafeDigit(Common::Point);
	void addSafeDigit(uint32, Common::Rect*);
	int getSafeDigit(uint32 d);
	void incrementSafeDigit(uint32 d);

	// Random values
	bool getRandomBool(uint);

	// Timer
	Common::String _timerSetting;
	Common::String _timerSkipSetting;
	uint32 _timerStartTime;
	uint32 _timerDelay;
	void setTimer(uint32 duration, const Common::String &setting, const Common::String &skipSetting);
	void clearTimer();
	void skipTimer();
	void checkTimer();

	// VM objects
	RectList _rects; // created by fCRect
};

extern PrivateEngine *g_private;

} // End of namespace Private

#endif
