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

#include "hollywood/gameplay/options_menu.h"

#include "common/events.h"
#include "common/file.h"
#include "common/formats/winexe.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kOptionsExecutableName = "MONSTERS.EXE";
const char *const kOptionsResource000Name = "RESOURCE.000";
const char *const kOptionsPaletteResourceType = "PALETA_OBJ_INT";
const char *const kOptionsPaletteResourceName = "obj_pal";
const char kOptionsQuitPromptText[] = "\xa8" "Est\xa0s seguro?";
const char kOptionsQuitYesText[] = "S";
const char kOptionsQuitNoText[] = "N";
const uint kOptionsResource000HeaderByteCount = 1;
const uint kOptionsResource000OffsetTableSize = 400;
const uint kOptionsResource000SizeTableSize = 400;
const uint kOptionsFramebufferEntry = 0x2a;
const uint kOptionsObjectPaletteResource000Entry = 0x31;
const uint kOptionsFramebufferSize = 0x78000;
const uint kOptionsViewportXOffset = 0x180;
const uint kOptionsObjectPaletteOffset = 0x210;
const uint kOptionsObjectPaletteSize = 0xf0;
const uint kOptionsObjectPalettePanelSize = 0x60;
const byte kOptionsTextActiveColor = 0xf1;
const byte kOptionsTextDisabledColor = 0xf0;
const byte kOptionsTextTestColor = 0xfa;
const byte kOptionsBarDisabledColor = 0xee;
const byte kOptionsBarEmptyColor = 0xe9;
const byte kOptionsStatusDotOffColor = 0xea;
const byte kOptionsStatusDotTestColor = 0xf9;
const uint32 kOptionsTickMillis = 10;
const byte kOptionsVolumeStep = 10;
const uint32 kOptionsVolumeInitialRepeatMillis = 160;
const uint32 kOptionsVolumeRepeatMillis = 60;
const int kOptionsGameplayPreviewX = 0x1a4 - kOptionsViewportXOffset;
const int kOptionsGameplayPreviewY = 0x24;
const int kOptionsGameplayPreviewWidth = 0xb8;
const int kOptionsGameplayPreviewHeight = 0x8a;
const int kOptionsTopStatusDotY = 0x24;
const int kOptionsSpeechStatusDotY = 0x271 - 0xc0;
const int kOptionsStatusDotSize = 0x0b;
const int kOptionsMusicStatusDotX = 0x290;
const int kOptionsSoundStatusDotX = 0x31a;
const int kOptionsTestStatusDotX = 0x38d;
const int kOptionsSpeechTextStatusDotX = 0x290;
const int kOptionsSpeechVoiceStatusDotX = 0x30b;
const int kOptionsSpeechBothStatusDotX = 0x386;
const int kOptionsQuitButtonShift = 9;
const int kOptionsQuitMainButtonLeft = 0x1a4;
const int kOptionsQuitMainButtonTop = 0x0f1;
const int kOptionsQuitMainButtonRight = 0x1ff;
const int kOptionsQuitMainButtonBottom = 0x10c;
const int kOptionsQuitChoiceTop = 0x188;
const int kOptionsQuitChoiceBottom = 0x1a3;
const int kOptionsQuitYesLeft = 0x1e4;
const int kOptionsQuitYesRight = 0x1ff;
const int kOptionsQuitNoLeft = 0x201;
const int kOptionsQuitNoRight = 0x21c;
const int kOptionsValueBarX = 0x15c;
const int kOptionsValueBarWidth = 200;
const int kOptionsValueBarHeight = 7;
const int kOptionsValueBarY[4] = { 0x7a, 0xcd, 0x120, 0x173 };
const int kOptionsToggleSquareX[2] = { 0x13a, 0x236 };
const int kOptionsToggleSquareY[4] = { 0x75, 0xc8, 0x11b, 0x16e };
const int kOptionsToggleSquareSize = 0x10;
const uint kOptionsMaximumLevel = 200;

GameplayOptionsMenu::GameplayOptionsMenu(HollywoodEngine *vm) :
		_vm(vm),
		_speechPreviewSampleId(0),
		_loaded(false),
		_confirmQuit(false),
		_loadedGame(false),
		_sceneMusicWasPlaying(false),
		_hasSpeechPreviewSample(false),
		_heldVolumeAction(kHitNone),
		_heldVolumeAccumulator(0),
		_heldVolumeRepeatMillis(kOptionsVolumeInitialRepeatMillis) {
	_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
}

void GameplayOptionsMenu::setAudioContext(const Common::Path &soundBank0ArchiveName,
		bool hasSpeechPreviewSample, uint16 speechPreviewSampleId) {
	_soundBank0ArchiveName = soundBank0ArchiveName;
	_hasSpeechPreviewSample = hasSpeechPreviewSample;
	_speechPreviewSampleId = speechPreviewSampleId;
}

