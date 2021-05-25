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

#ifndef PRIVATE_H
#define PRIVATE_H

#include "common/installshieldv3_archive.h"
#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "graphics/managed_surface.h"
#include "video/smk_decoder.h"

#include "private/grammar.h"

namespace Image {
class ImageDecoder;
}

namespace Graphics {
class ManagedSurface;
}

struct ADGameDescription;

namespace Private {

// debug channels
enum {
	kPrivateDebugFunction = 1 << 0,
	kPrivateDebugCode = 1 << 1,
	kPrivateDebugScript = 1 << 2
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

// arrays

typedef Common::Array<DossierInfo> DossierArray;

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

	SymbolMaps maps;

	Audio::SoundHandle _fgSoundHandle;
	Audio::SoundHandle _bgSoundHandle;
	Video::SmackerDecoder *_videoDecoder;
	Common::InstallShieldV3 _installerArchive;

	Common::Error run() override;
	void restartGame();
	void clearAreas();
	void initializePath(const Common::FSNode &gamePath) override;

	// Functions

	NameToPtr _functions;
	void initFuncs();

	// User input
	void selectPauseMovie(Common::Point);
	void selectMask(Common::Point);
	void selectExit(Common::Point);
	void selectLoadGame(Common::Point);
	void selectSaveGame(Common::Point);

	// Cursors
	bool cursorPauseMovie(Common::Point);
	bool cursorExit(Common::Point);
	bool cursorMask(Common::Point);

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveAutosaveCurrently() override  {
		return false;
	}
	bool canSaveGameStateCurrently() override {
		return true;
	}

	void ignoreEvents();
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void syncGameStream(Common::Serializer &s);

	Common::String convertPath(const Common::String &);
	void playVideo(const Common::String &);
	void skipVideo();

	Graphics::Surface *decodeImage(const Common::String &file);
	void loadImage(const Common::String &file, int x, int y);
	void drawScreenFrame();

	void changeCursor(const Common::String &);

	// Rendering
	Graphics::ManagedSurface *_compositeSurface;
	Graphics::Surface *loadMask(const Common::String &, int, int, bool);
	void drawMask(Graphics::Surface *);
	bool inMask(Graphics::Surface *, Common::Point);
	uint32 _transparentColor;
	Common::Rect screenRect;
	Common::String _framePath;
	Graphics::Surface *_frame;
	Common::String _nextVS;
	Common::Point _origin;
	void drawScreen();

	// settings
	Common::String _nextSetting;
	Common::String _pausedSetting;
	Common::String _currentSetting;

	// movies
	Common::String _nextMovie;
	Common::String _currentMovie;

	// Dossiers
	DossierArray _dossiers;
	unsigned int _dossierSuspect;
	unsigned int _dossierPage;
	MaskInfo _dossierNextSuspectMask;
	MaskInfo _dossierPrevSuspectMask;
	MaskInfo _dossierNextSheetMask;
	MaskInfo _dossierPrevSheetMask;
	bool selectDossierNextSuspect(Common::Point);
	bool selectDossierPrevSuspect(Common::Point);
	bool selectDossierNextSheet(Common::Point);
	bool selectDossierPrevSheet(Common::Point);
	void loadDossier();

	// Police Bust
	void policeBust();
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

	// Random values
	bool getRandomBool(uint);

	// Timers
	bool installTimer(uint32, Common::String *);
	void removeTimer();
};

extern PrivateEngine *g_private;

} // End of namespace Private

#endif
