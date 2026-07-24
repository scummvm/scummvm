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
 */

#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/shared.h"
#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"
#include "scumm/insane/rebel2/psx/video.h"

namespace Scumm {

Rebel2PSX::Rebel2PSX(ScummEngine_v7 *vm) : _vm(vm) {
}

class Rebel2PSX::Level1Handler : public Rebel2Level1Handler {
public:
	enum Error {
		kGameplayError,
		kEndingError,
		kDeathVideoError,
		kGameOverVideoError
	};

	Level1Handler(Rebel2PSX &psx, const RA2PSXModel &enemy, const RA2PSXModel &ship,
			const RA2PSXModel &crosshair, const RA2PSXModel &laser,
			const RA2PSXLevel1UI &ui, int &score) :
		_psx(psx), _enemy(enemy), _ship(ship), _crosshair(crosshair), _laser(laser),
		_ui(ui), _score(score), _error(kGameplayError) {
	}

	bool shouldQuit() const override {
		return _psx._vm->shouldQuit();
	}

	Result playAttempt(int &lives) override {
		switch (_psx.playLevel1(_enemy, _ship, _crosshair, _laser, _ui, lives, _score)) {
		case Rebel2PSX::kLevel1Quit:
			return kQuit;
		case Rebel2PSX::kLevel1Complete:
			return kComplete;
		case Rebel2PSX::kLevel1Death:
			return kDeath;
		case Rebel2PSX::kLevel1Error:
			return kError;
		}

		return kError;
	}

	bool playComplete() override {
		return playVideo("S1/L01_EXTR.STR", kEndingError);
	}

	bool playDeath() override {
		return playVideo("S1/L01_DIE.STR", kDeathVideoError);
	}

	bool playRetry(int) override {
		return true;
	}

	bool playGameOver(int) override {
		return playVideo("S1/L01_OVER.STR", kGameOverVideoError);
	}

	Error getError() const {
		return _error;
	}

private:
	bool playVideo(const char *path, Error error) {
		if (_psx.playVideo(path, 1, false))
			return true;
		_error = error;
		return false;
	}

	Rebel2PSX &_psx;
	const RA2PSXModel &_enemy;
	const RA2PSXModel &_ship;
	const RA2PSXModel &_crosshair;
	const RA2PSXModel &_laser;
	const RA2PSXLevel1UI &_ui;
	int &_score;
	Error _error;
};

Common::SeekableReadStream *Rebel2PSX::openResource(int number) {
	const Common::Path path(Common::String::format("RESOURCE.%03d", number));
	Common::File *file = new Common::File();
	if (file->open(path))
		return file;
	delete file;
	return nullptr;
}

Common::SeekableReadStream *Rebel2PSX::openRawFile(const Common::Path &path, int discNumber) {
	Common::File *file = new Common::File();
	Common::Path extractedPath = path;
	if (discNumber == 2) {
		const Common::Path disc2Path = Common::Path("DISC2").join(path);
		if (file->open(disc2Path))
			extractedPath = disc2Path;
	}
	if (!file->isOpen() && !file->open(extractedPath)) {
		delete file;
		return nullptr;
	}
	if (file->size() % 2352 != 0) {
		delete file;
		return nullptr;
	}
	return file;
}

bool Rebel2PSX::playVideo(const Common::Path &path, int discNumber, bool version2,
		const RA2PSXMovieText *movieText, RA2PSXMovieTextSequence textSequence) {
	Common::SeekableReadStream *stream = openRawFile(path, discNumber);
	if (!stream)
		return false;

	const RA2PSXStreamDecoder::FrameVersion frameVersion = version2 ?
			RA2PSXStreamDecoder::kVersion2 : RA2PSXStreamDecoder::kVersion3;
	RA2PSXStreamDecoder decoder(frameVersion);
	if (!decoder.loadStream(stream) || !decoder.setOutputPixelFormat(g_system->getScreenFormat())) {
		decoder.close();
		return false;
	}
	decoder.start();
	const bool cursorWasVisible = CursorMan.isVisible();
	CursorMan.showMouse(false);
	g_system->fillScreen(0);

	bool skipped = false;
	while (!_vm->shouldQuit() && !decoder.endOfVideo() && !skipped) {
		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			if (frame) {
				const int width = MIN<int>(frame->w, _vm->_screenWidth);
				const int height = MIN<int>(frame->h, _vm->_screenHeight);
				const int sourceX = (frame->w - width) / 2;
				const int sourceY = (frame->h - height) / 2;
				const int destX = (_vm->_screenWidth - width) / 2;
				const int destY = (_vm->_screenHeight - height) / 2;
				g_system->copyRectToScreen(frame->getBasePtr(sourceX, sourceY), frame->pitch,
						destX, destY, width, height);
				if (movieText && textSequence != kRA2PSXMovieTextNone) {
					Graphics::Surface *screen = g_system->lockScreen();
					// The original callback advances the STR's one-based frame ID.
					movieText->draw(*screen, textSequence, decoder.getCurFrame() + 2,
							(_vm->_screenWidth - 320) / 2, (_vm->_screenHeight - 240) / 2);
					g_system->unlockScreen();
				}
				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
					(event.customType == kScummActionInsaneSkip ||
					 event.customType == kScummActionInsaneBack)) ||
					(event.type == Common::EVENT_KEYDOWN &&
					 event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
					event.type == Common::EVENT_LBUTTONUP)
				skipped = true;
			else if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				_vm->quitGame();
		}
		g_system->delayMillis(5);
	}
	decoder.close();
	CursorMan.showMouse(cursorWasVisible);
	return !_vm->shouldQuit();
}