bool GameplayOptionsMenu::run(const Common::Array<byte> &basePalette) {
	if (!_vm || !load())
		return false;

	preparePalette(basePalette);
	captureGameplayPreview();
	_vm->syncSoundSettings();
	_confirmQuit = false;
	_loadedGame = false;
	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	beginAudioSession();

	bool done = false;
	uint32 lastMillis = g_system->getMillis();
	while (!done && !Engine::shouldQuit()) {
		pollEvents(done);

		const uint32 now = g_system->getMillis();
		uint32 delta = now - lastMillis;
		lastMillis = now;
		if (delta > 250)
			delta = 250;
		_vm->cursor()->advance(delta);
		advanceHeldVolume(delta);

		present();
		g_system->delayMillis(kOptionsTickMillis);
	}

	endAudioSession();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	return !Engine::shouldQuit();
}

bool GameplayOptionsMenu::load() {
	if (_loaded)
		return true;

	if (!loadMenuFramebuffer())
		return false;

	if (!loadObjectPalette())
		warning("Failed to load Hollywood options menu palette from %s", kOptionsExecutableName);

	_loaded = true;
	return true;
}

bool GameplayOptionsMenu::loadMenuFramebuffer() {
	Common::File file;
	if (!file.open(Common::Path(kOptionsResource000Name))) {
		warning("Failed to open %s for Hollywood options menu", kOptionsResource000Name);
		return false;
	}

	const uint32 startupTablesSize = kOptionsResource000HeaderByteCount +
		kOptionsResource000OffsetTableSize + kOptionsResource000SizeTableSize;
	if ((uint32)file.size() < startupTablesSize) {
		warning("%s is too small for Hollywood options menu tables", kOptionsResource000Name);
		return false;
	}

	Common::Array<byte> offsetTable;
	offsetTable.resize(kOptionsResource000OffsetTableSize);
	file.seek(kOptionsResource000HeaderByteCount);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size()) {
		warning("Failed to read %s options menu offset table", kOptionsResource000Name);
		return false;
	}

	const uint tableOffset = kOptionsFramebufferEntry * 4;
	if (tableOffset + 4 > offsetTable.size()) {
		warning("%s options menu table entry is out of range", kOptionsResource000Name);
		return false;
	}

	const uint32 frameOffset = readUint32LE(offsetTable, tableOffset);
	if (frameOffset > (uint32)file.size() || kOptionsFramebufferSize > (uint32)file.size() - frameOffset) {
		warning("%s options menu framebuffer is out of range", kOptionsResource000Name);
		return false;
	}

	_menuFramebuffer.resize(kOptionsFramebufferSize);
	file.seek(frameOffset);
	if (file.read(_menuFramebuffer.data(), _menuFramebuffer.size()) != _menuFramebuffer.size()) {
		warning("Failed to read Hollywood options menu framebuffer");
		return false;
	}

	return true;
}

bool GameplayOptionsMenu::loadObjectPalette() {
	Common::ScopedPtr<Common::WinResources> exe(Common::WinResources::createFromEXE(Common::Path(kOptionsExecutableName)));
	if (exe) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(exe->getResource(
			Common::WinResourceID(kOptionsPaletteResourceType), Common::WinResourceID(kOptionsPaletteResourceName)));
		if (stream && stream->size() >= kOptionsObjectPaletteSize) {
			_objectPaletteTriples.resize(kOptionsObjectPaletteSize);
			return stream->read(_objectPaletteTriples.data(), _objectPaletteTriples.size()) ==
				_objectPaletteTriples.size();
		}
	}

	return loadObjectPaletteFromResource000();
}

bool GameplayOptionsMenu::loadObjectPaletteFromResource000() {
	Common::File file;
	if (!file.open(Common::Path(kOptionsResource000Name)))
		return false;

	const uint offsetTableOffset = kOptionsResource000HeaderByteCount + kOptionsObjectPaletteResource000Entry * 4;
	const uint sizeTableOffset = kOptionsResource000HeaderByteCount + kOptionsResource000OffsetTableSize +
		kOptionsObjectPaletteResource000Entry * 4;
	if ((uint32)file.size() < sizeTableOffset + 4)
		return false;

	file.seek(offsetTableOffset);
	const uint32 paletteOffset = file.readUint32LE();
	file.seek(sizeTableOffset);
	const uint32 paletteSize = file.readUint32LE();
	if (paletteSize < kOptionsObjectPalettePanelSize ||
			paletteOffset > (uint32)file.size() ||
			kOptionsObjectPalettePanelSize > (uint32)file.size() - paletteOffset)
		return false;

	_objectPaletteTriples.resize(kOptionsObjectPaletteSize);
	memset(_objectPaletteTriples.data(), 0, _objectPaletteTriples.size());
	file.seek(paletteOffset);
	return file.read(_objectPaletteTriples.data(), kOptionsObjectPalettePanelSize) ==
		kOptionsObjectPalettePanelSize;
}

void GameplayOptionsMenu::preparePalette(const Common::Array<byte> &basePalette) {
	_palette = basePalette;
	if (_palette.size() < kPaletteSize) {
		const uint oldSize = _palette.size();
		_palette.resize(kPaletteSize);
		memset(_palette.data() + oldSize, 0, _palette.size() - oldSize);
	}

	if (_objectPaletteTriples.size() >= kOptionsObjectPalettePanelSize &&
			_palette.size() >= kOptionsObjectPaletteOffset + kOptionsObjectPalettePanelSize) {
		memcpy(_palette.data() + kOptionsObjectPaletteOffset, _objectPaletteTriples.data(),
			kOptionsObjectPalettePanelSize);
	}
}

