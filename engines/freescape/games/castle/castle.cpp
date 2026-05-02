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

#include "common/file.h"
#include "common/memstream.h"
#include "common/config-manager.h"
#include "common/random.h"
#include "graphics/cursorman.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"
#include "common/translation.h"

#include "audio/mods/protracker.h"

#include "freescape/freescape.h"
#include "freescape/gfx.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

CastleEngine::CastleEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	if (!Common::parseBool(ConfMan.get("rock_travel"), _useRockTravel))
		error("Failed to parse bool from rock_travel option");

	_soundIndexStart = 11;
	_soundIndexAreaChange = 24;
	_soundIndexCollide = 4;
	_soundIndexStepUp = 5;
	_soundIndexStepDown = 6;
	_soundIndexStartFalling = -1;

	k8bitVariableShield = 29;

	if (isDOS())
		initDOS();
	else if (isSpectrum())
		initZX();
	else if (isCPC())
		initCPC();
	else if (isC64())
		initC64();

	// Messages are assigned after loading in initGameState()

	_playerHeightNumber = 1;
	_playerHeightMaxNumber = 1;
	_lastTenSeconds = -1;

	_playerSteps.clear();
	_playerSteps.push_back(15);
	_playerSteps.push_back(30);
	_playerSteps.push_back(120);
	_playerStepIndex = 2;

	_angleRotations.push_back(5);

	_playerWidth = 8;
	_playerDepth = 8;
	_stepUpDistance = 32;
	_maxShield = 24;

	_option = nullptr;
	_optionTexture = nullptr;
	_spiritsMeterIndicatorFrame = nullptr;
	_spiritsMeterIndicatorBackgroundFrame = nullptr;
	_spiritsMeterIndicatorSideFrame = nullptr;
	_strenghtBackgroundFrame = nullptr;
	_strenghtBarFrame = nullptr;
	_strenghtBackgroundCLUT8 = nullptr;
	_strenghtBarCLUT8 = nullptr;
	_spiritsMeterBgCLUT8 = nullptr;
	_spiritsMeterIndCLUT8 = nullptr;
	_keysBorderCLUT8 = nullptr;
	_menu = nullptr;
	_menuButtons = nullptr;
	_cursorData = nullptr;
	_crosshairData = nullptr;
	_cursorW = 0;
	_cursorH = 0;

	_riddleTopFrame = nullptr;
	_riddleBottomFrame = nullptr;
	_riddleBackgroundFrame = nullptr;
	_riddleNailFrame = nullptr;

	_endGameThroneFrame = nullptr;
	_endGameBackgroundFrame = nullptr;
	_gameOverBackgroundFrame = nullptr;

	_menuCrawlIndicator = nullptr;
	_menuWalkIndicator = nullptr;
	_menuRunIndicator = nullptr;
	_menuFxOnIndicator = nullptr;
	_menuFxOffIndicator = nullptr;

	_spiritsMeter = 32;
	_spiritsToKill = 26;
	_spiritsMeterPosition = 0;
	_spiritsMeterMax = 64;
	_thunderTicks = 0;
	_thunderFrameDuration = 0;
	_thunderFrameIndex = 0;
}

CastleEngine::~CastleEngine() {
	if (_option) {
		_option->free();
		delete _option;
	}

	for (int i = 0; i < int(_keysBorderFrames.size()); i++) {
		if (_keysBorderFrames[i]) {
			_keysBorderFrames[i]->free();
			delete _keysBorderFrames[i];
		}
	}

	for (int i = 0; i < int(_keysMenuFrames.size()); i++) {
		if (_keysMenuFrames[i]) {
			_keysMenuFrames[i]->free();
			delete _keysMenuFrames[i];
		}
	}

	if (_spiritsMeterIndicatorBackgroundFrame) {
		_spiritsMeterIndicatorBackgroundFrame->free();
		delete _spiritsMeterIndicatorBackgroundFrame;
	}

	if (_spiritsMeterIndicatorFrame) {
		_spiritsMeterIndicatorFrame->free();
		delete _spiritsMeterIndicatorFrame;
	}

	if (_spiritsMeterIndicatorSideFrame) {
		_spiritsMeterIndicatorSideFrame->free();
		delete _spiritsMeterIndicatorSideFrame;
	}

	if (_strenghtBackgroundFrame) {
		_strenghtBackgroundFrame->free();
		delete _strenghtBackgroundFrame;
	}

	if (_strenghtBarFrame) {
		_strenghtBarFrame->free();
		delete _strenghtBarFrame;
	}

	for (int i = 0; i < int(_strenghtWeightsFrames.size()); i++) {
		if (_strenghtWeightsFrames[i]) {
			_strenghtWeightsFrames[i]->free();
			delete _strenghtWeightsFrames[i];
		}
	}

	for (int i = 0; i < int(_flagFrames.size()); i++) {
		if (_flagFrames[i]) {
			_flagFrames[i]->free();
			delete _flagFrames[i];
		}
	}

	for (int i = 0; i < int(_thunderFrames.size()); i++) {
		if (_thunderFrames[i]) {
			_thunderFrames[i]->free();
			delete _thunderFrames[i];
		}
	}

	for (int i = 0; i < int(_thunderTextures.size()); i++) {
		if (_thunderTextures[i]) {
			delete _thunderTextures[i];
		}
	}

	if (_riddleTopFrame) {
		_riddleTopFrame->free();
		delete _riddleTopFrame;
	}

	if (_riddleBackgroundFrame) {
		_riddleBackgroundFrame->free();
		delete _riddleBackgroundFrame;
	}

	if (_riddleBottomFrame) {
		_riddleBottomFrame->free();
		delete _riddleBottomFrame;
	}

	if (_endGameThroneFrame) {
		_endGameThroneFrame->free();
		delete _endGameThroneFrame;
	}

	if (_endGameBackgroundFrame) {
		_endGameBackgroundFrame->free();
		delete _endGameBackgroundFrame;
	}

	if (_gameOverBackgroundFrame) {
		_gameOverBackgroundFrame->free();
		delete _gameOverBackgroundFrame;
	}

	if (_menu) {
		_menu->free();
		delete _menu;
	}

	if (_menuButtons) {
		_menuButtons->free();
		delete _menuButtons;
	}

	if (_menuCrawlIndicator) {
		_menuCrawlIndicator->free();
		delete _menuCrawlIndicator;
	}

	if (_menuWalkIndicator) {
		_menuWalkIndicator->free();
		delete _menuWalkIndicator;
	}

	if (_menuRunIndicator) {
		_menuRunIndicator->free();
		delete _menuRunIndicator;
	}

	if (_menuFxOnIndicator) {
		_menuFxOnIndicator->free();
		delete _menuFxOnIndicator;
	}

	if (_menuFxOffIndicator) {
		_menuFxOffIndicator->free();
		delete _menuFxOffIndicator;
	}
}

Graphics::ManagedSurface *CastleEngine::loadFrameWithHeader(Common::SeekableReadStream *file, int pos, uint32 front, uint32 back) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	file->seek(pos);
	int16 width = file->readByte();
	int16 height = file->readByte();
	debugC(kFreescapeDebugParser, "Frame size: %d x %d", width, height);
	surface->create(width * 8, height, _gfx->_texturePixelFormat);

	/*byte mask =*/ file->readByte();

	surface->fillRect(Common::Rect(0, 0, width * 8, height), back);
	/*int frameSize =*/ file->readUint16LE();
	return loadFrame(file, surface, width, height, front);
}

Common::Array<Graphics::ManagedSurface *> CastleEngine::loadFramesWithHeader(Common::SeekableReadStream *file, int pos, int numFrames, uint32 front, uint32 back) {
	Graphics::ManagedSurface *surface = nullptr;
	file->seek(pos);
	int16 width = file->readByte();
	int16 height = file->readByte();
	/*byte mask =*/ file->readByte();

	/*int frameSize =*/ file->readUint16LE();
	Common::Array<Graphics::ManagedSurface *> frames;
	for (int i = 0; i < numFrames; i++) {
		surface = new Graphics::ManagedSurface();
		surface->create(width * 8, height, _gfx->_texturePixelFormat);
		surface->fillRect(Common::Rect(0, 0, width * 8, height), back);
		frames.push_back(loadFrame(file, surface, width, height, front));
	}

	return frames;
}

// loadFrame moved to FreescapeEngine (freescape.cpp)

Graphics::ManagedSurface *CastleEngine::loadFrameWithHeaderCPC(Common::SeekableReadStream *file, int pos, const uint32 *cpcPalette) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	file->seek(pos);
	int16 width = file->readByte();
	int16 height = file->readByte();
	debugC(kFreescapeDebugParser, "CPC Frame size: %d x %d", width, height);
	surface->create(width * 4, height, _gfx->_texturePixelFormat);

	/*byte mask =*/ file->readByte();

	surface->fillRect(Common::Rect(0, 0, width * 4, height), cpcPalette[0]);
	/*int frameSize =*/ file->readUint16LE();
	return loadFrameCPC(file, surface, width, height, cpcPalette);
}

Common::Array<Graphics::ManagedSurface *> CastleEngine::loadFramesWithHeaderCPC(Common::SeekableReadStream *file, int pos, int numFrames, const uint32 *cpcPalette) {
	Graphics::ManagedSurface *surface = nullptr;
	file->seek(pos);
	int16 width = file->readByte();
	int16 height = file->readByte();
	/*byte mask =*/ file->readByte();

	/*int frameSize =*/ file->readUint16LE();
	Common::Array<Graphics::ManagedSurface *> frames;
	for (int i = 0; i < numFrames; i++) {
		surface = new Graphics::ManagedSurface();
		surface->create(width * 4, height, _gfx->_texturePixelFormat);
		surface->fillRect(Common::Rect(0, 0, width * 4, height), cpcPalette[0]);
		frames.push_back(loadFrameCPC(file, surface, width, height, cpcPalette));
	}

	return frames;
}

void CastleEngine::convertCPCSprite(Graphics::ManagedSurface *clut8, Graphics::ManagedSurface *&argb, bool transparentInk0) {
	if (argb) {
		argb->free();
		delete argb;
	}
	if (transparentInk0) {
		// Ink 0 = value 0 (transparent for copyRectToSurfaceWithKey with back=0)
		argb = new Graphics::ManagedSurface();
		argb->create(clut8->w, clut8->h, _gfx->_texturePixelFormat);
		argb->fillRect(Common::Rect(0, 0, clut8->w, clut8->h), 0);

		byte palette[4 * 3];
		clut8->grabPalette(palette, 0, 4);

		for (int y = 0; y < clut8->h; y++) {
			for (int x = 0; x < clut8->w; x++) {
				byte idx = clut8->getPixel(x, y);
				if (idx != 0) {
					uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0xFF,
						palette[idx * 3], palette[idx * 3 + 1], palette[idx * 3 + 2]);
					argb->setPixel(x, y, color);
				}
			}
		}
	} else {
		// Opaque: ink 0 = solid black, fully covers what's beneath
		Graphics::Surface *converted = _gfx->convertImageFormatIfNecessary(clut8);
		argb = new Graphics::ManagedSurface();
		argb->copyFrom(*converted);
		converted->free();
		delete converted;
	}
}

