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

#include "common/array.h"
#include "common/installshieldv3_archive.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/str-array.h"
#include "engines/engine.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"

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
typedef struct FileData {
	Common::String name;
	ByteArray data;
} FileData;

typedef Common::Array<FileData> LibData;

class HypnoEngine : public Engine {
private:
	Common::RandomSource *_rnd;
	Image::ImageDecoder *_image;
	int _screenW, _screenH;

public:
	HypnoEngine(OSystem *syst, const ADGameDescription *gd);
	~HypnoEngine();

	const ADGameDescription *_gameDescription;
	bool isDemo() const;
	Common::Language _language;
	Common::Platform _platform;

	Audio::SoundHandle _soundHandle;
	Common::InstallShieldV3 _installerArchive;

	Common::Error run() override;
	Levels _levels;
	LibData _soundFiles;
	LibData _fontFiles;
	Common::HashMap<Common::String, int> _levelState;
	void resetLevelState();
	bool checkLevelCompleted();
	void runLevel(Common::String name);
	void runScene(Scene scene);
	void runArcade(ArcadeShooting arc);

	void restartGame();
	void clearAreas();
	void initializePath(const Common::FSNode &gamePath) override;
	virtual void loadAssets();
	void parseScene(Common::String prefix, Common::String filename);
	void parseArcadeShooting(Common::String prefix, Common::String name, Common::String data);
	ShootSequence parseShootList(Common::String name, Common::String data);
	void loadLib(Common::String filename, LibData &r);
	Common::HashMap<Common::String, Common::String> _assets;

	// User input
	void clickedHotspot(Common::Point);
	bool clickedShoot(Common::Point);
	bool hoverHotspot(Common::Point);

	// Cursors
	bool cursorPauseMovie(Common::Point);
	bool cursorExit(Common::Point);
	bool cursorMask(Common::Point);

	//bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveAutosaveCurrently() override {
		return false;
	}
	bool canSaveGameStateCurrently() override {
		return true;
	}

	void syncGameStream(Common::Serializer &s);

	Common::String _prefixDir;
	Common::String convertPath(const Common::String &);
	void playVideo(MVideo &video);
	void skipVideo(MVideo &video);

	Graphics::Surface *decodeFrame(const Common::String &name, int frame, bool convert = true);
	void loadImage(const Common::String &file, int x, int y, bool transparent);
	void drawImage(Graphics::Surface &image, int x, int y, bool transparent);

	// Cursors
	Common::String _defaultCursor;
	void disableCursor();
	void defaultCursor();
	void changeCursor(const Common::String &, uint32);

	// Actions
	void runIntro(MVideo &video);
	void runMenu(Hotspots hs);
	void runBackground(Background *a);
	void runOverlay(Overlay *a);
	void runMice(Mice *a);
	void runEscape(Escape *a);
	void runQuit(Quit *a);
	void runCutscene(Cutscene *a);
	void runPlay(Play *a);
	void runAmbient(Ambient *a);
	void runWalN(WalN *a);
	void runGlobal(Global *a);
	void runTalk(Talk *a);
	void runChangeLevel(ChangeLevel *a);

	// Screen
	Graphics::PixelFormat _pixelFormat;
	void changeScreenMode(Common::String mode);
	Graphics::ManagedSurface *_compositeSurface;
	uint32 _transparentColor;
	Common::Rect screenRect;
	void updateScreen(MVideo &video);
	void drawScreen();

	// intros
	Common::HashMap<Common::String, Videos> _intros;

	// levels
	Common::String _nextLevel;
	Common::String _currentLevel;

	// hotspots
	Hotspots *_nextHotsToAdd;
	Hotspots *_nextHotsToRemove;
	HotspotsStack stack;

	// Movies
	Videos _nextSequentialVideoToPlay;
	Videos _nextParallelVideoToPlay;
	Videos _videosPlaying;

	// Sounds
	Common::String _music;
	void playSound(Common::String name, uint32);
	void stopSound();
	bool isSoundActive();
	bool _noStopSounds;

	// Arcade
	void drawShoot(Common::Point);
	virtual void drawPlayer(Common::String player, MVideo &background);
	virtual void drawHealth(const Graphics::Font &font);
	int _health;
	int _maxHealth;
	Shoots _shoots;

	// Conversation
	Actions _conversation;
	bool _refreshConversation;
	void showConversation();
	bool rightClickedConversation(Common::Point mousePos);
	bool leftClickedConversation(Common::Point mousePos);

	// Hardcoded puzzles
	virtual void runPuzzle(Puzzle puzzle);

	// Timers
	bool installTimer(uint32, Common::String *);
	void removeTimer();
};

class WetEngine : public HypnoEngine {
public:
	WetEngine(OSystem *syst, const ADGameDescription *gd);

	void loadAssets() override;

	void drawPlayer(Common::String player, MVideo &background) override;
	void drawHealth(const Graphics::Font &font) override;
};

class SpiderEngine : public HypnoEngine {
public:
	SpiderEngine(OSystem *syst, const ADGameDescription *gd);

	void loadAssets() override;

	void drawPlayer(Common::String player, MVideo &background) override;
	void drawHealth(const Graphics::Font &font) override;
	void runPuzzle(Puzzle puzzle) override;

private:
	void runMatrix(Puzzle puzzle);
};

} // End of namespace Hypno

#endif
