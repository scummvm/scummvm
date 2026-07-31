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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/ripper.h"

#include "common/debug.h"
#include "common/serializer.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/scene_audio.h"
#include "ripper/script.h"

namespace Ripper {

namespace {

static const char kRipperSaveMagic[] = { 'R', 'S', 'A', 'V' };
static const uint32 kRipperSaveVersion = 4;
static const uint32 kRipperMinimumSaveVersion = 2;
static const uint16 kScreenWidth = 640;
static const uint16 kScreenHeight = 400;
static const uint32 kPaletteSize = 256 * 3;

} // End of anonymous namespace

bool RipperEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _gameplayStarted && _scripts->canSaveGame();
}

bool RipperEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return _scripts != nullptr;
}

Common::Error RipperEngine::saveGameState(int slot, const Common::String &desc,
		bool isAutosave) {
	if (slot < 0 || slot > getAutosaveSlot() || (!isAutosave && slot == getAutosaveSlot()))
		return Common::kWritingFailed;
	const Common::String saveDescription = desc.substr(0, 60);
	debugC(1, kDebugSaveLoad,
		"Ripper: writing %s save slot=%d description='%s'",
		isAutosave ? "emergency" : "manual", slot, saveDescription.c_str());
	const Common::Error result = Engine::saveGameState(slot, saveDescription, isAutosave);
	debugC(result.getCode() == Common::kNoError ? 1 : 2, kDebugSaveLoad,
		"Ripper: %s save slot=%d result=%d",
		isAutosave ? "emergency" : "manual", slot, result.getCode());
	return result;
}

Common::Error RipperEngine::loadGameState(int slot) {
	if (slot < 0 || slot > getAutosaveSlot())
		return Common::kReadingFailed;
	debugC(1, kDebugSaveLoad, "Ripper: restoring save slot=%d", slot);
	const Common::Error result = Engine::loadGameState(slot);
	debugC(result.getCode() == Common::kNoError ? 1 : 2, kDebugSaveLoad,
		"Ripper: restore slot=%d result=%d", slot, result.getCode());
	return result;
}

Common::Error RipperEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (!canSaveGameStateCurrently())
		return Common::kWritingFailed;

	Common::Serializer serializer(nullptr, stream);
	if (!serializer.matchBytes(kRipperSaveMagic, sizeof(kRipperSaveMagic)) ||
			!serializer.syncVersion(kRipperSaveVersion) ||
			!_scripts->syncGame(serializer) || !_sceneAudio->syncGame(serializer))
		return Common::kWritingFailed;

	uint16 width = kScreenWidth;
	uint16 height = kScreenHeight;
	serializer.syncAsUint16LE(width);
	serializer.syncAsUint16LE(height);
	Common::Array<byte> pixels;
	pixels.resize(kScreenWidth * kScreenHeight);
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != kScreenWidth ||
			screen->h != kScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return Common::kWritingFailed;
	}
	for (uint y = 0; y < kScreenHeight; ++y)
		memcpy(pixels.data() + y * kScreenWidth, screen->getBasePtr(0, y), kScreenWidth);
	g_system->unlockScreen();
	serializer.syncBytes(pixels.data(), pixels.size());

	Common::Array<byte> palette;
	palette.resize(kPaletteSize);
	g_system->getPaletteManager()->grabPalette(palette.data(), 0, 256);
	serializer.syncBytes(palette.data(), palette.size());
	if (serializer.err())
		return Common::kWritingFailed;

	debugC(1, kDebugSaveLoad,
		"Ripper: captured save state autosave=%d indexedDisplay=%ux%u paletteEntries=256",
		isAutosave, width, height);
	return Common::kNoError;
}

Common::Error RipperEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer serializer(stream, nullptr);
	if (!serializer.matchBytes(kRipperSaveMagic, sizeof(kRipperSaveMagic)) ||
			!serializer.syncVersion(kRipperSaveVersion) ||
			serializer.getVersion() < kRipperMinimumSaveVersion) {
		warning("Ripper: unsupported or invalid save-state header");
		return Common::kReadingFailed;
	}
	if (!_scripts->syncGame(serializer) || !_sceneAudio->syncGame(serializer))
		return Common::kReadingFailed;

	uint16 width = 0;
	uint16 height = 0;
	serializer.syncAsUint16LE(width);
	serializer.syncAsUint16LE(height);
	if (width != kScreenWidth || height != kScreenHeight)
		return Common::kReadingFailed;

	Common::Array<byte> pixels;
	pixels.resize(kScreenWidth * kScreenHeight);
	serializer.syncBytes(pixels.data(), pixels.size());
	Common::Array<byte> palette;
	palette.resize(kPaletteSize);
	serializer.syncBytes(palette.data(), palette.size());
	if (serializer.err())
		return Common::kReadingFailed;

	g_system->copyRectToScreen(pixels.data(), kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	g_system->getPaletteManager()->setPalette(palette.data(), 0, 256);
	g_system->updateScreen();
	_gameplayStarted = true;
	_input->discardMouseTransitions();
	debugC(1, kDebugSaveLoad,
		"Ripper: applied loaded save state indexedDisplay=%ux%u paletteEntries=256",
		width, height);
	return Common::kNoError;
}

} // End of namespace Ripper
