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

#include "harold/bcb/bcb.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/screen.h"
#include "graphics/sjis.h"
#include "graphics/surface.h"
#include "harold/bcb/debugger.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"
#include "video/avi_decoder.h"

namespace BCB {

#ifdef USE_FREETYPE2
static const int kCaptionTextNativeCharacterHeight = 18;
static const int kOwnerDrawUiTextNativeCharacterHeight = 16;

static Graphics::Font *loadLocalizedCjkFont(Common::Language language, int size, Graphics::TTFSizeMode sizeMode) {
	if (language == Common::KO_KOR) {
		Graphics::Font *font = Graphics::loadTTFFontFromArchive("NotoSansKR-Regular.otf", size, sizeMode, 0, 0, Graphics::kTTFRenderModeNormal);
		if (font)
			return font;
		font = Graphics::loadTTFFontFromArchive("NotoSansJP-Regular.otf", size, sizeMode, 0, 0, Graphics::kTTFRenderModeNormal);
		if (font)
			return font;
		return Graphics::loadTTFFontFromArchive("VL-Gothic-Regular.ttf", size, sizeMode, 0, 0, Graphics::kTTFRenderModeNormal);
	}

	Graphics::Font *font = Graphics::loadTTFFontFromArchive("VL-Gothic-Regular.ttf", size, sizeMode, 0, 0, Graphics::kTTFRenderModeNormal);
	if (font)
		return font;
	return Graphics::loadTTFFontFromArchive("NotoSansJP-Regular.otf", size, sizeMode, 0, 0, Graphics::kTTFRenderModeNormal);
}

static Graphics::Font *loadLocalizedOwnerDrawFont(Common::Language language) {
	if (language == Common::KO_KOR) {
		Graphics::Font *font = Graphics::loadTTFFontFromArchive("NotoSansKR-Regular.otf", kOwnerDrawUiTextNativeCharacterHeight, Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeNormal);
		if (font)
			return font;
	}

	Graphics::Font *font = Graphics::loadTTFFontFromArchive("NotoSansJP-Regular.otf", kOwnerDrawUiTextNativeCharacterHeight, Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeNormal);
	if (font)
		return font;
	return Graphics::loadTTFFontFromArchive("VL-Gothic-Regular.ttf", kOwnerDrawUiTextNativeCharacterHeight, Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeNormal);
}
#endif

void BCBEngine::loadFonts() {
#ifdef USE_FREETYPE2
	_captionTextFont = loadLocalizedCjkFont(_gameDescription->desc.language, kCaptionTextNativeCharacterHeight, Graphics::kTTFSizeModeCharacter);
	_gameOverTextFont = loadLocalizedCjkFont(_gameDescription->desc.language, 20, Graphics::kTTFSizeModeCell);
	_ownerDrawUiTextFont = loadLocalizedOwnerDrawFont(_gameDescription->desc.language);
#endif
}

BCBEngine::BCBEngine(OSystem *syst, const HaroldGameDescription *desc)
	: Engine(syst), _gameDescription(desc) {
}

BCBEngine::~BCBEngine() {
	delete _screen;
	if (_uiBitmap) {
		_uiBitmap->free();
		delete _uiBitmap;
	}
	if (_chipBitmap) {
		_chipBitmap->free();
		delete _chipBitmap;
	}
	if (_computerBitmap) {
		_computerBitmap->free();
		delete _computerBitmap;
	}
	delete _activeMovieVideo;
	delete _mapMoveVideo;
	delete _sceneIdleVideo;
	delete _gameOverVideo;
	freeSceneChoiceBacking();
	if (_computerDetailBacking) {
		_computerDetailBacking->free();
		delete _computerDetailBacking;
	}
	delete _sjisFont;
	delete _captionTextFont;
	delete _gameOverTextFont;
	delete _ownerDrawUiTextFont;
	if (_titleBitmap) {
		_titleBitmap->free();
		delete _titleBitmap;
	}
	stopMenuBackgroundMusic();
}

Common::Error BCBEngine::run() {
	const Graphics::PixelFormat screenFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);