void GameplayOptionsMenu::pollEvents(bool &done) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			done = true;
			break;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			break;
		case Common::EVENT_KEYDOWN:
			handleKeyDown(event.kbd.keycode, done);
			break;
		case Common::EVENT_LBUTTONDOWN:
			_vm->cursor()->updatePosition(event.mouse);
			handleLeftClick(_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY(), done);
			break;
		case Common::EVENT_LBUTTONUP:
			stopHeldVolume();
			break;
		case Common::EVENT_RBUTTONDOWN:
			stopHeldVolume();
			if (_confirmQuit)
				_confirmQuit = false;
			else
				done = true;
			break;
		default:
			break;
		}
	}
}

void GameplayOptionsMenu::handleKeyDown(uint16 keycode, bool &done) {
	switch (keycode) {
	case Common::KEYCODE_ESCAPE:
		stopHeldVolume();
		if (_confirmQuit)
			_confirmQuit = false;
		else
			done = true;
		break;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		stopHeldVolume();
		if (_confirmQuit) {
			Engine::quitGame();
			done = true;
		}
		break;
	default:
		break;
	}
}

void GameplayOptionsMenu::handleLeftClick(uint16 cursorX, uint16 cursorY, bool &done) {
	GameplayState &state = _vm->gameState();
	const HitAction action = hitActionAt(cursorX, cursorY);
	bool syncSettings = false;
	bool refreshTestAudio = false;
	switch (action) {
	case kHitSave:
		disableTestAudio();
		_vm->saveGameDialog();
		break;
	case kHitLoad:
		disableTestAudio();
		if (_vm->loadGameDialog()) {
			_loadedGame = true;
			done = true;
		}
		break;
	case kHitQuit:
		disableTestAudio();
		_confirmQuit = true;
		break;
	case kHitPlay:
		disableTestAudio();
		done = true;
		break;
	case kHitMusicToggle:
		state.musicEnabled = !state.musicEnabled;
		syncSettings = true;
		refreshTestAudio = true;
		break;
	case kHitSoundToggle:
		state.soundEffectsEnabled = !state.soundEffectsEnabled;
		syncSettings = true;
		refreshTestAudio = true;
		break;
	case kHitTestToggle:
		state.optionsTestAudioEnabled = !state.optionsTestAudioEnabled;
		refreshTestAudio = true;
		break;
	case kHitMusicDown:
		if (applyVolumeHitAction(action)) {
			syncSettings = true;
			refreshTestAudio = true;
			startHeldVolume(action);
		}
		break;
	case kHitMusicUp:
		if (applyVolumeHitAction(action)) {
			syncSettings = true;
			refreshTestAudio = true;
			startHeldVolume(action);
		}
		break;
	case kHitSoundDown:
		if (applyVolumeHitAction(action)) {
			syncSettings = true;
			refreshTestAudio = true;
			startHeldVolume(action);
		}
		break;
	case kHitSoundUp:
		if (applyVolumeHitAction(action)) {
			syncSettings = true;
			refreshTestAudio = true;
			startHeldVolume(action);
		}
		break;
	case kHitVoiceDown:
		if (applyVolumeHitAction(action)) {
			syncSettings = true;
			refreshTestAudio = true;
			startHeldVolume(action);
		}
		break;
	case kHitVoiceUp:
		if (applyVolumeHitAction(action)) {
			syncSettings = true;
			refreshTestAudio = true;
			startHeldVolume(action);
		}
		break;
	case kHitTextSpeedDown:
		if (applyVolumeHitAction(action)) {
			syncSettings = true;
			startHeldVolume(action);
		}
		break;
	case kHitTextSpeedUp:
		if (applyVolumeHitAction(action)) {
			syncSettings = true;
			startHeldVolume(action);
		}
		break;
	case kHitSpeechText:
		setSpeechMode(2);
		syncSettings = true;
		refreshTestAudio = true;
		break;
	case kHitSpeechVoice:
		setSpeechMode(0);
		syncSettings = true;
		refreshTestAudio = true;
		break;
	case kHitSpeechBoth:
		setSpeechMode(1);
		syncSettings = true;
		refreshTestAudio = true;
		break;
	case kHitConfirmQuit:
		disableTestAudio();
		Engine::quitGame();
		done = true;
		break;
	case kHitCancelQuit:
		_confirmQuit = false;
		break;
	default:
		break;
	}

	if (!isVolumeHitAction(action))
		stopHeldVolume();
	if (syncSettings)
		_vm->syncSoundSettingsFromGameState();
	if (refreshTestAudio)
		updateTestAudio();
}

