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
const uint kOptionsResource000HeaderByteCount = 1;
const uint kOptionsResource000OffsetTableSize = 400;
const uint kOptionsResource000SizeTableSize = 400;
const uint kOptionsFramebufferEntry = 0x2a;
const uint kOptionsFramebufferSize = 0x78000;
const uint kOptionsFramebufferWidth = 1024;
const uint kOptionsViewportXOffset = 0x180;
const uint kOptionsObjectPaletteOffset = 0x210;
const uint kOptionsObjectPaletteSize = 0xf0;
const uint kOptionsObjectPalettePanelSize = 0x60;
const byte kOptionsTextActiveColor = 0xf1;
const byte kOptionsTextDisabledColor = 0xf0;
const byte kOptionsTextTestColor = 0xfa;
const byte kOptionsBarDisabledColor = 0xee;
const byte kOptionsBarEmptyColor = 0xe9;
const uint32 kOptionsTickMillis = 10;
const byte kOptionsVolumeStep = 10;
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
		_loaded(false),
		_confirmQuit(false) {
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
}

bool GameplayOptionsMenu::run(const Common::Array<byte> &basePalette) {
	if (!_vm || !load())
		return false;

	preparePalette(basePalette);
	_vm->syncSoundSettings();
	_confirmQuit = false;
	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());

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

		present();
		g_system->delayMillis(kOptionsTickMillis);
	}

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
	if (!exe)
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> stream(exe->getResource(
		Common::WinResourceID(kOptionsPaletteResourceType), Common::WinResourceID(kOptionsPaletteResourceName)));
	if (!stream || stream->size() < kOptionsObjectPaletteSize)
		return false;

	_objectPaletteTriples.resize(kOptionsObjectPaletteSize);
	return stream->read(_objectPaletteTriples.data(), _objectPaletteTriples.size()) ==
		_objectPaletteTriples.size();
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
		case Common::EVENT_RBUTTONDOWN:
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
		if (_confirmQuit)
			_confirmQuit = false;
		else
			done = true;
		break;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
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
	switch (action) {
	case kHitSave:
		_vm->saveGameDialog();
		break;
	case kHitLoad:
		if (_vm->loadGameDialog())
			done = true;
		break;
	case kHitQuit:
		_confirmQuit = true;
		break;
	case kHitPlay:
		done = true;
		break;
	case kHitMusicToggle:
		state.musicEnabled = !state.musicEnabled;
		break;
	case kHitSoundToggle:
		state.soundEffectsEnabled = !state.soundEffectsEnabled;
		break;
	case kHitTestToggle:
		state.optionsTestAudioEnabled = !state.optionsTestAudioEnabled;
		break;
	case kHitMusicDown:
		if (state.musicEnabled)
			changeVolume(state.musicVolumeLevel, -kOptionsVolumeStep);
		break;
	case kHitMusicUp:
		if (state.musicEnabled)
			changeVolume(state.musicVolumeLevel, kOptionsVolumeStep);
		break;
	case kHitSoundDown:
		if (state.soundEffectsEnabled)
			changeVolume(state.soundEffectsVolumeLevel, -kOptionsVolumeStep);
		break;
	case kHitSoundUp:
		if (state.soundEffectsEnabled)
			changeVolume(state.soundEffectsVolumeLevel, kOptionsVolumeStep);
		break;
	case kHitVoiceDown:
		if (state.actorSpeechTextMode < 2)
			changeVolume(state.voiceVolumeLevel, -kOptionsVolumeStep);
		break;
	case kHitVoiceUp:
		if (state.actorSpeechTextMode < 2)
			changeVolume(state.voiceVolumeLevel, kOptionsVolumeStep);
		break;
	case kHitTextSpeedDown:
		if (state.actorSpeechTextMode == 2)
			changeVolume(state.speechTextSpeedLevel, -kOptionsVolumeStep);
		break;
	case kHitTextSpeedUp:
		if (state.actorSpeechTextMode == 2)
			changeVolume(state.speechTextSpeedLevel, kOptionsVolumeStep);
		break;
	case kHitSpeechText:
		setSpeechMode(2);
		break;
	case kHitSpeechVoice:
		setSpeechMode(0);
		break;
	case kHitSpeechBoth:
		setSpeechMode(1);
		break;
	case kHitConfirmQuit:
		Engine::quitGame();
		done = true;
		break;
	case kHitCancelQuit:
		_confirmQuit = false;
		break;
	default:
		break;
	}

	_vm->syncSoundSettingsFromGameState();
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

void GameplayOptionsMenu::composeScreen() {
	if (_menuFramebuffer.size() < kOptionsFramebufferSize) {
		memset(_screen.data(), 0, _screen.size());
		return;
	}

	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint sourceOffset = y * kOptionsFramebufferWidth + kOptionsViewportXOffset;
		memcpy(_screen.data() + y * HollywoodEngine::kScreenWidth,
			_menuFramebuffer.data() + sourceOffset, HollywoodEngine::kScreenWidth);
	}
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
	drawText(surface, "Salir del juego?", 0x1ff, 0x168, kOptionsTextActiveColor, true);
	drawText(surface, "Si", 0x1f1, 0x18c, kOptionsTextActiveColor, true);
	drawText(surface, "No", 0x20e, 0x18c, kOptionsTextActiveColor, true);
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

void GameplayOptionsMenu::drawValueBar(int rowIndex, byte value, byte color) {
	if (rowIndex < 0 || rowIndex >= 4)
		return;

	const int top = kOptionsValueBarY[rowIndex];
	const int filledWidth = CLIP<int>(value, 0, kOptionsValueBarWidth);
	for (int row = 0; row < kOptionsValueBarHeight; ++row) {
		byte *pixels = _screen.data() + (top + row) * HollywoodEngine::kScreenWidth + kOptionsValueBarX;
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
		byte *pixels = _screen.data() + (top + row) * HollywoodEngine::kScreenWidth + left;
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

	Graphics::Surface screenSurface;
	screenSurface.init(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
		HollywoodEngine::kScreenWidth, _screen.data(), Graphics::PixelFormat::createFormatCLUT8());
	drawControls(screenSurface);
	if (_confirmQuit)
		drawQuitConfirmation(screenSurface);

	uploadPalette6Bit(_palette);
	g_system->copyRectToScreen(_screen.data(), HollywoodEngine::kScreenWidth, 0, 0,
		HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight);
	g_system->updateScreen();
}

} // End of namespace Hollywood