	setDebugger(new Debugger(this));

	if (!initializeSearchPaths())
		return Common::kPathDoesNotExist;

	initGraphics(640, 480, &screenFormat);
	_screen = new Graphics::Screen(640, 480, g_system->getScreenFormat());
	CursorMan.lock(false);
	CursorMan.setDefaultArrowCursor();
	CursorMan.showMouse(true);
	_screen->update();
	_lastCursorRefreshMillis = g_system->getMillis();
	_sjisFont = Graphics::FontSJIS::createFont();
	loadFonts();
	if (!_sjisFont && !_captionTextFont && !_gameOverTextFont && !_ownerDrawUiTextFont)
		warning("BCBEngine::run(): No CJK font available; captions and choice text will not render");

	if (!loadScriptData())
		return Common::kReadingFailed;

	if (!loadExecutableDataTables())
		return Common::kReadingFailed;

	resetGameState();

	if (!showInitialLogoScreen())
		return Common::kReadingFailed;

	mainLoop();
	return Common::kNoError;
}

void BCBEngine::clearDisplayAndResetState() {
	_currentDisplayState = 0;
	stateDword(kStateRequiredDiscIndex) = 0;
	stopSceneIdleMovie();
	stopMapMoveMovie();
	restoreActiveCaptionBackingRect();
	freeSceneChoiceBacking();
	_sceneChoiceActive = false;
	_mapControlsActive = false;

	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen) {
		const uint32 black = screen->format.RGBToColor(0, 0, 0);
		screen->fillRect(Common::Rect(0, 0, 640, 480), black);
		_screen->makeAllDirty();
	}
	_screen->update();
}

void BCBEngine::processGameStateDispatcher(bool runScriptBeforeDispatch) {
	_scriptStopFlag = false;

	while (!shouldQuit() && !_scriptStopFlag) {
		checkCurrentMediaPathReady();
		if (showPendingStillImageScreen())
			return;

		if (processPendingMovieRequest())
			return;

		if (runScriptBeforeDispatch)
			executeCurrentScriptSequence();

		if (stateDword(kStateDispatcherControlCode) > 6) {
			warning("BCBEngine::processGameStateDispatcher(): Invalid dispatcher control code %u", stateDword(kStateDispatcherControlCode));
			_scriptStopFlag = true;
			return;
		}

		if (dispatchGameStateControlCode(stateDword(kStateDispatcherControlCode)))
			return;
	}
}

bool BCBEngine::dispatchGameStateControlCode(uint32 dispatcherControlCode) {
	switch (dispatcherControlCode) {
	case 0:
		return dispatchGameStateNoOp();
	case 1:
		return dispatchSceneChoiceControls();
	case 2:
		return dispatchMapMoveControls();
	case 3:
		return dispatchComputerMainControls();
	case 4:
		return dispatchComputerDetailControls();
	case 5:
		return dispatchGameOverScreen();
	case 6:
		return dispatchEndingStillDisplayState();
	default:
		warning("BCBEngine::dispatchGameStateControlCode(): Unhandled dispatcher control code %u", dispatcherControlCode);
		_scriptStopFlag = true;
		return true;
	}
}

bool BCBEngine::dispatchGameStateNoOp() {
	return false;
}

bool BCBEngine::dispatchSceneChoiceControls() {
	return showActiveSceneChoiceControls();
}

