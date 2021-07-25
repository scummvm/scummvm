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

#ifndef HYPNO_H
#define HYPNO_H

#include "common/installshieldv3_archive.h"
#include "common/random.h"
#include "common/str-array.h"
#include "common/array.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "graphics/managed_surface.h"
#include "video/smk_decoder.h"

#include "hypno/grammar.h"

namespace Image {
class ImageDecoder;
}

namespace Graphics {
class ManagedSurface;
}

struct ADGameDescription;

namespace Hypno {

// debug channels
enum {
	kHypnoDebugFunction = 1 << 0,
	kHypnoDebugCode = 1 << 1,
	kHypnoDebugScript = 1 << 2
};


typedef Common::Array<byte> ByteArray;
typedef struct LibData {
	Common::Array<Common::String> filenames;
	ByteArray data;
} LibData;

class HypnoEngine : public Engine {
private:
	Common::RandomSource *_rnd;
	Graphics::PixelFormat _pixelFormat;
	Image::ImageDecoder *_image;
	int _screenW, _screenH;

public:
	HypnoEngine(OSystem *syst, const ADGameDescription *gd);
	~HypnoEngine();

	const ADGameDescription *_gameDescription;
	bool isDemo() const;
	Common::Language _language;
	Common::Platform _platform;

	Audio::SoundHandle _fgSoundHandle;
	Audio::SoundHandle _bgSoundHandle;
	Video::SmackerDecoder *_videoDecoder;
	Common::InstallShieldV3 _installerArchive;

	Common::Error run() override;
	void restartGame();
	void clearAreas();
	void initializePath(const Common::FSNode &gamePath) override;
	void loadMis(Common::String filename);
	LibData loadLib(char *filename);

	// Functions

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

	//bool hasFeature(EngineFeature f) const override;
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
	//Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	//Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void syncGameStream(Common::Serializer &s);

	Common::String convertPath(const Common::String &);
	void playVideo(const Common::String &);
	void skipVideo();

	Graphics::Surface *decodeImage(const Common::String &file);
	Graphics::Surface *decodeFrame(const Common::String &name, int frame, bool convert = true);
	void loadImage(const Common::String &file, int x, int y);
	void drawScreenFrame();

	// Cursors
	void changeCursor(const Common::String &, uint32);
	Common::String getInventoryCursor();
	Common::String getExitCursor();

	// Actions
	void prepareHotspots(Hotspots hs);
    void runBackground(const Hotspot h, Background *a);
	void runOverlay(const Hotspot h, Overlay *a);
	void runMice(const Hotspot h, Mice *a);

	Graphics::ManagedSurface *_compositeSurface;
	Graphics::Surface *loadMask(const Common::String &, int, int, bool);
	void drawMask(Graphics::Surface *);
	void fillRect(uint32, Common::Rect);
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
	Common::String _diaryLocPrefix;
	void loadLocations(const Common::Rect &);
	void loadInventory(uint32, const Common::Rect &, const Common::Rect &);
	bool _toTake;

	// Save/Load games
	int _mode;
	bool _modified;
	Common::String _repeatedMovieExit;

	// Masks/Exits

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
	Common::String _phonePrefix;
	Common::String _phoneCallSound;

	Common::String getRandomPhoneClip(const char *, int, int);
	void selectAMRadioArea(Common::Point);
	void selectPoliceRadioArea(Common::Point);
	void selectPhoneArea(Common::Point);
	void checkPhoneCall();

	// Safe
	uint32 _safeColor;
	Common::String _safeNumberPath;
	Common::Rect _safeDigitRect[3];

	bool selectSafeDigit(Common::Point);
	void addSafeDigit(uint32, Common::Rect*);
	void renderSafeDigit(uint32);

	// Random values
	bool getRandomBool(uint);

	// Timers
	bool installTimer(uint32, Common::String *);
	void removeTimer();
};

//extern PrivateEngine *g_private;

} // End of namespace Hypno

#endif