Graphics::ManagedSurface *CastleEngine::loadFrameWithHeaderCPCIndexed(Common::SeekableReadStream *file, int pos) {
	file->seek(pos);
	int w = file->readByte();
	int h = file->readByte();
	file->readByte(); // mask
	file->readUint16LE(); // frameSize
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(w * 4, h, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, w * 4, h), 0);
	return loadFrameCPCIndexed(file, surface, w, h);
}

Common::Array<Graphics::ManagedSurface *> CastleEngine::loadFramesWithHeaderCPCIndexed(Common::SeekableReadStream *file, int pos, int numFrames) {
	file->seek(pos);
	int w = file->readByte();
	int h = file->readByte();
	file->readByte(); // mask
	file->readUint16LE(); // frameSize
	Common::Array<Graphics::ManagedSurface *> frames;
	for (int f = 0; f < numFrames; f++) {
		Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
		surface->create(w * 4, h, Graphics::PixelFormat::createFormatCLUT8());
		surface->fillRect(Common::Rect(0, 0, w * 4, h), 0);
		loadFrameCPCIndexed(file, surface, w, h);
		frames.push_back(surface);
	}
	return frames;
}

void CastleEngine::updateCPCSpritesPalette() {
	byte palette[4 * 3];
	for (int c = 0; c < 4; c++) {
		uint8 r, g, b;
		_gfx->selectColorFromFourColorPalette(c, r, g, b);
		palette[c * 3 + 0] = r;
		palette[c * 3 + 1] = g;
		palette[c * 3 + 2] = b;
	}

	if (_keysBorderCLUT8) {
		_keysBorderCLUT8->setPalette(palette, 0, 4);
		convertCPCSprite(_keysBorderCLUT8, _keysBorderFrames[0], true);
	}
	if (_spiritsMeterBgCLUT8) {
		_spiritsMeterBgCLUT8->setPalette(palette, 0, 4);
		convertCPCSprite(_spiritsMeterBgCLUT8, _spiritsMeterIndicatorBackgroundFrame);
	}
	if (_spiritsMeterIndCLUT8) {
		_spiritsMeterIndCLUT8->setPalette(palette, 0, 4);
		convertCPCSprite(_spiritsMeterIndCLUT8, _spiritsMeterIndicatorFrame, true);
	}
	if (_strenghtBackgroundCLUT8) {
		_strenghtBackgroundCLUT8->setPalette(palette, 0, 4);
		convertCPCSprite(_strenghtBackgroundCLUT8, _strenghtBackgroundFrame);
	}
	if (_strenghtBarCLUT8) {
		_strenghtBarCLUT8->setPalette(palette, 0, 4);
		convertCPCSprite(_strenghtBarCLUT8, _strenghtBarFrame);
	}
	for (int f = 0; f < (int)_strenghtWeightsCLUT8.size(); f++) {
		_strenghtWeightsCLUT8[f]->setPalette(palette, 0, 4);
		convertCPCSprite(_strenghtWeightsCLUT8[f], _strenghtWeightsFrames[f], true);
	}
	for (int f = 0; f < (int)_flagCLUT8.size(); f++) {
		_flagCLUT8[f]->setPalette(palette, 0, 4);
		convertCPCSprite(_flagCLUT8[f], _flagFrames[f]);
	}
}

Graphics::ManagedSurface *CastleEngine::loadFrameCPC(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height, const uint32 *cpcPalette) {
	for (int y = 0; y < height; y++) {
		for (int col = 0; col < width; col++) {
			byte cpc_byte = file->readByte();
			for (int i = 0; i < 4; i++) {
				int pixel = getCPCPixel(cpc_byte, i, true);
				if (pixel != 0)
					surface->setPixel(col * 4 + i, y, cpcPalette[pixel]);
			}
		}
	}
	return surface;
}