void BCBEngine::mainLoop() {
	while (!shouldQuit()) {
		bool screenUpdated = false;

		if (!pollEvents())
			return;

		if (_currentDisplayState == 10) {
			const uint32 now = g_system->getMillis();
			while (now - _lastTimerTick >= 50 && _currentDisplayState == 10) {
				_lastTimerTick += 50;
				++_initialLogoElapsedTimerTicks;
				if (_initialLogoElapsedTimerTicks > 0x3c)
					transitionFromLogoToTitle();
			}
		} else if (_currentDisplayState == 1 && !_activeMovieVideo && !_sceneIdleVideo) {
			const uint32 now = g_system->getMillis();
			while (now - _lastTimerTick >= 50 && _currentDisplayState == 1 && !_activeMovieVideo && !_sceneIdleVideo) {
				_lastTimerTick += 50;
				++_sceneIdleMovieTimerTicks;
				if (_sceneIdleMovieTimerTicks > 200)
					startSceneIdleMovie();
			}
		} else if (_currentDisplayState == 6) {
			const uint32 now = g_system->getMillis();
			while (now - _lastTimerTick >= 50 && _currentDisplayState == 6) {
				_lastTimerTick += 50;
				++_pendingStillElapsedTimerTicks;
				if (kStillImageDurationTicks[_pendingStillDurationIndex] < _pendingStillElapsedTimerTicks)
					completePendingStillImageScreen();
			}
		} else if (_currentDisplayState == 9) {
			const uint32 now = g_system->getMillis();
			while (now - _lastTimerTick >= 50 && _currentDisplayState == 9) {
				_lastTimerTick += 50;
				++_gameOverElapsedTimerTicks;
				if (stateDword(kStateClearDisplayBeforeMediaCheckFlag) != 0 || _gameOverElapsedTimerTicks > 300) {
					resetGameState();
					showTitleScreen();
					startMenuBackgroundMusic();
				}
			}
		}

		updateMenuBackgroundMusic();

		if (_sceneIdleVideo) {
			if (_sceneIdleVideo->endOfVideo()) {
				_sceneIdleVideo->stop();
				_sceneIdleVideo->rewind();
				_sceneIdleVideo->decodeNextFrame();
				_sceneIdleVideo->start();
			}

			if (_sceneIdleVideo->needsUpdate()) {
				const Graphics::Surface *frame = _sceneIdleVideo->decodeNextFrame();
				if (frame) {
					_screen->copyRectToSurface(frame->getPixels(), frame->pitch, 0x10, 0x30, frame->w, frame->h);
					_screen->update();
					screenUpdated = true;
				}
			}
		}

		if (_mapMoveVideo) {
			if (_mapMoveVideo->endOfVideo()) {
				_mapMoveVideo->stop();
				_mapMoveVideo->rewind();
				_mapMoveVideo->decodeNextFrame();
				_mapMoveVideo->start();
			}

			if (_mapMoveVideo->needsUpdate())
				_mapMoveVideo->decodeNextFrame();
		}

		if (_gameOverVideo) {
			if (_gameOverVideo->needsUpdate())
				_gameOverVideo->decodeNextFrame();
			if (_gameOverVideo->endOfVideo())
				stopGameOverMusic();
		}

		if (_activeMovieVideo) {
			bool movieUpdated = false;
			if (_activeMovieVideo->needsUpdate()) {
				const Graphics::Surface *frame = _activeMovieVideo->decodeNextFrame();
				if (frame) {
					const Common::Point moviePosition = getActiveMoviePosition();
					_screen->copyRectToSurface(frame->getPixels(), frame->pitch, moviePosition.x, moviePosition.y, frame->w, frame->h);
					movieUpdated = true;
				}
			}
			if (drawTimedVideoTextOverlay())
				movieUpdated = true;
			if (movieUpdated) {
				_screen->update();
				screenUpdated = true;
			}

			if (_activeMovieVideo->endOfVideo()) {
				const bool dispatchAfterEnd = _activeMovieDispatchOnFinish;
				stopActiveMovie();
				if (dispatchAfterEnd) {
					_activeMovieDispatchOnFinish = false;
					if (_currentDisplayState == 7 || _currentDisplayState == 8) {
						processQueuedMovieSequence();
					} else if (_currentDisplayState == 1) {
						drawMainSceneScreen(false);
						processGameStateDispatcher(true);
					} else {
						if (_currentDisplayState == 2)
							drawClockAndChipIndicator();
						processGameStateDispatcher(true);
					}
				}
			}
		}

		serviceCursorRefresh(screenUpdated);

		delayWithEventPolling(10);
	}
}

} // End of namespace BCB