GameplayOptionsMenu::HitAction GameplayOptionsMenu::hitActionAt(uint16 cursorX, uint16 cursorY) const {
	if (_confirmQuit) {
		if (pointInGlobalRect(cursorX, cursorY, 0x1e3, 0x188, 0x1ff, 0x1a3))
			return kHitConfirmQuit;
		if (pointInGlobalRect(cursorX, cursorY, 0x200, 0x188, 0x21c, 0x1a3))
			return kHitCancelQuit;
		return kHitNone;
	}

	if (pointInGlobalRect(cursorX, cursorY, 0x1a3, 0x0d4, 0x1ff, 0x0ef))
		return kHitSave;
	if (pointInGlobalRect(cursorX, cursorY, 0x200, 0x0d4, 0x25c, 0x0ef))
		return kHitLoad;
	if (pointInGlobalRect(cursorX, cursorY, 0x1a3, 0x0f1, 0x1ff, 0x10c))
		return kHitQuit;
	if (pointInGlobalRect(cursorX, cursorY, 0x200, 0x0f1, 0x25c, 0x10c))
		return kHitPlay;

	if (pointInGlobalRect(cursorX, cursorY, 0x28b, 0x020, 0x312, 0x033))
		return kHitMusicToggle;
	if (pointInGlobalRect(cursorX, cursorY, 0x315, 0x020, 0x385, 0x033))
		return kHitSoundToggle;
	if (pointInGlobalRect(cursorX, cursorY, 0x388, 0x020, 0x3fa, 0x033))
		return kHitTestToggle;

	if (pointInGlobalRect(cursorX, cursorY, 0x2b4, 0x070, 0x2d0, 0x08b))
		return kHitMusicDown;
	if (pointInGlobalRect(cursorX, cursorY, 0x3af, 0x070, 0x3cb, 0x08b))
		return kHitMusicUp;
	if (pointInGlobalRect(cursorX, cursorY, 0x2b4, 0x0c3, 0x2d0, 0x0de))
		return kHitSoundDown;
	if (pointInGlobalRect(cursorX, cursorY, 0x3af, 0x0c3, 0x3cb, 0x0de))
		return kHitSoundUp;
	if (pointInGlobalRect(cursorX, cursorY, 0x2b4, 0x116, 0x2d0, 0x131))
		return kHitVoiceDown;
	if (pointInGlobalRect(cursorX, cursorY, 0x3af, 0x116, 0x3cb, 0x131))
		return kHitVoiceUp;
	if (pointInGlobalRect(cursorX, cursorY, 0x2b4, 0x169, 0x2d0, 0x184))
		return kHitTextSpeedDown;
	if (pointInGlobalRect(cursorX, cursorY, 0x3af, 0x169, 0x3cb, 0x184))
		return kHitTextSpeedUp;

	if (pointInGlobalRect(cursorX, cursorY, 0x28b, 0x1ad, 0x303, 0x1c0))
		return kHitSpeechText;
	if (pointInGlobalRect(cursorX, cursorY, 0x306, 0x1ad, 0x37e, 0x1c0))
		return kHitSpeechVoice;
	if (pointInGlobalRect(cursorX, cursorY, 0x381, 0x1ad, 0x3f9, 0x1c0))
		return kHitSpeechBoth;

	return kHitNone;
}

bool GameplayOptionsMenu::isVolumeHitAction(HitAction action) const {
	return action == kHitMusicDown || action == kHitMusicUp ||
		action == kHitSoundDown || action == kHitSoundUp ||
		action == kHitVoiceDown || action == kHitVoiceUp ||
		action == kHitTextSpeedDown || action == kHitTextSpeedUp;
}

bool GameplayOptionsMenu::applyVolumeHitAction(HitAction action) {
	GameplayState &state = _vm->gameState();
	byte *volume = nullptr;
	int delta = 0;

	switch (action) {
	case kHitMusicDown:
		if (state.musicEnabled) {
			volume = &state.musicVolumeLevel;
			delta = -kOptionsVolumeStep;
		}
		break;
	case kHitMusicUp:
		if (state.musicEnabled) {
			volume = &state.musicVolumeLevel;
			delta = kOptionsVolumeStep;
		}
		break;
	case kHitSoundDown:
		if (state.soundEffectsEnabled) {
			volume = &state.soundEffectsVolumeLevel;
			delta = -kOptionsVolumeStep;
		}
		break;
	case kHitSoundUp:
		if (state.soundEffectsEnabled) {
			volume = &state.soundEffectsVolumeLevel;
			delta = kOptionsVolumeStep;
		}
		break;
	case kHitVoiceDown:
		if (state.actorSpeechTextMode < 2) {
			volume = &state.voiceVolumeLevel;
			delta = -kOptionsVolumeStep;
		}
		break;
	case kHitVoiceUp:
		if (state.actorSpeechTextMode < 2) {
			volume = &state.voiceVolumeLevel;
			delta = kOptionsVolumeStep;
		}
		break;
	case kHitTextSpeedDown:
		if (state.actorSpeechTextMode == 2) {
			volume = &state.speechTextSpeedLevel;
			delta = -kOptionsVolumeStep;
		}
		break;
	case kHitTextSpeedUp:
		if (state.actorSpeechTextMode == 2) {
			volume = &state.speechTextSpeedLevel;
			delta = kOptionsVolumeStep;
		}
		break;
	default:
		break;
	}

	if (!volume)
		return false;

	const byte oldVolume = *volume;
	changeVolume(*volume, delta);
	return *volume != oldVolume;
}

void GameplayOptionsMenu::startHeldVolume(HitAction action) {
	_heldVolumeAction = action;
	_heldVolumeAccumulator = 0;
	_heldVolumeRepeatMillis = kOptionsVolumeInitialRepeatMillis;
}