void CastleEngine::initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) {
	FreescapeEngine::initKeymaps(engineKeyMap, infoScreenKeyMap, target);
	Common::Action *act;

	act = new Common::Action("SELECTPRINCE", _("Select prince"));
	act->setCustomEngineActionEvent(kActionSelectPrince);
	act->addDefaultInputMapping("1");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("SELECTPRINCESS", _("Select princess"));
	act->setCustomEngineActionEvent(kActionSelectPrincess);
	act->addDefaultInputMapping("2");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("SAVE", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("s");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	if (_language == Common::ES_ESP)
		act->addDefaultInputMapping("c");
	else
		act->addDefaultInputMapping("l");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("QUIT", _("Quit game"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("q");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("TOGGLESOUND", _("Toggle sound"));
	act->setCustomEngineActionEvent(kActionToggleSound);
	act->addDefaultInputMapping("t");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("ROTL", _("Rotate left"));
	act->setCustomEngineActionEvent(kActionRotateLeft);
	act->addDefaultInputMapping("z");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTR", _("Rotate right"));
	act->setCustomEngineActionEvent(kActionRotateRight);
	act->addDefaultInputMapping("x");
	engineKeyMap->addAction(act);

	act = new Common::Action("RUNMODE", _("Run"));
	act->setCustomEngineActionEvent(kActionRunMode);
	act->addDefaultInputMapping("r");
	engineKeyMap->addAction(act);

	act = new Common::Action("WALK", _("Walk"));
	act->setCustomEngineActionEvent(kActionWalkMode);
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping(_useWASDControls ? "2" : "w");
	engineKeyMap->addAction(act);

	act = new Common::Action("CRAWL", _("Crawl"));
	act->setCustomEngineActionEvent(kActionCrawlMode);
	act->addDefaultInputMapping("JOY_Y");
	act->addDefaultInputMapping("c");
	engineKeyMap->addAction(act);

	act = new Common::Action("FACEFRWARD", _("Face forward"));
	act->setCustomEngineActionEvent(kActionFaceForward);
	act->addDefaultInputMapping("f");
	engineKeyMap->addAction(act);

	act = new Common::Action("ACTIVATE", _("Activate"));
	act->setCustomEngineActionEvent(kActionActivate);
	act->addDefaultInputMapping(_useWASDControls ? "e" : "a");
	engineKeyMap->addAction(act);

	if (_useWASDControls) {
		act = new Common::Action("RUNMOD", _("Run (hold)"));
		act->setCustomEngineActionEvent(kActionRunModifier);
		act->addDefaultInputMapping("LSHIFT");
		act->addDefaultInputMapping("RSHIFT");
		act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
		engineKeyMap->addAction(act);
	}
}

void CastleEngine::beforeStarting() {
	if (isDOS())
		waitInLoop(250);
	else if (isSpectrum() || isCPC())
		waitInLoop(100);
	else if (isAmiga() || isAtariST())
		waitInLoop(250);
}

void CastleEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);

	if (!_areaMap.contains(areaID) && isDemo())
		return; // Abort area change if the destination does not exist (demo only)

	if (!_exploredAreas.contains(areaID)) {
		_gameStateVars[k8bitVariableScore] += 17500;
		_exploredAreas[areaID] = true;
	}

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();
	_maxFallingDistance = MAX(32, _currentArea->getScale() * 16 - 2);

	_nearClipPlane = _currentArea->isOutside() ? 2 : 0.5;

	if (entranceID > 0)
		traverseEntrance(entranceID);

	_position = _currentArea->separateFromWall(_position);
	_lastPosition = _position;

	if (_currentArea->_skyColor > 0 && _currentArea->_skyColor != 255) {
		_gfx->_keyColor = 0;
	} else
		_gfx->_keyColor = 255;

	_lastPosition = _position;

	if (areaID == _startArea && entranceID == _startEntrance) {
		if (getGameBit(31))
			playSound(13, true, _soundFxHandle);
		else
			playSound(_soundIndexStart, false, _soundFxHandle);

		// Start ProTracker background music for Amiga demo
		if (isAmiga() && !_modData.empty() && !_mixer->isSoundHandleActive(_musicHandle)) {
			Common::MemoryReadStream modStream(_modData.data(), _modData.size());
			Audio::AudioStream *musicStream = Audio::makeProtrackerStream(&modStream);
			if (musicStream)
				_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, musicStream);
		}
	} else if (areaID == _endArea && entranceID == _endEntrance) {
		_pitch = -85;
	} else {
		// If escaped, play a different sound
		if (hasEscaped())
			playSound(13, true, _soundFxHandle);
		else
			playSound(_soundIndexAreaChange, true, _soundFxHandle);
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	clearTemporalMessages();
	// Ignore sky/ground fields
	_gfx->_keyColor = 0;
	_gfx->clearColorPairArray();
	if (isCPC())
		_gfx->fillColorPairArray();

	swapPalette(areaID);

	// Enable/disable COLOR15 cycling based on per-area flag (Amiga/Atari)
	if ((isAmiga() || isAtariST()) && _currentArea)
		_gfx->_colorCyclingTimer = _currentArea->_colorCycling ? 0 : -1;

	if (isCPC())
		updateCPCSpritesPalette();

	if (isDOS()) {
		_gfx->_colorPair[_currentArea->_underFireBackgroundColor] = _currentArea->_extraColor[1];
		_gfx->_colorPair[_currentArea->_usualBackgroundColor] = _currentArea->_extraColor[0];
		_gfx->_colorPair[_currentArea->_paperColor] = _currentArea->_extraColor[2];
		_gfx->_colorPair[_currentArea->_inkColor] = _currentArea->_extraColor[3];
	} else if (isAmiga()) {
		// Unclear why these colors are always overwritten
		byte (*palette)[16][3] = (byte (*)[16][3])_gfx->_palette;

		(*palette)[1][0] = 0x44;
		(*palette)[1][1] = 0x44;
		(*palette)[1][2] = 0x44;

		(*palette)[2][0] = 0x66;
		(*palette)[2][1] = 0x66;
		(*palette)[2][2] = 0x66;

		(*palette)[3][0] = 0x88;
		(*palette)[3][1] = 0x88;
		(*palette)[3][2] = 0x88;

		(*palette)[5][0] = 0xcc;
		(*palette)[5][1] = 0xcc;
		(*palette)[5][2] = 0xcc;
	}

	if (isSpectrum())
		_gfx->_paperColor = 0;

	if (isCPC()) {
		_currentArea->_skyColor = _currentArea->_usualBackgroundColor;
	}

	resetInput();

	if (entranceID > 0) {
		executeMovementConditions();
	}
}

void CastleEngine::initGameState() {
	FreescapeEngine::initGameState();
	_playerHeightNumber = 1;

	// Platform-specific message strings (indices differ between DOS and Amiga)
	if (isAmiga() || isAtariST()) {
		_notEnoughRoomMessage = _messagesList[21];
		_tooWeakMessage = _messagesList[22];
		_crawlSelectedMessage = _messagesList[23];
		_walkSelectedMessage = _messagesList[24];
		_runSelectedMessage = _messagesList[25];
		_ghostInAreaMessage = _messagesList[126];
	} else if (isDOS()) {
		_notEnoughRoomMessage = _messagesList[11];
		_tooWeakMessage = _messagesList[12];
		_crawlSelectedMessage = _messagesList[13];
		_walkSelectedMessage = _messagesList[14];
		_runSelectedMessage = _messagesList[15];
		_ghostInAreaMessage = _messagesList[116];
	} else {
		// ZX/CPC: same indices for movement messages, no ghost warning message
		_notEnoughRoomMessage = _messagesList[11];
		_tooWeakMessage = _messagesList[12];
		_crawlSelectedMessage = _messagesList[13];
		_walkSelectedMessage = _messagesList[14];
		_runSelectedMessage = _messagesList[15];
		_ghostInAreaMessage = "";
	}

	// Fix typos in the original Spanish releases
	if (_language == Common::ES_ESP) {
		for (uint i = 0; i < _messagesList.size(); i++) {
			Common::replace(_messagesList[i], "ELIGIDO", "ELEGIDO");
			Common::replace(_messagesList[i], "BRILLIANTE", "BRILLANTE");
		}
	}

	_gameStateVars[k8bitVariableShield] = 16;
	_gameStateVars[k8bitVariableEnergy] = 1;
	_gameStateVars[8] = 128; // -1
	_countdown = INT_MAX - 8;
	_keysCollected.clear();
	_spiritsMeter = 32;
	_spiritsMeterPosition = _spiritsMeter * _spiritsToKill / _spiritsToKill;

	_exploredAreas[_startArea] = true;
	if (_useRockTravel) // Enable cheat
		setGameBit(k8bitGameBitTravelRock);

	_gfx->_shakeOffset = Common::Point();

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	_lastMinute = minutes;
	_lastTenSeconds = seconds / 10;

	_droppingGateStartTicks = 0;
	_thunderFrameDuration = 0;
}

bool CastleEngine::checkIfGameEnded() {
	if (_gameStateControl == kFreescapeGameStatePlaying) {
		if (_hasFallen && _avoidRenderingFrames == 0) {
			_hasFallen = false;
			playSound(_soundIndexFallen, false, _soundFxHandle);

			stopMovement();
			// If shield is less than 11 after a fall, the game ends
			if (_gameStateVars[k8bitVariableShield] > 5) {
				_gameStateVars[k8bitVariableShield] -= 5;
				return false; // Game can continue
			}
			if (!_fallenMessage.empty())
				insertTemporaryMessage(_fallenMessage, _countdown - 4);
			_gameStateControl = kFreescapeGameStateEnd;
		}
		if (hasEscaped()) {
			_gameStateControl = kFreescapeGameStateEnd;
			return true;
		}
	}
	return FreescapeEngine::checkIfGameEnded();
}

bool CastleEngine::triggerWinCondition() {
	if (isDOS() || isAmiga() || isAtariST()) {
		if (!_areaMap.contains(74))
			return false;
		gotoArea(74, 0);
	} else if (isCastleMaster2()) {
		// CM2: escape is script-driven (game boolean 3 bit 6).
		// The escape animation plays from the current location.
		return true;
	} else {
		_gameStateVars[31] = 10;
		gotoArea(16, 136);
	}
	return true;
}

void CastleEngine::endGame() {
	_shootingFrames = 0;
	_delayedShootObject = nullptr;
	_endGamePlayerEndArea = true;

	if (hasEscaped()) {
		insertTemporaryMessage(_messagesList[5], INT_MIN);

		if (isDOS() && !isCastleMaster2()) {
			drawFullscreenEndGameAndWait();
		} else if (isCastleMaster2()) {
			executeEscapeCameraSequence();
			drawFullscreenGameOverAndWait();
		} else
			drawFullscreenGameOverAndWait();
	} else {
		drawFullscreenGameOverAndWait();
	}

	_gameStateControl = kFreescapeGameStateRestart;
	_endGameKeyPressed = false;
}

void CastleEngine::executeEscapeCameraSequence() {
	// Escape camera animation from CM2 disassembly (L9fa1_escape_castle_sequence):
	// 1. Rotate player to face yaw=180 (away from castle), pitch=0 (level)
	// 2. Walk backward (increase Z) until leaving the map

	// Step 1: Rotate toward yaw=180, pitch=0
	float yawStep = (_yaw < 180.0f) ? 10.0f : -10.0f;
	float pitchStep = (_pitch < 180.0f) ? -10.0f : 10.0f;

	while (ABS(_yaw - 180.0f) > 5.0f || (ABS(_pitch) > 5.0f && ABS(_pitch - 360.0f) > 5.0f)) {
		if (ABS(_yaw - 180.0f) > 5.0f)
			_yaw += yawStep;
		if (ABS(_pitch) > 5.0f && ABS(_pitch - 360.0f) > 5.0f)
			_pitch += pitchStep;

		if (_yaw < 0) _yaw += 360.0f;
		if (_yaw >= 360.0f) _yaw -= 360.0f;
		if (_pitch < 0) _pitch += 360.0f;
		if (_pitch >= 360.0f) _pitch -= 360.0f;

		_cameraFront = directionToVector(_yaw, _pitch, false);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(40);
	}

	_yaw = 180.0f;
	_pitch = 0.0f;
	_cameraFront = directionToVector(_yaw, _pitch, false);

	// Step 2: Move backward (increase Z) until out of bounds
	for (int i = 0; i < 40; i++) {
		_position.setValue(2, _position.z() + 256.0f);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(40);
	}
}

bool CastleEngine::hasEscaped() {
	if (isDOS())
		return _currentArea && _currentArea->getAreaID() == 74;

	return getGameBit(31);
}

void CastleEngine::pressedKey(const int keycode) {
	// This code is duplicated in the DrillerEngine::pressedKey (except for the J case)
	if (keycode == Common::KEYCODE_s) {
		// TODO: show score
	} else if (keycode == kActionRunMode) {
		if (_playerHeightNumber == 0) {
			if (_gameStateVars[k8bitVariableShield] <= 3) {
				insertTemporaryMessage(_tooWeakMessage, _countdown - 2);
				return;
			}

			if (!rise()) {
				_playerStepIndex = 0;
				insertTemporaryMessage(_notEnoughRoomMessage, _countdown - 2);
				return;
			}
			_gameStateVars[k8bitVariableCrawling] = 0;
		}
		// TODO: raising can fail if there is no room, so the action should fail
		_playerStepIndex = 2;
		insertTemporaryMessage(_runSelectedMessage, _countdown - 2);
	} else if (keycode == kActionWalkMode) {
		if (_playerHeightNumber == 0) {
			if (_gameStateVars[k8bitVariableShield] <= 3) {
				insertTemporaryMessage(_tooWeakMessage, _countdown - 2);
				return;
			}

			if (!rise()) {
				_playerStepIndex = 0;
				insertTemporaryMessage(_notEnoughRoomMessage, _countdown - 2);
				return;
			}
			_gameStateVars[k8bitVariableCrawling] = 0;
		}

		// TODO: raising can fail if there is no room, so the action should fail
		_playerStepIndex = 1;
		insertTemporaryMessage(_walkSelectedMessage, _countdown - 2);
	} else if (keycode == kActionCrawlMode) {
		if (_playerHeightNumber == 1) {
			lower();
			_gameStateVars[k8bitVariableCrawling] = 128;
		}
		_playerStepIndex = 0;
		insertTemporaryMessage(_crawlSelectedMessage, _countdown - 2);
	} else if (keycode == kActionRunModifier) {
		// Shift-to-run: save current mode, switch to run while held
		if (_playerStepIndex == 2)
			return; // already running
		if (_playerHeightNumber == 0) {
			if (_gameStateVars[k8bitVariableShield] <= 3)
				return;
			if (!rise()) {
				return;
			}
			_gameStateVars[k8bitVariableCrawling] = 0;
		}
		_savedPlayerStepIndex = _playerStepIndex;
		_playerStepIndex = 2;
	} else if (keycode == kActionFaceForward) {
		_pitch = 0;
		updateCamera();
	} else if (keycode == kActionActivate)
		activate();
}

void CastleEngine::releasedKey(const int keycode) {
	if (keycode == kActionRunModifier) {
		// Shift released: restore the mode from before running
		if (_savedPlayerStepIndex >= 0) {
			_playerStepIndex = _savedPlayerStepIndex;
			_savedPlayerStepIndex = -1;
		}
	}
}

void CastleEngine::setAmigaCursor(bool crosshair) {
	if (!_cursorData || !_crosshairData)
		return;

	static const byte cursorPalette[16 * 3] = {
		0x00, 0x00, 0x00,  0x44, 0x44, 0x44,  0x66, 0x66, 0x66,  0x88, 0x88, 0x88,
		0xAA, 0xAA, 0xAA,  0xCC, 0xCC, 0xCC,  0xAA, 0xAA, 0xAA,  0xCC, 0xCC, 0xCC,
		0x44, 0x44, 0x44,  0x66, 0x66, 0x66,  0x88, 0x88, 0x88,  0xCC, 0xCC, 0xCC,
		0x66, 0x66, 0x66,  0xCC, 0xCC, 0xCC,  0xAA, 0xAA, 0xAA,  0xEE, 0xEE, 0xEE,
	};

	byte *srcData = crosshair ? _crosshairData : _cursorData;
	int scale = MAX(1, g_system->getWidth() / _screenW);
	int sw = _cursorW * scale;
	int sh = _cursorH * scale;
	int hotX = crosshair ? 7 * scale : 1 * scale;
	int hotY = crosshair ? 7 * scale : 1 * scale;

	byte *scaledCursor = new byte[sw * sh];
	for (int y = 0; y < sh; y++)
		for (int x = 0; x < sw; x++)
			scaledCursor[y * sw + x] = srcData[(y / scale) * _cursorW + (x / scale)];

	Graphics::PixelFormat cursorFormat = Graphics::PixelFormat::createFormatCLUT8();
	CursorMan.replaceCursor(scaledCursor, sw, sh, hotX, hotY, 0, false, &cursorFormat);
	CursorMan.replaceCursorPalette(cursorPalette, 0, 16);
	delete[] scaledCursor;
}

void CastleEngine::drawInfoMenu() {
	PauseToken pauseToken = pauseEngine();
	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}
	_savedScreen = _gfx->getScreenshot();

	uint8 r, g, b;
	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, color);

	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 front = 0;
	surface->fillRect(_viewArea, black);

	int score = _gameStateVars[k8bitVariableScore];
	int shield = _gameStateVars[k8bitVariableShield];
	int spiritsDestroyed = _gameStateVars[k8bitVariableSpiritsDestroyed];
	Common::Array<Common::Rect> keyRects;

	if (isDOS()) {
		CursorMan.setDefaultArrowCursor();
		CursorMan.showMouse(true);
		surface->copyRectToSurface(*_menu, 47, 35, Common::Rect(0, 0, _menu->w, _menu->h));

		_gfx->readFromPalette(10, r, g, b);
		front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
		drawStringInSurface(Common::String::format("%07d", score), 166, 71, front, black, surface);
		drawStringInSurface(centerAndPadString(Common::String::format("%s", _messagesList[135 + shield / 6].c_str()), 10), 151, 102,  front, black, surface);

		Common::String keysCollected = _messagesList[141];
		Common::replace(keysCollected, "X", Common::String::format("%d", _keysCollected.size()));
		drawStringInSurface(keysCollected, 103, 41,  front, black, surface);

		Common::String spiritsDestroyedString = _messagesList[133];
		Common::replace(spiritsDestroyedString, "X", Common::String::format("%d", spiritsDestroyed));
		drawStringInSurface(spiritsDestroyedString, 145 , 132,  front, black, surface);

		for (int  i = 0; i < int(_keysCollected.size()) ; i++) {
			int y = 58 + (i / 2) * 18;

			if (i % 2 == 0) {
				surface->copyRectToSurfaceWithKey(*_keysBorderFrames[i], 58, y, Common::Rect(0, 0, _keysBorderFrames[i]->w, _keysBorderFrames[i]->h), black);
				keyRects.push_back(Common::Rect(58, y, 58 + _keysBorderFrames[i]->w / 2, y + _keysBorderFrames[i]->h));
			} else {
				surface->copyRectToSurfaceWithKey(*_keysBorderFrames[i], 80, y, Common::Rect(0, 0, _keysBorderFrames[i]->w, _keysBorderFrames[i]->h), black);
				keyRects.push_back(Common::Rect(80, y, 80 + _keysBorderFrames[i]->w / 2, y + _keysBorderFrames[i]->h));
			}
		}
	} else if (isAmiga() || isAtariST()) {
		if (_cursorData)
			setAmigaCursor(false); // arrow for the menu
		else
			CursorMan.setDefaultArrowCursor();
		CursorMan.showMouse(true);
		if (_menu)
			surface->copyRectToSurface(*_menu, 47, 35, Common::Rect(0, 0, MIN<int>(_menu->w, surface->w - 47), MIN<int>(_menu->h, surface->h - 35)));

		_gfx->readFromPalette(15, r, g, b);
		front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

		// Positions and formulas from assembly at FUN_1AE0:
		// Score at (167, 71): move.w #$a7,d0; move.w #$47,d1
		drawStringInSurface(Common::String::format("%07d", score), 167, 71, front, black, surface);

		// Shield at (154, 102): move.w #$9a,d0; move.w #$66,d1
		// Index = (shield - 1) / 4 (from: subq #1,d0; lsr #2,d0; muls #$c,d0)
		// Amiga shield text at message indices 171-177 (skipping 174 which is empty)
		{
			static const int kAmigaShieldMsgIdx[] = {171, 172, 173, 175, 176, 177};
			int shieldIdx = (shield > 0) ? (shield - 1) / 4 : 0;
			if (shieldIdx > 5) shieldIdx = 5;
			drawStringInSurface(centerAndPadString(_messagesList[kAmigaShieldMsgIdx[shieldIdx]], 10), 154, 102, front, black, surface);
		}

		// Keys collected at (104, 41): move.w #$68,d0; move.w #$29,d1 (from FUN_22CC)
		// Messages: 162="NO KEYS COLLECTED", 163="XX KEYS COLLECTED", 164=" 1 KEY COLLECTED"
		{
			Common::String keysText;
			int numKeys = _keysCollected.size();
			if (numKeys == 0)
				keysText = _messagesList[162];
			else if (numKeys == 1)
				keysText = _messagesList[164];
			else {
				keysText = _messagesList[163];
				Common::replace(keysText, "XX", Common::String::format("%2d", numKeys));
			}
			drawStringInSurface(keysText, 104, 41, front, black, surface);
		}

		// Spirits destroyed at (145, 133): move.w #$91,d0; move.w #$85,d1
		// Messages: 156="NONE DESTROYED", 157=" XX DESTROYED "
		{
			Common::String spiritsText;
			if (spiritsDestroyed == 0)
				spiritsText = _messagesList[156];
			else {
				spiritsText = _messagesList[157];
				Common::replace(spiritsText, "XX", Common::String::format("%2d", spiritsDestroyed));
			}
			drawStringInSurface(spiritsText, 145, 133, front, black, surface);
		}

		// Movement indicator at (96, 118) from assembly at 0x1BE8-0x1BF0
		{
			Graphics::ManagedSurface *moveIndicator = nullptr;
			if (_playerStepIndex == 0)
				moveIndicator = _menuCrawlIndicator;
			else if (_playerStepIndex == 1)
				moveIndicator = _menuWalkIndicator;
			else
				moveIndicator = _menuRunIndicator;
			if (moveIndicator)
				surface->copyRectToSurfaceWithKey((const Graphics::Surface)*moveIndicator, 96, 118,
					Common::Rect(0, 0, moveIndicator->w, moveIndicator->h), black);
		}

		// Sound indicator at (96, 103) from assembly at 0x1C1A-0x1C22
		// Frame 3 = sound off, frame 4 = sound on (offset $360, optionally +$120)
		{
			Graphics::ManagedSurface *sndIndicator = _menuFxOnIndicator ? _menuFxOnIndicator : _menuFxOffIndicator;
			if (sndIndicator)
				surface->copyRectToSurfaceWithKey((const Graphics::Surface)*sndIndicator, 96, 103,
					Common::Rect(0, 0, sndIndicator->w, sndIndicator->h), black);
		}
	} else if (isSpectrum() || isCPC()) {
		Common::Array<Common::String> lines;
		lines.push_back(centerAndPadString("********************", 21));

		if (isCPC() && _messagesList.size() > 74) {
			Common::String commandLine = _messagesList[68];
			Common::String keysLabel = _messagesList[69];
			Common::String spiritsLabel = _messagesList[70];
			Common::String strengthLabel = _messagesList[71];
			Common::String keysText = _messagesList[72];
			Common::String spiritsText = _messagesList[73];
			Common::String scoreText = _messagesList[74];
			Common::String strengthText = _messagesList[62 + shield / 6];

			commandLine.trim();
			keysLabel.trim();
			spiritsLabel.trim();
			strengthLabel.trim();
			keysText.trim();
			spiritsText.trim();
			scoreText.trim();
			strengthText.trim();

			Common::replace(keysText, "XX", Common::String::format("%2d", _keysCollected.size()));
			Common::replace(spiritsText, "XX", Common::String::format("%2d", spiritsDestroyed));
			Common::replace(scoreText, "XXXXXXX", Common::String::format("%07d", score));

			lines.push_back(centerAndPadString(commandLine, 21));
			lines.push_back("");
			lines.push_back(centerAndPadString(Common::String::format("%s %s", keysLabel.c_str(), keysText.c_str()), 21));
			lines.push_back(centerAndPadString(Common::String::format("%s %s", spiritsLabel.c_str(), spiritsText.c_str()), 21));
			lines.push_back(centerAndPadString(Common::String::format("%s %s", strengthLabel.c_str(), strengthText.c_str()), 21));
			lines.push_back(centerAndPadString(scoreText, 21));
		} else if (_language == Common::EN_ANY) {
			lines.push_back(centerAndPadString("s-save l-load q-quit", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString(Common::String::format("keys   %d collected", _keysCollected.size()), 21));
			lines.push_back(centerAndPadString(Common::String::format("spirits  %d destroyed", spiritsDestroyed), 21));
			lines.push_back(centerAndPadString(Common::String::format("strength  %s", _messagesList[62 + shield / 6].c_str()), 21));
			lines.push_back(centerAndPadString(Common::String::format("score   %07d", score), 21));
		} else if (_language == Common::ES_ESP) {
			lines.push_back(centerAndPadString("s-salv c-carg q-quit", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString(Common::String::format("llaves %d recogidas", _keysCollected.size()), 21));
			lines.push_back(centerAndPadString(Common::String::format("espirit %d destruidos", spiritsDestroyed), 21));
			lines.push_back(centerAndPadString(Common::String::format("fuerza  %s", _messagesList[62 + shield / 6].c_str()), 21));
			lines.push_back(centerAndPadString(Common::String::format("puntos   %07d", score), 21));
		} else {
			lines.push_back(centerAndPadString("s-save l-load q-quit", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString(Common::String::format("keys   %d collected", _keysCollected.size()), 21));
			lines.push_back(centerAndPadString(Common::String::format("spirits  %d destroyed", spiritsDestroyed), 21));
			lines.push_back(centerAndPadString(Common::String::format("strength  %s", _messagesList[62 + shield / 6].c_str()), 21));
			lines.push_back(centerAndPadString(Common::String::format("score   %07d", score), 21));
		}

		lines.push_back("");
		lines.push_back(centerAndPadString("********************", 21));
		surface = drawStringsInSurface(lines, surface);
	}

	Common::Event event;
	Common::Point mousePos;
	bool cont = true;

	Common::Rect loadGameRect(101, 67, 133, 79);
	Common::Rect saveGameRect(101, 82, 133, 95);
	Common::Rect toggleSoundRect(101, 101, 133, 114);
	Common::Rect cycleRect(101, 116, 133, 129);
	Common::Rect backRect(101, 131, 133, 144);

	Graphics::Surface *originalSurface = new Graphics::Surface();
	originalSurface->copyFrom(*surface);

	Texture *menuTexture = _gfx->createTexture(surface);
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionLoad) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					_eventManager->purgeMouseEvents();

					loadGameDialog();
					_eventManager->purgeMouseEvents();
					if (isDOS() || isAmiga() || isAtariST()) {
						g_system->lockMouse(false);
						CursorMan.showMouse(true);
					}

					_gfx->setViewport(_viewArea);
				} else if (event.customType == kActionSave) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					_eventManager->purgeMouseEvents();

					saveGameDialog();
					_eventManager->purgeMouseEvents();
					if (isDOS() || isAmiga() || isAtariST()) {
						g_system->lockMouse(false);
						CursorMan.showMouse(true);
					}

					_gfx->setViewport(_viewArea);
				} else if (isDOS() && event.customType == kActionToggleSound) {
					// TODO
				} else if (event.customType == kActionQuit) {
					_forceEndGame = true;
					cont = false;
				} else
					cont = false;
				break;
			case Common::EVENT_KEYDOWN:
					cont = false;
				break;
			case Common::EVENT_MOUSEMOVE:
				if ((isAmiga() || isAtariST()) && _cursorData && _crosshairData) {
					Common::Point mp = getNormalizedPosition(event.mouse);
					setAmigaCursor(_viewArea.contains(mp));
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				// TODO: properly refresh screen
				break;
			case Common::EVENT_RBUTTONDOWN:
			// fallthrough
			case Common::EVENT_LBUTTONDOWN:
				if (isSpectrum() || isCPC())
					break;

				mousePos = getNormalizedPosition(event.mouse);
				for (int i = 0; i < int(keyRects.size()); i++) {
					if (keyRects[i].contains(mousePos)) {
						surface->copyFrom(*originalSurface);
						surface->frameRect(keyRects[i], front);
						drawStringInSurface(_messagesList[ 145 + _keysCollected[i] ], 103, 41,  front, black, surface);
						menuTexture->update(surface);
						break;
					}
				}

				if (loadGameRect.contains(mousePos)) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					loadGameDialog();
					g_system->lockMouse(false);
					CursorMan.showMouse(true);

					_gfx->setViewport(_viewArea);
				} else if (saveGameRect.contains(mousePos)) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					saveGameDialog();
					g_system->lockMouse(false);
					CursorMan.showMouse(true);

					_gfx->setViewport(_viewArea);
				} else if (toggleSoundRect.contains(mousePos)) {
					// Toggle sounds
				} else if (cycleRect.contains(mousePos)) {
					// Cycle between crawl, walk or run
					// It can fail if there is no room
				} else if (backRect.contains(mousePos))
					cont = false; // Back to game
				break;
			default:
				break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		drawFrame();
		if (surface)
			_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, menuTexture);

		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	_savedScreen = nullptr;

	originalSurface->free();
	delete originalSurface;
	surface->free();
	delete surface;

	delete menuTexture;
	pauseToken.clear();
	CursorMan.showMouse(false);
}

