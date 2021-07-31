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
	Levels _levels;
	Common::HashMap<Common::String, int> _levelState;
	void resetLevelState();
	bool checkLevelCompleted();
	void runMis(Common::String name);

	void restartGame();
	void clearAreas();
	void initializePath(const Common::FSNode &gamePath) override;
	void loadMis(Common::String filename);
	LibData loadLib(char *filename);

	// User input
	void clickedHotspot(Common::Point);
	bool hoverHotspot(Common::Point);

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

	void syncGameStream(Common::Serializer &s);

	Common::String convertPath(const Common::String &);
	void playVideo(const Common::String &);
	void skipVideo();

	Graphics::Surface *decodeImage(const Common::String &file);
	Graphics::Surface *decodeFrame(const Common::String &name, int frame, bool convert = true);
	void loadImage(const Common::String &file, int x, int y);

	// Cursors
	void changeCursor(const Common::String &, uint32);

	// Actions
	void runIntro();
	void runMenu(Hotspots hs);
    void runBackground(const Hotspot h, Background *a);
	void runOverlay(const Hotspot h, Overlay *a);
	void runMice(const Hotspot h, Mice *a);
	void runEscape(const Hotspot h, Escape *a);
	void runQuit(const Hotspot h, Quit *a);
	void runCutscene(const Hotspot h, Cutscene *a);
	void runPlay(const Hotspot h, Play *a);
	void runWalN(const Hotspot h, WalN *a);
	void runGlobal(const Hotspot h, Global *a);

	Graphics::ManagedSurface *_compositeSurface;
	uint32 _transparentColor;
	Common::Rect screenRect;
	void drawScreen();

	// intros
	Common::HashMap<Common::String, Movies> _intros;

	// settings 
	Common::String _nextSetting;
	Common::String _currentSetting;

	// hotspots
	Hotspots *_nextHotsToAdd;
	Hotspots *_nextHotsToRemove;
	HotspotsStack stack;

	// Movies
	Movies _nextMoviesToPlay;
	Common::List<Common::Point> _nextMoviesPositions;
	Common::List<bool> _nextMoviesScales;
	Common::Point _moviePosition;
	bool _movieScale;
	Common::String _currentMovie;

	// Sounds
	void playSound(const Common::String &, uint, bool, bool);
	void stopSound(bool);
	bool isSoundActive();
	bool _noStopSounds;

	// Timers
	bool installTimer(uint32, Common::String *);
	void removeTimer();
};

} // End of namespace Hypno

#endif