void GameplayOptionsMenu::stopHeldVolume() {
	_heldVolumeAction = kHitNone;
	_heldVolumeAccumulator = 0;
	_heldVolumeRepeatMillis = kOptionsVolumeInitialRepeatMillis;
}

void GameplayOptionsMenu::advanceHeldVolume(uint32 delta) {
	if (_heldVolumeAction == kHitNone)
		return;

	_heldVolumeAccumulator += delta;
	if (_heldVolumeAccumulator < _heldVolumeRepeatMillis)
		return;

	while (_heldVolumeAccumulator >= _heldVolumeRepeatMillis &&
			_heldVolumeAction != kHitNone) {
		_heldVolumeAccumulator -= _heldVolumeRepeatMillis;
		_heldVolumeRepeatMillis = kOptionsVolumeRepeatMillis;
		if (!applyVolumeHitAction(_heldVolumeAction)) {
			stopHeldVolume();
			break;
		}
		_vm->syncSoundSettingsFromGameState();
		updateTestAudio();
	}
}

bool GameplayOptionsMenu::pointInGlobalRect(uint16 cursorX, uint16 cursorY, uint16 left, uint16 top,
		uint16 right, uint16 bottom) const {
	const uint globalX = cursorX + kOptionsViewportXOffset;
	return globalX > left && globalX < right && cursorY > top && cursorY < bottom;
}

void GameplayOptionsMenu::changeVolume(byte &volume, int delta) {
	const int nextValue = CLIP<int>((int)volume + delta, 0, kOptionsMaximumLevel);
	volume = (byte)nextValue;
}

void GameplayOptionsMenu::setSpeechMode(byte mode) {
	_vm->gameState().actorSpeechTextMode = mode;
}

void GameplayOptionsMenu::beginAudioSession() {
	MusicPlayer *gameplayMusic = _vm->gameplayMusic();
	_sceneMusicWasPlaying = gameplayMusic->isPlaying();
	_testMusic.setArchive(gameplayMusic->archiveName());
	if (!_soundBank0ArchiveName.empty())
		_testSound.setArchive(_soundBank0ArchiveName);

	if (_sceneMusicWasPlaying)
		gameplayMusic->stop();

	updateTestAudio();
}

void GameplayOptionsMenu::endAudioSession() {
	disableTestAudio();

	if (_sceneMusicWasPlaying && !_loadedGame && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested())
		_vm->gameplayMusic()->resumeLastCue();
	_sceneMusicWasPlaying = false;
}

void GameplayOptionsMenu::disableTestAudio() {
	_vm->gameState().optionsTestAudioEnabled = false;
	stopTestAudio();
}

void GameplayOptionsMenu::updateTestAudio() {
	GameplayState &state = _vm->gameState();
	if (!state.optionsTestAudioEnabled) {
		stopTestAudio();
		return;
	}

	if (state.musicEnabled) {
		if (!_testMusic.isPlaying())
			_testMusic.playMusicCue(testMusicCueId(), 100, true);
		_testMusic.setVolume(100);
	} else {
		_testMusic.stop();
	}

	if (state.soundEffectsEnabled && !_soundBank0ArchiveName.empty()) {
		if (!_testSound.isPlaying())
			_testSound.playSample(testSoundCueId(), 100, true);
		_testSound.setVolume(100);
	} else {
		_testSound.stop();
	}

	if (state.actorSpeechTextMode < 2 && _hasSpeechPreviewSample) {
		if (!_testSpeech.isPlaying())
			_testSpeech.playSample(_speechPreviewSampleId, 100, true);
		_testSpeech.setVolume(100);
	} else {
		_testSpeech.stop();
	}
}

void GameplayOptionsMenu::stopTestAudio() {
	_testMusic.stop();
	_testSound.stop();
	_testSpeech.stop();
}

uint16 GameplayOptionsMenu::testMusicCueId() const {
	const uint16 chapter = _vm->gameState().mainFlowStateId / 1000;
	if (chapter == 9)
		return 0x0c;
	if (chapter == 7)
		return 0x0e;
	return 0x0f;
}

uint16 GameplayOptionsMenu::testSoundCueId() const {
	return _vm->gameState().mainFlowStateId / 1000 == 9 ? 0x0f : 2;
}