void CastleEngine::drawFullscreenEndGameAndWait() {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00));
	surface->fillRect(_viewArea, _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00));
	surface->copyRectToSurface(*_endGameBackgroundFrame, 46, 38, Common::Rect(0, 0, _endGameBackgroundFrame->w, _endGameBackgroundFrame->h));

	Common::Event event;
	bool cont = true;
	bool magisterAlive = true;
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				if (magisterAlive) {
					surface->copyRectToSurface(*_endGameThroneFrame, 121, 52, Common::Rect(0, 0, _endGameThroneFrame->w - 1, _endGameThroneFrame->h));
					magisterAlive = false;
				} else
					cont = false;
				break;

			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionShoot) {
					if (magisterAlive) {
						surface->copyRectToSurface(*_endGameThroneFrame, 121, 52, Common::Rect(0, 0, _endGameThroneFrame->w - 1, _endGameThroneFrame->h));
						magisterAlive = false;
					} else
						cont = false;
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				break;
			default:
				break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		drawBorder();
		if (_currentArea)
			drawUI();

		drawFullscreenSurface(surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	surface->free();
	delete surface;
}

void CastleEngine::drawFullscreenGameOverAndWait() {
	Common::Event event;
	bool cont = true;

	int score = _gameStateVars[k8bitVariableScore];
	int spiritsDestroyed = _gameStateVars[k8bitVariableSpiritsDestroyed];

	Common::String keysCollectedString;
	if (isDOS())
		keysCollectedString = _messagesList[130];
	else if (isSpectrum()) {
		if (_language == Common::EN_ANY)
			keysCollectedString = "X COLLECTED";
		else if (_language == Common::ES_ESP)
			keysCollectedString = "X RECOGIDAS";
		else
			error("Language not supported");
	}

	if (isDOS() && _keysCollected.size() == 0)
		keysCollectedString = _messagesList[128];
	else
		Common::replace(keysCollectedString, "X", Common::String::format("%d", _keysCollected.size()));
	keysCollectedString = centerAndPadString(keysCollectedString, 15);

	Common::String scoreString;
	if (isDOS())
		scoreString = _messagesList[131];
	else if (isSpectrum() || isCPC()) {
		if (_language == Common::EN_ANY)
			scoreString = "SCORE XXXXXXX";
		else if (_language == Common::ES_ESP)
			scoreString = "PUNTOS XXXXXXX";
		else
			scoreString = "SCORE XXXXXXX";
	}

	Common::replace(scoreString, "XXXXXXX", Common::String::format("%07d", score));
	scoreString = centerAndPadString(scoreString, 15);

	Common::String spiritsDestroyedString;
	if (isDOS())
		spiritsDestroyedString = _messagesList[133];
	else if (isSpectrum() || isCPC()) {
		if (_language == Common::EN_ANY)
			spiritsDestroyedString = "X DESTROYED";
		else if (_language == Common::ES_ESP)
			spiritsDestroyedString = "X DESTRUIDOS";
		else
			spiritsDestroyedString = "X DESTROYED";
	}

	Common::replace(spiritsDestroyedString, "X", Common::String::format("%d", spiritsDestroyed));
	spiritsDestroyedString = centerAndPadString(spiritsDestroyedString, 15);
	_droppingGateStartTicks = _ticks;

	if (isDOS()) {
		// TODO: playSound(X, false, _soundFxHandle);
	} else if (isSpectrum() || isCPC()) {
		playSound(9, false, _soundFxHandle);
	}

	if (!isDOS() && hasEscaped()) {
		insertTemporaryMessage(_messagesList[5], _countdown - 1);
	}

	while (!shouldQuit() && cont) {
		if (_temporaryMessageDeadlines.empty()) {
			insertTemporaryMessage(scoreString, _countdown - 2);
			insertTemporaryMessage(spiritsDestroyedString, _countdown - 4);
			insertTemporaryMessage(keysCollectedString, _countdown - 6);
			if (!isDOS() && hasEscaped()) {
				insertTemporaryMessage(_messagesList[5], _countdown - 8);
			}
		}

		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				cont = false;
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionShoot || event.customType == kActionChangeMode || event.customType == kActionSkip) {
					cont = false;
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				break;
			default:
				break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		drawFrame();

		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}
}

// Same as FreescapeEngine::executeExecute but updates the spirits destroyed counter
void CastleEngine::executeDestroy(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction._destination > 0) {
		objectID = instruction._destination;
		areaID = instruction._source;
	} else {
		objectID = instruction._source;
	}

	debugC(1, kFreescapeDebugCode, "Destroying obj %d in area %d!", objectID, areaID);
	assert(_areaMap.contains(areaID));
	Object *obj = _areaMap[areaID]->objectWithID(objectID);
	assert(obj); // We know that an object should be there

	if (!obj->isDestroyed() && obj->getType() == kSensorType && isCastle()) {
		_shootingFrames = 0;
		_gfx->_inkColor = _currentArea->_inkColor;
		_gfx->_shakeOffset = Common::Point();
	}

	if (obj->isDestroyed())
		debugC(1, kFreescapeDebugCode, "WARNING: Destroying obj %d in area %d already destroyed!", objectID, areaID);

	obj->makeInvisible();
	obj->destroy();
}

void CastleEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source;
	_currentAreaMessages.clear();
	if (index == 128 && isSpectrum()) {
		drawFullscreenRiddleAndWait(8);
		return;
	} else if (index == 128 && isDemo()) {
		drawFullscreenRiddleAndWait(18);
		return;
	} else if (index >= 129) {
		index = index - 129;
		drawFullscreenRiddleAndWait(index);
		return;
	}
	if (isAmiga() || isAtariST()) {
		index = index + 10;
	}
	debugC(1, kFreescapeDebugCode, "Printing message %d: \"%s\"", index, _messagesList[index].c_str());
	insertTemporaryMessage(_messagesList[index], _countdown - 3);
}

void CastleEngine::executeRedraw(FCLInstruction &instruction) {
	FreescapeEngine::executeRedraw(instruction);
	tryToCollectKey();
}

void CastleEngine::loadAssets() {
	FreescapeEngine::loadAssets();

	addGhosts();
	_endArea = 1;
	_endEntrance = 42;

	_timeoutMessage = _messagesList[1];
	// Shield is unused in Castle Master
	_noEnergyMessage = _messagesList[2];
	_crushedMessage = _messagesList[3];
	_fallenMessage = _messagesList[4];
	_outOfReachMessage = _messagesList[7];
	_noEffectMessage = _messagesList[8];

	if (!isAmiga() && !isCPC()) {
		Graphics::Surface *tmp;
		tmp = loadBundledImage("castle_gate", !isDOS());
		_gameOverBackgroundFrame = new Graphics::ManagedSurface;
		_gameOverBackgroundFrame->copyFrom(*tmp);
		_gameOverBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat);
		tmp->free();
		delete tmp;
	}

	Common::List<int> globalIds = _areaMap[255]->getEntranceIds();
	for (auto &it : _areaMap) {
		if (it._value->getAreaID() == 255)
			continue;

		it._value->addStructure(_areaMap[255]);

		if (isDOS() || isAmiga() || isAtariST()) {
			if (it._value->objectWithID(125)) {
				_areaMap[it._key]->addGroupFromArea(195, _areaMap[255]);
				//group = (Group *)_areaMap[it._key]->objectWithID(195);

				_areaMap[it._key]->addGroupFromArea(212, _areaMap[255]);
				//group = (Group *)_areaMap[it._key]->objectWithID(212);
			}

			if (it._value->objectWithID(126)) {
				_areaMap[it._key]->addGroupFromArea(191, _areaMap[255]);
				//group = (Group *)_areaMap[it._key]->objectWithID(191);
			}

			if (it._value->objectWithID(127)) {
				_areaMap[it._key]->addGroupFromArea(182, _areaMap[255]);
				//group = (Group *)_areaMap[it._key]->objectWithID(193);
			}
		}

		for (auto &id : globalIds) {
			if (it._value->entranceWithID(id))
				continue;

			Object *obj = _areaMap[255]->entranceWithID(id);
			assert(obj);
			assert(obj->getType() == ObjectType::kEntranceType);
			// The entrance is not in the current area, so we need to add it
			it._value->addObjectFromArea(id, _areaMap[255]);
		}
	}
	_areaMap[1]->addFloor();
	_areaMap[2]->addFloor();

}

