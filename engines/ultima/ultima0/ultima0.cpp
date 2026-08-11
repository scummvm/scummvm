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

#include "common/system.h"
#include "common/savefile.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "ultima/ultima0/ultima0.h"
#include "ultima/ultima0/console.h"

namespace Ultima {
namespace Ultima0 {

static const byte PALETTE[][3] = {
	{ 0, 0, 0 },		// Black
	{ 0, 0, 255 },		// Blue
	{ 0, 255, 0 },		// Green,
	{ 0, 255, 255 },	// Cyan
	{ 255, 0, 0 },		// Red
	{ 255, 0, 255 },	// Purple
	{ 255, 255, 0 },	// Yellow
	{ 255, 255, 255 },	// White
	{ 255, 0, 128 },	// Rose
	{ 80, 80, 255 },	// Violet
	{ 180, 180, 180 },	// Grey
	{ 255, 80, 80 }		// Orange
};


Ultima0Engine *g_engine;

Ultima0Engine::Ultima0Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), /*_gameDescription(gameDesc), */_randomSource("Ultima0"),
		_palette(&PALETTE[0][0], sizeof(PALETTE) / 3) {
	g_engine = this;
}

Ultima0Engine::~Ultima0Engine() {
	stopMidi();
}

Common::Error Ultima0Engine::run() {
	// Initialize the graphics
	initGraphics(DEFAULT_SCX, DEFAULT_SCY);
	g_system->getPaletteManager()->setPalette(_palette);

	// Set the debugger console
	setDebugger(new Console());
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);

	// Play the game
	runGame();

	return Common::kNoError;
}

bool Ultima0Engine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);

}

bool Ultima0Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	auto *view = focusedView();
	if (!view)
		return false;

	Common::String name = focusedView()->getName();
	return name == "WorldMap" || name == "Dungeon";
}

Common::Error Ultima0Engine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	syncSavegame(s);

	stopMidi();
	replaceView(_player._level == 0 ? "WorldMap" : "Dungeon");

	return Common::kNoError;
}

Common::Error Ultima0Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	syncSavegame(s);
	return Common::kNoError;
}

void Ultima0Engine::syncSavegame(Common::Serializer &s) {
	_player.synchronize(s);
	_worldMap.synchronize(s);
	_dungeon.synchronize(s);
}

bool Ultima0Engine::savegamesExist() const {
	Common::String slotName = getSaveStateName(1);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != nullptr;

	delete saveFile;
	return result;
}

void Ultima0Engine::playMidi(const char *name) {
	stopMidi();

	_music = new MusicPlayer(name);
	_music->playSMF(true);
}

void Ultima0Engine::stopMidi() {
	if (_music) {
		_music->stop();
		delete _music;
		_music = nullptr;
	}
}

} // namespace Ultima0
} // namespace Ultima