void GameplayOptionsMenu::captureGameplayPreview() {
	if (_palette.size() < kPaletteSize)
		return;

	Graphics::Surface *lockedScreen = g_system->lockScreen();
	if (!lockedScreen)
		return;

	if (lockedScreen->format.bytesPerPixel != 1 || lockedScreen->w <= 0 || lockedScreen->h <= 0) {
		g_system->unlockScreen();
		return;
	}

	_gameplayPreview.create(kOptionsGameplayPreviewWidth, kOptionsGameplayPreviewHeight,
		Graphics::PixelFormat::createFormatCLUT8());

	for (int dstY = 0; dstY < kOptionsGameplayPreviewHeight; ++dstY) {
		int srcY0 = (dstY * lockedScreen->h) / kOptionsGameplayPreviewHeight;
		int srcY1 = ((dstY + 1) * lockedScreen->h) / kOptionsGameplayPreviewHeight;
		if (srcY1 <= srcY0)
			srcY1 = srcY0 + 1;
		srcY1 = MIN<int>(srcY1, lockedScreen->h);

		byte *dst = (byte *)_gameplayPreview.getBasePtr(0, dstY);
		for (int dstX = 0; dstX < kOptionsGameplayPreviewWidth; ++dstX) {
			int srcX0 = (dstX * lockedScreen->w) / kOptionsGameplayPreviewWidth;
			int srcX1 = ((dstX + 1) * lockedScreen->w) / kOptionsGameplayPreviewWidth;
			if (srcX1 <= srcX0)
				srcX1 = srcX0 + 1;
			srcX1 = MIN<int>(srcX1, lockedScreen->w);

			uint red = 0;
			uint green = 0;
			uint blue = 0;
			uint sampleCount = 0;
			for (int srcY = srcY0; srcY < srcY1; ++srcY) {
				const byte *src = (const byte *)lockedScreen->getBasePtr(srcX0, srcY);
				for (int srcX = srcX0; srcX < srcX1; ++srcX) {
					const uint paletteOffset = (uint)*src++ * 3;
					red += _palette[paletteOffset];
					green += _palette[paletteOffset + 1];
					blue += _palette[paletteOffset + 2];
					++sampleCount;
				}
			}

			if (sampleCount == 0) {
				dst[dstX] = 0;
			} else {
				dst[dstX] = nearestPreviewPaletteColor(red / sampleCount,
					green / sampleCount, blue / sampleCount);
			}
		}
	}

	g_system->unlockScreen();
}

byte GameplayOptionsMenu::nearestPreviewPaletteColor(uint red, uint green, uint blue) const {
	uint bestDistance = 0xffffffff;
	byte bestColor = 0;
	for (uint color = 0; color < 256; ++color) {
		const uint paletteOffset = color * 3;
		const int deltaRed = (int)red - _palette[paletteOffset];
		const int deltaGreen = (int)green - _palette[paletteOffset + 1];
		const int deltaBlue = (int)blue - _palette[paletteOffset + 2];
		const uint distance = (uint)(deltaRed * deltaRed + deltaGreen * deltaGreen +
			deltaBlue * deltaBlue);
		if (distance < bestDistance) {
			bestDistance = distance;
			bestColor = (byte)color;
			if (distance == 0)
				break;
		}
	}
	return bestColor;
}

void GameplayOptionsMenu::composeScreen() {
	if (_menuFramebuffer.size() < kOptionsFramebufferSize) {
		_screen.fillRect(_screen.getBounds(), 0);
		return;
	}

	_screen.copyRectToSurface(_menuFramebuffer.surface(), 0, 0,
		Common::Rect(kOptionsViewportXOffset, 0,
			kOptionsViewportXOffset + HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight));
}

void GameplayOptionsMenu::drawGameplayPreview() {
	if (!_gameplayPreview.getPixels())
		return;

	_screen.copyRectToSurface(_gameplayPreview.rawSurface(), kOptionsGameplayPreviewX,
		kOptionsGameplayPreviewY, Common::Rect(0, 0, _gameplayPreview.w, _gameplayPreview.h));
}