void CastleEngine::loadRiddles(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);

	Common::Array<Common::Point> origins;
	for (int i = 0; i < number; i++) {
		Common::Point origin;
		origin.x = file->readByte();
		origin.y = file->readByte();
		debugC(1, kFreescapeDebugParser, "riddle %d origin: %d, %d", i, origin.x, origin.y);
		origins.push_back(origin);
	}

	debugC(1, kFreescapeDebugParser, "Riddle table:");
	int maxLineSize = (isSpectrum()) ? 20 : 24;

	for (int i = 0; i < number; i++) {
		Riddle riddle;
		riddle._origin = origins[i];
		int numberLines = file->readByte();
		debugC(1, kFreescapeDebugParser, "riddle %d number of lines: %d", i, numberLines);

		int8 x, y;
		for (int j = 0; j < numberLines; j++) {

			x = file->readByte();
			y = file->readByte();
			int size = file->readByte();
			const uint32 recordOffset = file->pos() - 3;
			debugC(1, kFreescapeDebugParser, "size: %d (max %d?)", size, maxLineSize);

			// Castle CPC French has one malformed riddle record in CM.BIN where the
			// stored size for "NOUS PARVIENT" is 11 instead of 13.
			if (isCPC() && _language == Common::FR_FRA && recordOffset == 0x86d && size == 11)
				size = 13;

			Common::String message = "";
			if (size == 255) {
				size = 19;
				while (size-- > 0)
					message = message + "*";

				debugC(1, kFreescapeDebugParser, "'%s' with offset: %d, %d", message.c_str(), x, y);
				riddle._lines.push_back(RiddleText(x, y, message));
				continue;
			} else if (size > maxLineSize) {
				assert(0);
			} else if (size == 0) {
				assert(0);
			}

			debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
			if (i == 20 && j == 1 && _language == Common::ES_ESP)
				size = size + 3;

			while (size-- > 0) {
				byte c = file->readByte();
				if (c > 0x7F) {
					file->seek(-1, SEEK_CUR);
					break;
				} else if (c != 0)
					message = message + c;
			}

			if (isAmiga() || isAtariST())
				debug("extra byte: %x", file->readByte());
			debugC(1, kFreescapeDebugParser, "'%s' with offset: %d, %d", message.c_str(), x, y);

			riddle._lines.push_back(RiddleText(x, y, message));
		}
		_riddleList.push_back(riddle);
	}
	debugC(1, kFreescapeDebugParser, "End of riddles at %" PRIx64, file->pos());
}

void CastleEngine::drawFullscreenRiddleAndWait(uint16 riddle) {
	debugC(1, kFreescapeDebugCode, "Printing fullscreen riddle %d", riddle);

	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}

	_savedScreen = _gfx->getScreenshot();
	int frontColor = 6;
	switch (_renderMode) {
		case Common::kRenderZX:
			frontColor = 7;
			break;
		case Common::kRenderCPC:
			frontColor = _gfx->_inkColor;
			break;
		default:
			break;
	}
	uint8 r, g, b;
	_gfx->readFromPalette(frontColor, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	if (isAmiga())
		front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0xAA, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);

	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionSkip) {
					cont = false;
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				break;
			case Common::EVENT_RBUTTONDOWN:
				// fallthrough
			case Common::EVENT_LBUTTONDOWN:
				if (isTouchscreenActive())
					cont = false;
				break;
			default:
				break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		drawBorder();
		if (_currentArea) {
			// Draw both UI and riddle on the same surface, since
			// drawFullscreenSurface uses a single shared texture.
			uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);
			surface->fillRect(Common::Rect(0, 0, _screenW, _screenH), gray);
			drawPlatformUI(surface);
		}
		drawRiddle(riddle, front, transparent, surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	_savedScreen = nullptr;
	surface->free();
	delete surface;
}

void CastleEngine::drawRiddle(uint16 riddle, uint32 front, uint32 back, Graphics::Surface *surface) {
	int x = 0;
	int y = 0;
	int maxWidth = 136;

	if (isDOS()) {
		x = 40;
		y = 34;
	} else if (isCPC()) {
		x = 40;
		y = 46;
		maxWidth = 139;
	} else if (isSpectrum()) {
		x = 64;
		y = 37;
	} else if (isAmiga()) {
		x = 32;
		y = 33;
		maxWidth = 139;
	}
	// Draw rope lines and nail above the riddle frame (CPC)
	if (isCPC() && _riddleNailFrame) {
		int nailX = x + (_viewArea.width() - _riddleNailFrame->w) / 2;
		int nailY = _viewArea.top + 2;
		int nailCenterX = nailX + _riddleNailFrame->w / 2;
		int nailCenterY = nailY + _riddleNailFrame->h / 2;
		// Rope lines first, then nail on top
		surface->drawLine(nailCenterX, nailCenterY, x, y, front);
		surface->drawLine(nailCenterX, nailCenterY, x + _viewArea.width() - 1, y, front);
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_riddleNailFrame, nailX, nailY, Common::Rect(0, 0, _riddleNailFrame->w, _riddleNailFrame->h), 0);
	}

	// Draw riddle frame borders (if available), clipped to viewport
	if (_riddleTopFrame) {
		Common::Rect srcRect(0, 0, _riddleTopFrame->w, _riddleTopFrame->h);
		Common::Rect destRect(x, y, x + _riddleTopFrame->w, y + _riddleTopFrame->h);
		destRect.clip(_viewArea);
		srcRect = Common::Rect(destRect.left - x, destRect.top - y, destRect.right - x, destRect.bottom - y);
		if (srcRect.isValidRect() && !srcRect.isEmpty())
			surface->copyRectToSurface((const Graphics::Surface)*_riddleTopFrame, destRect.left, destRect.top, srcRect);
		y += _riddleTopFrame->h;
	}
	if (_riddleBackgroundFrame) {
		for (; y < maxWidth;) {
			Common::Rect srcRect(0, 0, _riddleBackgroundFrame->w, _riddleBackgroundFrame->h);
			Common::Rect destRect(x, y, x + _riddleBackgroundFrame->w, y + _riddleBackgroundFrame->h);
			destRect.clip(_viewArea);
			srcRect = Common::Rect(destRect.left - x, destRect.top - y, destRect.right - x, destRect.bottom - y);
			if (srcRect.isValidRect() && !srcRect.isEmpty())
				surface->copyRectToSurface((const Graphics::Surface)*_riddleBackgroundFrame, destRect.left, destRect.top, srcRect);
			y += _riddleBackgroundFrame->h;
		}
	}
	if (_riddleBottomFrame) {
		Common::Rect srcRect(0, 0, _riddleBottomFrame->w, _riddleBottomFrame->h - 1);
		Common::Rect destRect(x, maxWidth, x + _riddleBottomFrame->w, maxWidth + _riddleBottomFrame->h - 1);
		destRect.clip(_viewArea);
		srcRect = Common::Rect(destRect.left - x, destRect.top - maxWidth, destRect.right - x, destRect.bottom - maxWidth);
		if (srcRect.isValidRect() && !srcRect.isEmpty())
			surface->copyRectToSurface((const Graphics::Surface)*_riddleBottomFrame, destRect.left, destRect.top, srcRect);
	}

	Common::Array<RiddleText> riddleMessages = _riddleList[riddle]._lines;
	x = _riddleList[riddle]._origin.x;
	y = _riddleList[riddle]._origin.y;

	if (isDOS()) {
		x = 38;
		y = 33;
	} else if (isCPC()) {
		x = 40;
		y = 33;
	} else if (isSpectrum()) {
		x = 64;
		y = 36;
	} else if (isAmiga()) {
		x = 40;
		y = 32;
	}

	for (int i = 0; i < int(riddleMessages.size()); i++) {
		x = x + riddleMessages[i]._dx;
		y = y + riddleMessages[i]._dy;
		drawRiddleStringInSurface(riddleMessages[i]._text, x, y, front, back, surface);
	}
	drawFullscreenSurface(surface);
}

void CastleEngine::drawRiddleStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface) {
	Common::String ustr = str;
	ustr.toUppercase();
	if (isDOS() || isAmiga() || isAtariST()) {
		_fontRiddle.setBackground(backColor);
		_fontRiddle.drawString(surface, ustr, x, y, _screenW, fontColor);
	} else {
		_font.setBackground(backColor);
		_font.drawString(surface, ustr, x, y, _screenW, fontColor);
	}
}