bool Rebel2PSX::playIntroSequence(const RA2PSXMovieText &movieText) {
	static const char *const videos[] = {
		"LEC_LOGO.STR",
		"F5_LOGO.STR",
		"OPENING.STR"
	};
	for (uint i = 0; i < ARRAYSIZE(videos); ++i) {
		const RA2PSXMovieTextSequence textSequence = i == 2 ?
				kRA2PSXMovieTextOpening : kRA2PSXMovieTextNone;
		if (!playVideo(videos[i], 1, false, &movieText, textSequence))
			return false;
	}
	return true;
}

bool Rebel2PSX::loadGlobalAssets(RA2PSXMainMenuUI &menu) {
	Common::SeekableReadStream *stream = openResource(0);
	if (!stream)
		return false;

	RA2PSXArchive archive;
	const bool loaded = archive.load(*stream);
	delete stream;
	Common::Array<byte> soundData;
	Common::Array<byte> soundProjectData;
	return loaded && menu.load(archive) &&
			archive.getMember("SNDsmp", soundData) &&
			archive.getMember("sNDdata", soundProjectData) &&
			_soundBank.load(soundData, soundProjectData);
}

bool Rebel2PSX::loadMovieTextAssets(RA2PSXMovieText &movieText) {
	Common::File executable;
	return executable.open("SLUS_003.81") && movieText.load(executable);
}

bool Rebel2PSX::loadLevel1Assets(RA2PSXModel &enemy, RA2PSXModel &ship,
		RA2PSXModel &crosshair, RA2PSXModel &laser, RA2PSXLevel1UI &ui) {
	Common::SeekableReadStream *stream = openResource(1);
	if (!stream)
		return false;
	RA2PSXArchive archive;
	const bool loaded = archive.load(*stream);
	delete stream;
	if (!loaded)
		return false;

	Common::Array<byte> enemyData;
	Common::Array<byte> shipData;
	Common::Array<byte> crosshairData;
	Common::Array<byte> laserData;
	Common::Array<byte> enemyTextureData;
	Common::Array<byte> shipTextureData;
	return archive.getMember("fOFS/TieFighter/main", enemyData) && enemy.load(enemyData) &&
			archive.getMember("tex/Ties", enemyTextureData) && enemy.loadTextures(enemyTextureData) &&
			archive.getMember("fOFS/Ship", shipData) && ship.load(shipData) &&
			archive.getMember("tex/BWingCockp", shipTextureData) && ship.loadTextures(shipTextureData) &&
			archive.getMember("fOFS/CrosshairW", crosshairData) && crosshair.load(crosshairData) &&
			archive.getMember("fOFS/WingLaser", laserData) && laser.load(laserData) &&
			ui.load(archive);
}

Common::Error Rebel2PSX::runGame() {
#ifdef USE_TINYGL
	RA2PSXMainMenuUI menu;
	RA2PSXMovieText movieText;
	if (!loadGlobalAssets(menu))
		return Common::Error(Common::kReadingFailed,
				_("Could not load the PlayStation menu resources"));
	if (!loadMovieTextAssets(movieText))
		return Common::Error(Common::kReadingFailed,
				_("Could not load the PlayStation movie fonts"));
	if (!playIntroSequence(movieText)) {
		if (_vm->shouldQuit())
			return Common::kNoError;
		return Common::Error(Common::kReadingFailed,
				_("Could not play the PlayStation introduction"));
	}

	const MenuResult menuResult = runMainMenu(menu);
	if (menuResult == kMenuQuit)
		return Common::kNoError;

	RA2PSXModel enemy;
	RA2PSXModel ship;
	RA2PSXModel crosshair;
	RA2PSXModel laser;
	RA2PSXLevel1UI ui;
	if (!loadLevel1Assets(enemy, ship, crosshair, laser, ui))
		return Common::Error(Common::kReadingFailed,
				_("Could not load the PlayStation Level 1 resources"));

	if (!playVideo("S1/L01_INTR.STR", 1, false, &movieText,
			kRA2PSXMovieTextChapter1)) {
		if (_vm->shouldQuit())
			return Common::kNoError;
		return Common::Error(Common::kReadingFailed,
				_("Could not play the PlayStation Level 1 introduction"));
	}

	int lives = 3;
	int score = 0;
	Level1Handler handler(*this, enemy, ship, crosshair, laser, ui, score);
	if (runRebel2Level1(handler, lives) != Rebel2Level1Handler::kError)
		return Common::kNoError;

	switch (handler.getError()) {
	case Level1Handler::kEndingError:
		return Common::Error(Common::kReadingFailed,
				_("Could not play the PlayStation Level 1 ending"));
	case Level1Handler::kDeathVideoError:
		return Common::Error(Common::kReadingFailed,
				_("Could not play the PlayStation Level 1 death video"));
	case Level1Handler::kGameOverVideoError:
		return Common::Error(Common::kReadingFailed,
				_("Could not play the PlayStation game-over video"));
	default:
		return Common::Error(Common::kReadingFailed,
				_("Could not run the PlayStation Level 1 gameplay"));
	}
#else
	return Common::Error(Common::kUnsupportedGameidError,
			_s("Rebel Assault II PlayStation support requires TinyGL"));
#endif
}

} // End of namespace Scumm