void GameplayOptionsMenu::drawControls(Graphics::Surface &surface) {
	GameplayState &state = _vm->gameState();

	drawText(surface, "Salvar", 0x1d1, 0xd7, kOptionsTextActiveColor, true);
	drawText(surface, "Cargar", 0x22e, 0xd7, kOptionsTextActiveColor, true);
	drawText(surface, "Salir", 0x1d1, 0xf4, kOptionsTextActiveColor, true);
	drawText(surface, "Jugar", 0x22e, 0xf4, kOptionsTextActiveColor, true);

	drawText(surface, state.musicEnabled ? "Musica ON" : "Musica OFF", 0x2a3, 0x20,
		state.musicEnabled ? kOptionsTextActiveColor : kOptionsTextDisabledColor, false);
	drawText(surface, state.soundEffectsEnabled ? "SFX ON" : "SFX OFF", 0x32d, 0x20,
		state.soundEffectsEnabled ? kOptionsTextActiveColor : kOptionsTextDisabledColor, false);
	drawText(surface, state.optionsTestAudioEnabled ? "Test ON" : "Test OFF", 0x3a0, 0x20,
		state.optionsTestAudioEnabled ? kOptionsTextTestColor : kOptionsTextDisabledColor, false);
	drawStatusDots();

	drawText(surface, "Volumen musica", 0x33f, 0x55,
		state.musicEnabled ? kOptionsTextActiveColor : kOptionsTextDisabledColor, true);
	drawToggleSquare(0, 0, state.musicEnabled && state.musicVolumeLevel != 0 ?
		kOptionsTextActiveColor : kOptionsTextDisabledColor);
	drawValueBar(0, state.musicVolumeLevel, state.musicEnabled ? kOptionsTextActiveColor : kOptionsBarDisabledColor);
	drawToggleSquare(1, 0, state.musicEnabled && state.musicVolumeLevel < kOptionsMaximumLevel ?
		kOptionsTextActiveColor : kOptionsTextDisabledColor);

	drawText(surface, "Volumen SFX", 0x33f, 0xa8,
		state.soundEffectsEnabled ? kOptionsTextActiveColor : kOptionsTextDisabledColor, true);
	drawToggleSquare(0, 1, state.soundEffectsEnabled && state.soundEffectsVolumeLevel != 0 ?
		kOptionsTextActiveColor : kOptionsTextDisabledColor);
	drawValueBar(1, state.soundEffectsVolumeLevel,
		state.soundEffectsEnabled ? kOptionsTextActiveColor : kOptionsBarDisabledColor);
	drawToggleSquare(1, 1, state.soundEffectsEnabled && state.soundEffectsVolumeLevel < kOptionsMaximumLevel ?
		kOptionsTextActiveColor : kOptionsTextDisabledColor);

	drawText(surface, "Volumen voz", 0x33f, 0xfb,
		state.actorSpeechTextMode < 2 ? kOptionsTextActiveColor : kOptionsTextDisabledColor, true);
	drawToggleSquare(0, 2, state.actorSpeechTextMode < 2 && state.voiceVolumeLevel != 0 ?
		kOptionsTextActiveColor : kOptionsTextDisabledColor);
	drawValueBar(2, state.voiceVolumeLevel,
		state.actorSpeechTextMode < 2 ? kOptionsTextActiveColor : kOptionsBarDisabledColor);
	drawToggleSquare(1, 2, state.actorSpeechTextMode < 2 && state.voiceVolumeLevel < kOptionsMaximumLevel ?
		kOptionsTextActiveColor : kOptionsTextDisabledColor);

	drawText(surface, "Velocidad texto", 0x33f, 0x14e,
		state.actorSpeechTextMode == 2 ? kOptionsTextActiveColor : kOptionsTextDisabledColor, true);
	drawToggleSquare(0, 3, state.actorSpeechTextMode == 2 && state.speechTextSpeedLevel != 0 ?
		kOptionsTextActiveColor : kOptionsTextDisabledColor);
	drawValueBar(3, state.speechTextSpeedLevel,
		state.actorSpeechTextMode == 2 ? kOptionsTextActiveColor : kOptionsBarDisabledColor);
	drawToggleSquare(1, 3, state.actorSpeechTextMode == 2 && state.speechTextSpeedLevel < kOptionsMaximumLevel ?
		kOptionsTextActiveColor : kOptionsTextDisabledColor);

	drawText(surface, "Texto", 0x2a3, 0x1ad,
		state.actorSpeechTextMode == 2 ? kOptionsTextActiveColor : kOptionsTextDisabledColor, false);
	drawText(surface, "Voz", 0x31e, 0x1ad,
		state.actorSpeechTextMode == 0 ? kOptionsTextActiveColor : kOptionsTextDisabledColor, false);
	drawText(surface, "Ambos", 0x399, 0x1ad,
		state.actorSpeechTextMode == 1 ? kOptionsTextActiveColor : kOptionsTextDisabledColor, false);
}

void GameplayOptionsMenu::drawQuitConfirmation(Graphics::Surface &surface) {
	drawText(surface, kOptionsQuitPromptText, 0x1ff, 0x168, kOptionsTextActiveColor, true);
	drawQuitConfirmationBacking();
	drawText(surface, kOptionsQuitYesText, 0x1f1, 0x18c, kOptionsTextActiveColor, true);
	drawText(surface, kOptionsQuitNoText, 0x20e, 0x18c, kOptionsTextActiveColor, true);
}

void GameplayOptionsMenu::drawText(Graphics::Surface &surface, const Common::String &text, int globalX, int y,
		byte color, bool centered) {
	HollywoodFont *font = _vm->font();
	if (!font || !font->isLoaded() || text.empty())
		return;

	font->setShadowColor(0);
	int x = globalX - (int)kOptionsViewportXOffset;
	const int textWidth = font->getStringWidth(text) + 2;
	if (centered)
		x -= textWidth / 2;
	font->drawString(&surface, text, x, y, textWidth, color, Graphics::kTextAlignLeft, 0,
		false, true);
}

void GameplayOptionsMenu::drawQuitConfirmationBacking() {
	drawQuitButtonFrame(kOptionsQuitYesLeft, kOptionsQuitYesRight);
	drawQuitButtonFrame(kOptionsQuitNoLeft, kOptionsQuitNoRight);
}

void GameplayOptionsMenu::drawQuitButtonFrame(int globalLeft, int globalRight) {
	const int left = globalLeft - kOptionsViewportXOffset;
	const int top = kOptionsQuitChoiceTop;
	const int width = globalRight - globalLeft;
	const int height = kOptionsQuitChoiceBottom - kOptionsQuitChoiceTop;
	const int right = left + width - 1;
	const int bottom = top + height - 1;

	for (int y = top; y <= bottom; ++y) {
		byte *pixels = (byte *)_screen.getBasePtr(left, y);
		for (int x = 0; x < width; ++x) {
			if (y == top || x == 0)
				pixels[x] = kOptionsTextDisabledColor;
			else if (y == bottom || left + x == right)
				pixels[x] = kOptionsStatusDotOffColor;
		}
	}
}