void CastleEngine::drawEnergyMeter(Graphics::Surface *surface, Common::Point origin) {
	if (_strenghtBackgroundFrame)
		surface->copyRectToSurface((const Graphics::Surface)*_strenghtBackgroundFrame, origin.x, origin.y, Common::Rect(0, 0, _strenghtBackgroundFrame->w, _strenghtBackgroundFrame->h));

	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 back = 0;

	if (isDOS() || isAmiga() || isAtariST())
		back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	int strength = _gameStateVars[k8bitVariableShield];

	// When strength < 4, the bar and weights shift down since smaller weights have smaller diameter
	int extraYOffset = (strength < 4) ? (4 - strength) : 0;

	Common::Point barFrameOrigin = origin;

	if (_strenghtBarFrame) {
		if (isDOS())
			barFrameOrigin += Common::Point(5, 6 + extraYOffset);
		else if (isSpectrum())
			barFrameOrigin += Common::Point(0, 6 + extraYOffset);
		else if (isCPC())
			barFrameOrigin += Common::Point(0, 6 + extraYOffset);

		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtBarFrame, barFrameOrigin.x, barFrameOrigin.y, Common::Rect(0, 0, _strenghtBarFrame->w, _strenghtBarFrame->h), black);
	}

	Common::Point weightPoint;
	int frameIdx = -1;

	if (_strenghtWeightsFrames.empty())
		return;

	int weightWidth = _strenghtWeightsFrames[0]->w;

	// Weight discs overlap: step is smaller than sprite width (3 pixels in original ZX assembly).
	// Each disc is drawn at pixel-level precision, converging from outside toward center.
	int weightStep;
	int weightOffset;
	int rightWeightPos;

	if (isCPC()) {
		weightStep = 3;
		weightOffset = 9;
		rightWeightPos = 59;
	} else if (isSpectrum()) {
		weightStep = 3;
		weightOffset = 5;
		rightWeightPos = 63;
	} else if (isAmiga() || isAtariST()) {
		weightStep = 3;
		weightOffset = 10;
		rightWeightPos = 62;
	} else { // DOS
		weightStep = 3;
		weightOffset = 10;
		rightWeightPos = 62;
	}

	// Weights are drawn 6 pixels above the bar (at the bar position minus 6)
	int weightY = origin.y + extraYOffset;

	// Left side
	weightPoint = Common::Point(origin.x + weightOffset, weightY);
	frameIdx = strength % 4;

	if (frameIdx != 0) {
		frameIdx = 4 - frameIdx;
		if (isSpectrum())
			surface->copyRectToSurface((const Graphics::Surface)*_strenghtWeightsFrames[frameIdx], weightPoint.x, weightPoint.y, Common::Rect(0, 0, weightWidth, _strenghtWeightsFrames[frameIdx]->h));
		else
			surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtWeightsFrames[frameIdx], weightPoint.x, weightPoint.y, Common::Rect(0, 0, weightWidth, _strenghtWeightsFrames[frameIdx]->h), back);
		weightPoint += Common::Point(weightStep, 0);
	}

	for (int i = 0; i < strength / 4; i++) {
		if (isSpectrum())
			surface->copyRectToSurface((const Graphics::Surface)*_strenghtWeightsFrames[0], weightPoint.x, weightPoint.y, Common::Rect(0, 0, weightWidth, _strenghtWeightsFrames[0]->h));
		else
			surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtWeightsFrames[0], weightPoint.x, weightPoint.y, Common::Rect(0, 0, weightWidth, _strenghtWeightsFrames[0]->h), back);
		weightPoint += Common::Point(weightStep, 0);
	}

	// Right side: draw from innermost to outermost (left to right) so that each
	// weight's background pixels don't overwrite the previous weight's disc
	// (the disc is at local pixels 1-2, on the left side of the 8-pixel sprite).
	int numFullRight = strength / 4;
	int hasPartial = (strength % 4 != 0) ? 1 : 0;
	int totalRight = numFullRight + hasPartial;

	weightPoint = Common::Point(origin.x + rightWeightPos - (totalRight - 1) * weightStep, weightY);

	for (int i = 0; i < numFullRight; i++) {
		if (isSpectrum())
			surface->copyRectToSurface((const Graphics::Surface)*_strenghtWeightsFrames[0], weightPoint.x, weightPoint.y, Common::Rect(0, 0, weightWidth, _strenghtWeightsFrames[0]->h));
		else
			surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtWeightsFrames[0], weightPoint.x, weightPoint.y, Common::Rect(0, 0, weightWidth, _strenghtWeightsFrames[0]->h), back);
		weightPoint += Common::Point(weightStep, 0);
	}

	if (hasPartial) {
		frameIdx = 4 - (strength % 4);
		if (isSpectrum())
			surface->copyRectToSurface((const Graphics::Surface)*_strenghtWeightsFrames[frameIdx], weightPoint.x, weightPoint.y, Common::Rect(0, 0, weightWidth, _strenghtWeightsFrames[frameIdx]->h));
		else
			surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtWeightsFrames[frameIdx], weightPoint.x, weightPoint.y, Common::Rect(0, 0, weightWidth, _strenghtWeightsFrames[frameIdx]->h), back);
	}
}

void CastleEngine::addGhosts() {
}

void CastleEngine::checkSensors() {
	if (_lastTick == _ticks)
		return;

	_lastTick = _ticks;

	if (_sensors.empty()) {
		_gfx->_shakeOffset = Common::Point();
		return;
	}

	if (!ghostInArea()) {
		_mixer->stopHandle(_soundFxGhostHandle);
		_gfx->_shakeOffset = Common::Point();
		return;
	}

	if (_disableSensors)
		return;

	/*if (!_mixer->isSoundHandleActive(_soundFxGhostHandle)) {
		_speaker->play(Audio::PCSpeaker::kWaveFormSquare, 25.0f, -1);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxGhostHandle, _speaker, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}*/

	// This is the frequency to shake the screen
	if (_ticks % 5 == 0) {
		if (_underFireFrames <= 0)
			_underFireFrames = 1;
	}

	// This is the frequency to take damage
	if (_ticks % 100 == 0) {
		takeDamageFromSensor();
	}
}

bool CastleEngine::ghostInArea() {
	for (auto &it : _sensors) {
		if (it->isDestroyed() || it->isInvisible())
			continue;
		return true;
		break;
	}
	return false;
}

void CastleEngine::drawSensorShoot(Sensor *sensor) {
	if (isSpectrum() || isCPC()) {
		_gfx->_inkColor = 1 + (_gfx->_inkColor + 1) % 7;
	} else if (isDOS()) {
		float shakeIntensity = 10;
		Common::Point shakeOffset;
		shakeOffset.x = (_rnd->getRandomNumber(10) / 10.0 - 0.5f) * shakeIntensity;
		shakeOffset.y = (_rnd->getRandomNumber(10) / 10.0 - 0.5f) * shakeIntensity;
		_gfx->_shakeOffset = shakeOffset;
	} else {
		/* TODO */
	}
}

void CastleEngine::tryToCollectKey() {
	if (_gameStateVars[32] > 0) { // Key collected!
		if (_keysCollected.size() < 10) {
			_gameStateVars[31]++;
			setGameBit(_gameStateVars[32]);
			_keysCollected.push_back(_gameStateVars[32]);
		}
		_gameStateVars[32] = 0;
	}
}

void CastleEngine::updateTimeVariables() {
	if (_gameStateControl != kFreescapeGameStatePlaying)
		return;
	// This function only executes "on collision" room/global conditions
	tryToCollectKey();

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	if (_lastMinute != minutes / 2) {
		int spiritsDestroyed = _gameStateVars[k8bitVariableSpiritsDestroyed];
		_lastMinute = minutes / 2;
		_spiritsMeter++;
		_spiritsMeterPosition = _spiritsMeter * (_spiritsToKill - spiritsDestroyed) / _spiritsToKill;
		if (_spiritsMeterPosition >= _spiritsMeterMax)
			_countdown = -1;
	}

	if (_lastTenSeconds != seconds / 10) {
		_lastTenSeconds = seconds / 10;
		executeLocalGlobalConditions(false, false, true);
	}
}

void CastleEngine::borderScreen() {
	if (isAmiga() && isDemo())
		return; // Skip character selection

	if (isSpectrum() || isCPC() || isC64())
		FreescapeEngine::borderScreen();
	else {
		uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
		Graphics::Surface *surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		surface->fillRect(_fullscreenViewArea, color);

		int x = 40;
		int y = 34;

		Common::Array<RiddleText> selectMessage = _riddleList[19]._lines;
		for (int i = 0; i < int(selectMessage.size()); i++) {
			x = x + selectMessage[i]._dx;
			y = y + selectMessage[i]._dy;
			// Color is not important, as the font has already a palette
			drawStringInSurface(selectMessage[i]._text, x, y, 0, 0, surface);
		}
		drawFullscreenSurface(surface);
		drawBorderScreenAndWait(surface, 6 * 60);
		surface->free();
		delete surface;
	}

	if (!isCastleMaster2())
		selectCharacterScreen();
}

void CastleEngine::drawOption() {
	_gfx->setViewport(_fullscreenViewArea);
	if (_option) {
		if (!_optionTexture) {
			Graphics::Surface *title = _gfx->convertImageFormatIfNecessary(_option);
			_optionTexture = _gfx->createTexture(title);
			title->free();
			delete title;
		}
		_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _optionTexture);
	}
	_gfx->setViewport(_viewArea);
}

void CastleEngine::selectCharacterScreen() {
	Common::Array<Common::String> lines;
	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, color);

	if (isSpectrum() || isCPC()) {
		if (_language == Common::ES_ESP) {
			// No accent in "príncipe" since it is not supported by the font
			lines.push_back(centerAndPadString("*******************", 21));
			lines.push_back(centerAndPadString("Seleccion el ", 21));
			lines.push_back(centerAndPadString("personaje que quiera", 21));
			lines.push_back(centerAndPadString("ser y pulse enter", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("1. Principe", 21));
			lines.push_back(centerAndPadString("2. Princesa", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("*******************", 21));
		} else if (isCPC() && _language == Common::FR_FRA) {
			// Original Castle Master CPC multilingual strings from CM.BIN @ 0x143.
			lines.push_back(centerAndPadString("*******************", 21));
			lines.push_back(centerAndPadString("SELECTIONNEZ LE", 21));
			lines.push_back(centerAndPadString("PERSONNAGE DESIRE ET", 21));
			lines.push_back(centerAndPadString("APPUYEZ SUR RETURN", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("1. PRINCE", 21));
			lines.push_back(centerAndPadString("2. PRINCESSE", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("*******************", 21));
		} else if (isCPC() && _language == Common::DE_DEU) {
			// Original Castle Master CPC multilingual strings from CM.BIN @ 0x1ac.
			lines.push_back(centerAndPadString("*******************", 21));
			lines.push_back(centerAndPadString("GEWUNSCHTE FIGUR", 21));
			lines.push_back(centerAndPadString("WAHLEN UND", 21));
			lines.push_back(centerAndPadString("RETURN DRUCKEN", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("1. PRINZ", 21));
			lines.push_back(centerAndPadString("2. PRINZESSIN", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("*******************", 21));
		} else if (isCPC()) {
			// Original Castle Master CPC English strings from CM.BIN @ 0xda.
			lines.push_back(centerAndPadString("*******************", 21));
			lines.push_back(centerAndPadString("SELECT THE CHARACTER", 21));
			lines.push_back(centerAndPadString("YOU WISH TO PLAY", 21));
			lines.push_back(centerAndPadString("AND PRESS RETURN", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("1. PRINCE", 21));
			lines.push_back(centerAndPadString("2. PRINCESS", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("*******************", 21));
		} else {
			lines.push_back(centerAndPadString("*******************", 21));
			lines.push_back(centerAndPadString("Select the character", 21));
			lines.push_back(centerAndPadString("you wish to play", 21));
			lines.push_back(centerAndPadString("and press enter", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("1. Prince  ", 21));
			lines.push_back(centerAndPadString("2. Princess", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("*******************", 21));
		}
		drawStringsInSurface(lines, surface);
	} else {
		int x = 0;
		int y = 0;

		Common::Array<RiddleText> selectMessage = _riddleList[21]._lines;
		for (int i = 0; i < int(selectMessage.size()); i++) {
			x = x + selectMessage[i]._dx;
			y = y + selectMessage[i]._dy;
			drawStringInSurface(selectMessage[i]._text, x, y, color, color, surface);
		}
		drawFullscreenSurface(surface);
	}

	if (isTouchscreenActive()) {
		CursorMan.setDefaultArrowCursor();
	}
	_system->lockMouse(false);
	CursorMan.showMouse(true);

	// Calculate tap/click rectangles from actual rendered text positions.
	// lines[5] = prince, lines[6] = princess for ZX/CPC.
	// For DOS, use riddle text line positions.
	Common::Rect princeSelector, princessSelector;
	if (isSpectrum() || isCPC()) {
		int x = _viewArea.left + 3;
		int lineHeight = 12; // Castle Master line spacing in drawStringsInSurface
		int princeY = _viewArea.top + 3 + 5 * lineHeight;
		int princessY = _viewArea.top + 3 + 6 * lineHeight;
		// Use the full padded line (what's actually rendered on screen)
		princeSelector = _font.getBoundingBox(lines[5], x, princeY);
		princessSelector = _font.getBoundingBox(lines[6], x, princessY);
	} else {
		// DOS: text comes from _riddleList[21], calculate from actual riddle line positions
		Common::Array<RiddleText> selectMessage = _riddleList[21]._lines;
		int x = 0, y = 0;
		for (int i = 0; i < int(selectMessage.size()); i++) {
			x += selectMessage[i]._dx;
			y += selectMessage[i]._dy;
			if (i == int(selectMessage.size()) - 2)
				princeSelector = _font.getBoundingBox(selectMessage[i]._text, x, y);
			else if (i == int(selectMessage.size()) - 1)
				princessSelector = _font.getBoundingBox(selectMessage[i]._text, x, y);
		}
	}

	// On touchscreen, highlight the tap areas with red outlines (expand 1px for readability)
	princeSelector.grow(1);
	princessSelector.grow(1);
	if (isTouchscreenActive()) {
		uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0x00, 0x00);
		surface->frameRect(princeSelector, red);
		surface->frameRect(princessSelector, red);
	}

	bool selected = false;
	while (!selected) {
		Common::Event event;
		Common::Point mouse;
		while (_eventManager->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				quitGame();
				return;

			// Left mouse click or touchscreen tap
			case Common::EVENT_LBUTTONDOWN:
				// fallthrough
			case Common::EVENT_RBUTTONDOWN:
				mouse.x = _screenW * event.mouse.x / g_system->getWidth();
				mouse.y = _screenH * event.mouse.y / g_system->getHeight();

				if (princeSelector.contains(mouse)) {
					selected = true;
					// Nothing, since game bit should be already zero
				} else if (princessSelector.contains(mouse)) {
					selected = true;
					setGameBit(32);
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				_gfx->clear(0, 0, 0, true);
				break;
			default:
				break;
			}
			switch (event.customType) {
				case kActionSelectPrince:
					selected = true;
					// Nothing, since game bit should be already zero
					break;
				case kActionSelectPrincess:
					selected = true;
					setGameBit(32);
					break;
				default:
					break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		if (_option)
			drawOption();
		else
			drawBorder();
		drawFullscreenSurface(surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}
	_system->lockMouse(true);
	CursorMan.showMouse(false);
	_gfx->clear(0, 0, 0, true);

}

void CastleEngine::drawLiftingGate(Graphics::Surface *surface) {
	uint32 keyColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x24, 0xA5);
	int duration = 0;

	if (isDOS())
		duration = 250;
	else if (isSpectrum())
		duration = 100;
	else if (isAmiga() || isAtariST())
		duration = 250;
	else if (isCPC())
		duration = 100;

	if ((_gameStateControl == kFreescapeGameStateStart || _gameStateControl == kFreescapeGameStateRestart) && _ticks <= duration) { // Draw the _gameOverBackgroundFrame gate lifting up slowly
		int gate_w = _gameOverBackgroundFrame->w;
		int gate_h = _gameOverBackgroundFrame->h;

		// The gate should move up by the height of the view area to disappear.
		int y_offset = _ticks * _viewArea.height() / duration;

		// Initial position is with the gate bottom at the view area bottom.
		int dx = _viewArea.left + (_viewArea.width() - gate_w) / 2;
		int dy = (_viewArea.bottom - gate_h) - y_offset;

		// Define destination rect for the full gate
		Common::Rect destRect(dx, dy, dx + gate_w, dy + gate_h);

		// Find intersection with view area to clip
		Common::Rect clippedDest = destRect.findIntersectingRect(_viewArea);

		// If there is something to draw
		if (clippedDest.isValidRect() && clippedDest.width() > 0 && clippedDest.height() > 0) {
			// Adjust source rect based on clipping
			int src_x = clippedDest.left - destRect.left;
			int src_y = clippedDest.top - destRect.top;
			Common::Rect clippedSrc(src_x, src_y, src_x + clippedDest.width(), src_y + clippedDest.height());

			// Draw the clipped part
			surface->copyRectToSurfaceWithKey(*_gameOverBackgroundFrame, clippedDest.left, clippedDest.top, clippedSrc, keyColor);
		}
	}
}

void CastleEngine::drawDroppingGate(Graphics::Surface *surface) {
	if (!isDOS() && hasEscaped())
		return; // No gate dropping when the player escaped

	if (_droppingGateStartTicks <= 0)
		return;

	uint32 keyColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x24, 0xA5);
	int duration = 60;
	int ticks = _ticks - _droppingGateStartTicks;

	if (_gameStateControl == kFreescapeGameStateEnd && _ticks <= _droppingGateStartTicks + duration) { // Draw the _gameOverBackgroundFrame gate dropping down slowly
		int gate_w = _gameOverBackgroundFrame->w;
		int gate_h = _gameOverBackgroundFrame->h;

		// The gate should move down by the height of the view area to appear.
		int y_offset = (duration - ticks) * _viewArea.height() / duration;

		// Initial position is with the gate bottom at the view area bottom.
		int dx = _viewArea.left + (_viewArea.width() - gate_w) / 2;
		int dy = (_viewArea.bottom - gate_h) - y_offset;

		// Define destination rect for the full gate
		Common::Rect destRect(dx, dy, dx + gate_w, dy + gate_h);

		// Find intersection with view area to clip
		Common::Rect clippedDest = destRect.findIntersectingRect(_viewArea);

		// If there is something to draw
		if (clippedDest.isValidRect() && clippedDest.width() > 0 && clippedDest.height() > 0) {
			// Adjust source rect based on clipping
			int src_x = clippedDest.left - destRect.left;
			int src_y = clippedDest.top - destRect.top;
			Common::Rect clippedSrc(src_x, src_y, src_x + clippedDest.width(), src_y + clippedDest.height());

			// Draw the clipped part
			surface->copyRectToSurfaceWithKey(*_gameOverBackgroundFrame, clippedDest.left, clippedDest.top, clippedSrc, keyColor);
		}
	} else {
		// Draw the gate fully down
		int gate_w = _gameOverBackgroundFrame->w;
		int gate_h = _gameOverBackgroundFrame->h;
		int dx = _viewArea.left + (_viewArea.width() - gate_w) / 2;
		int dy = (_viewArea.bottom - gate_h);
		surface->copyRectToSurfaceWithKey(*_gameOverBackgroundFrame, dx, dy, Common::Rect(0, 0, gate_w, gate_h), keyColor);
	}
}

Common::Error CastleEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	stream->writeUint32LE(_keysCollected.size());
	for (auto &it : _keysCollected) {
		stream->writeUint32LE(it);
	}

	stream->writeUint32LE(_spiritsMeter);

	for (auto &it : _areaMap) {
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_exploredAreas[it._key]);
	}

	return Common::kNoError;
}

Common::Error CastleEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	_keysCollected.clear();
	int numberKeys = stream->readUint32LE();
	for (int i = 0; i < numberKeys; i++) {
		_keysCollected.push_back(stream->readUint32LE());
	}

	_spiritsMeter = stream->readUint32LE();

	for (uint i = 0; i < _areaMap.size(); i++) {
		uint16 key = stream->readUint16LE();
		_exploredAreas[key] = stream->readUint32LE();
	}

	if (_useRockTravel) // Enable cheat
		setGameBit(k8bitGameBitTravelRock);

	for (auto &it : _areaMap) {
		it._value->resetAreaGroups();
	}
	return Common::kNoError;
}


void CastleEngine::drawBackground() {
	clearBackground();
	_gfx->drawBackground(_currentArea->_skyColor);

	if (_avoidRenderingFrames == 0 && _currentArea->isOutside()) {
		if (_background) {
			if (!_skyTexture)
				_skyTexture = _gfx->createTexture(_background->surfacePtr(), true);
			_gfx->drawSkybox(_skyTexture, _position);
			if (_thunderTextures.empty()) {
				for (auto &it : _thunderFrames ) {
					_thunderTextures.push_back(_gfx->createTexture(it->surfacePtr(), true));
				}
			}
			updateThunder();
		}
	}
}

void CastleEngine::updateThunder() {
	if (_thunderFrames.empty() || !_thunderFrames[0])
		return;

	if (_thunderFrameDuration > 0) {
		//debug("Thunder frame duration: %d", _thunderFrameDuration);
		//debug("Size: %f", 2 * _thunderOffset.length());
		//debug("Offset: %.1f, %.1f, %.1f", _thunderOffset.x(), _thunderOffset.y(), _thunderOffset.z());
		_gfx->drawThunder(_thunderTextures[_thunderFrameIndex], _position + _thunderOffset, 100);
		_thunderFrameDuration--;

		if (_thunderFrameDuration == 5)
			_gfx->clear(255, 255, 255);

		if (_thunderFrameDuration == 0)
			if (isSpectrum() || isCPC() || isDOS())
				playSound(8, false, _soundFxHandle);
		return;
	}

	if (_thunderTicks > 0) {
		//debug("Thunder ticks: %d", _thunderTicks);
		_thunderTicks--;
		if (_thunderTicks <= 0) {
			if (isDOS())
				_thunderFrameIndex = int(_rnd->getRandomNumber(_thunderTextures.size() - 1));
			_thunderFrameDuration = 10;
			_thunderOffset = Math::Vector3d();

			// Project to floor plane
			Math::Vector3d forward = _cameraFront; 
			forward.y() = 0;
			forward.normalize();

			Math::Angle yaw = Math::Angle(30.0f - (float)_rnd->getRandomNumber(60));
			Math::Angle pitch = Math::Angle(5.0f + (float)_rnd->getRandomNumber(20));

			// Rotate on floor plane
			float x = forward.x();
			forward.x() = forward.x() * yaw.getCosine() - forward.z() * yaw.getSine();
			forward.z() = x * yaw.getSine() + forward.z() * yaw.getCosine();

			// Rotate upwards
			Math::Vector3d tiltedForward = (forward * pitch.getCosine()) + (_upVector * pitch.getSine());
			_thunderOffset = tiltedForward * (400.0f + (float)_rnd->getRandomNumber(200));
		}
	} else {
		// Schedule next thunder, between 10 and 10 + 10 seconds
		_thunderTicks = 50 * (10 + _rnd->getRandomNumber(10));
	}
}

} // End of namespace Freescape