void GameplayOptionsMenu::shiftGlobalRectColors(int globalLeft, int top, int globalRight, int bottom, int delta) {
	const int left = globalLeft - kOptionsViewportXOffset;
	const int right = globalRight - kOptionsViewportXOffset;
	for (int y = top; y < bottom; ++y) {
		byte *pixels = (byte *)_screen.getBasePtr(left, y);
		for (int x = 0; x < right - left; ++x) {
			if (pixels[x] != 0)
				pixels[x] = (byte)CLIP<int>((int)pixels[x] + delta, 0, 255);
		}
	}
}

void GameplayOptionsMenu::drawStatusDot(int globalX, int y, byte color) {
	const int left = globalX - kOptionsViewportXOffset;
	uint changed = 0;
	for (int row = 0; row < kOptionsStatusDotSize; ++row) {
		byte *pixels = (byte *)_screen.getBasePtr(left, y + row);
		for (int x = 0; x < kOptionsStatusDotSize; ++x) {
			if (pixels[x] == kOptionsStatusDotOffColor ||
					pixels[x] == kOptionsTextActiveColor ||
					pixels[x] == kOptionsStatusDotTestColor ||
					pixels[x] == kOptionsTextTestColor) {
				pixels[x] = color;
				++changed;
			}
		}
	}

	if (changed != 0)
		return;

	const int center = kOptionsStatusDotSize / 2;
	const int radiusSquared = center * center;
	for (int row = 0; row < kOptionsStatusDotSize; ++row) {
		byte *pixels = (byte *)_screen.getBasePtr(left, y + row);
		for (int x = 0; x < kOptionsStatusDotSize; ++x) {
			const int dx = x - center;
			const int dy = row - center;
			if (dx * dx + dy * dy <= radiusSquared)
				pixels[x] = color;
		}
	}
}

void GameplayOptionsMenu::drawStatusDots() {
	const GameplayState &state = _vm->gameState();
	drawStatusDot(kOptionsMusicStatusDotX, kOptionsTopStatusDotY,
		state.musicEnabled ? kOptionsTextActiveColor : kOptionsStatusDotOffColor);
	drawStatusDot(kOptionsSoundStatusDotX, kOptionsTopStatusDotY,
		state.soundEffectsEnabled ? kOptionsTextActiveColor : kOptionsStatusDotOffColor);
	drawStatusDot(kOptionsTestStatusDotX, kOptionsTopStatusDotY,
		state.optionsTestAudioEnabled ? kOptionsStatusDotTestColor : kOptionsStatusDotOffColor);
	drawStatusDot(kOptionsSpeechTextStatusDotX, kOptionsSpeechStatusDotY,
		state.actorSpeechTextMode == 2 ? kOptionsTextActiveColor : kOptionsStatusDotOffColor);
	drawStatusDot(kOptionsSpeechVoiceStatusDotX, kOptionsSpeechStatusDotY,
		state.actorSpeechTextMode == 0 ? kOptionsTextActiveColor : kOptionsStatusDotOffColor);
	drawStatusDot(kOptionsSpeechBothStatusDotX, kOptionsSpeechStatusDotY,
		state.actorSpeechTextMode == 1 ? kOptionsTextActiveColor : kOptionsStatusDotOffColor);
}

void GameplayOptionsMenu::drawValueBar(int rowIndex, byte value, byte color) {
	if (rowIndex < 0 || rowIndex >= 4)
		return;

	const int top = kOptionsValueBarY[rowIndex];
	const int filledWidth = CLIP<int>(value, 0, kOptionsValueBarWidth);
	for (int row = 0; row < kOptionsValueBarHeight; ++row) {
		byte *pixels = (byte *)_screen.getBasePtr(kOptionsValueBarX, top + row);
		for (int x = 0; x < kOptionsValueBarWidth; ++x)
			pixels[x] = x < filledWidth ? color : kOptionsBarEmptyColor;
	}
}

void GameplayOptionsMenu::drawToggleSquare(int columnIndex, int rowIndex, byte color) {
	if (columnIndex < 0 || columnIndex >= 2 || rowIndex < 0 || rowIndex >= 4)
		return;

	const int left = kOptionsToggleSquareX[columnIndex];
	const int top = kOptionsToggleSquareY[rowIndex];
	for (int row = 0; row < kOptionsToggleSquareSize; ++row) {
		byte *pixels = (byte *)_screen.getBasePtr(left, top + row);
		for (int column = 0; column < kOptionsToggleSquareSize; ++column) {
			if (pixels[column] == kOptionsTextActiveColor ||
					pixels[column] == kOptionsTextDisabledColor ||
					pixels[column] == kOptionsBarDisabledColor)
				pixels[column] = color;
		}
	}
}

void GameplayOptionsMenu::present() {
	composeScreen();
	drawGameplayPreview();

	Graphics::Surface *screenSurface = _screen.surfacePtr();
	drawControls(*screenSurface);
	if (_confirmQuit) {
		shiftGlobalRectColors(kOptionsQuitMainButtonLeft, kOptionsQuitMainButtonTop,
			kOptionsQuitMainButtonRight, kOptionsQuitMainButtonBottom, kOptionsQuitButtonShift);
		drawQuitConfirmation(*screenSurface);
	}

	_displayPalette.uploadFrom6Bit(_palette);
	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, _screen.w, _screen.h);
	g_system->updateScreen();
}

} // End of namespace Hollywood
