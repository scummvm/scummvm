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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "common/savefile.h"
#include "engines/util.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "m4/m4.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/conv_io.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/gui/hotkeys.h"
#include "m4/platform/sound/digi.h"
#include "m4/platform/sound/midi.h"
#include "m4/detection.h"
#include "m4/console.h"
#include "m4/metaengine.h"
#include "m4/core/param.h"

namespace M4 {

#define SAVEGAME_VERSION 1

M4Engine *g_engine;

M4Engine::M4Engine(OSystem *syst, const M4GameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("M4") {
	g_engine = this;

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "goodstuf");
	SearchMan.addSubDirectoryMatching(gameDataDir, "resource");
}

M4Engine::~M4Engine() {
}

uint32 M4Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String M4Engine::getGameId() const {
	return _gameDescription->desc.gameId;
}

int M4Engine::getGameType() const {
	return _gameDescription->gameType;
}

Common::Language M4Engine::getLanguage() const {
	return _gameDescription->desc.language;
}

int M4Engine::isDemo() const {
	if ((getFeatures() & ADGF_DEMO) == 0)
		return GStyle_Game;
	else if (_gameDescription->features & kFeaturesNonInteractiveDemo)
		return GStyle_NonInteractiveDemo;
	else
		return GStyle_Demo;
}

Common::Error M4Engine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(640, 480);
	syncSoundSettings();

	// Instantiate globals and setup
	Vars *vars = createVars();

	if (vars->init()) {
		// Set the console
		setupConsole();

		// Check for launcher savegame to load
		_useOriginalSaveLoad = ConfMan.getBool("original_menus");
		if (ConfMan.hasKey("save_slot")) {
			_G(kernel).restore_slot = ConfMan.getInt("save_slot");
			_G(game).previous_room = KERNEL_RESTORING_GAME;
		}

		// Run game here
		m4_inflight();
	}

	delete vars;
	return Common::kNoError;
}

#define KEEP_PLAYING (_G(kernel).going && !shouldQuit())

void M4Engine::m4_inflight() {
	g_vars->getHotkeys()->add_hot_keys();
	_G(kernel).going = true;

	while (KEEP_PLAYING) {
		if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
			midi_stop();
			kernel_load_game(_G(kernel).restore_slot);
		}

		// Start up next section
		_G(between_rooms) = true;
		global_section_constructor();		// Sets the active section
		_G(kernel).going = kernel_section_startup();
		section_init();

		while (_G(game).new_section == _G(game).section_id && KEEP_PLAYING) {
			m4SceneLoad();
			m4RunScene();
			m4EndScene();
		}
	}
}

void M4Engine::syncSoundSettings() {
	Engine::syncSoundSettings();

	int volume = ConfMan.getBool("sfx_mute") ? 0 : CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, volume);
}

bool M4Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return g_vars && INTERFACE_VISIBLE && player_commands_allowed();
}

bool M4Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	return g_vars && INTERFACE_VISIBLE && player_commands_allowed();
}

void M4Engine::showSaveScreen() {
	saveGameDialog();
}

void M4Engine::showLoadScreen(LoadDialogSource source) {
	loadGameDialog();
}

Common::Error M4Engine::loadGameState(int slot) {
	// Don't load savegame immediately, just set the slot for the engine's
	// kernel to take care of in the outer game loop
	_G(kernel).restore_slot = slot;
	_G(game).new_room = KERNEL_RESTORING_GAME;
	_G(game).new_section = KERNEL_RESTORING_GAME;
	_G(game).previous_room = KERNEL_RESTORING_GAME;

	return Common::kNoError;
}

Common::Error M4Engine::loadGameStateDoIt(int slot) {
	Common::InSaveFile *save = getOriginalSave(slot);

	if (save) {
		// Skip original description
		int descSize = save->readUint32LE();
		save->seek(descSize + 45, SEEK_CUR);

		int thumbSize = save->readUint32LE();
		save->seek(thumbSize, SEEK_CUR);

		// We're now at data section, handle it
		Common::Serializer s(save, nullptr);
		s.setVersion(1);
		Common::Error result = syncGame(s);

		delete save;

		return result;

	} else {
		return Engine::loadGameState(slot);
	}
}

Common::InSaveFile *M4Engine::getOriginalSave(int slot) const {
	Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(
		getSaveStateName(slot));
	char name[16];

	if (save) {
		if (save->seek(-44, SEEK_END) && save->read(name, 7) == 7 &&
				!strncmp(name, "MIRROR", 7)) {
			save->seek(0);
			return save;
		} else if (save->seek(-44, SEEK_END) && save->read(name, 7) == 7 &&
				!strncmp(name, "FAUCET ", 7)) {
			save->seek(0);
			return save;
		}

		delete save;
	}

	return nullptr;
}

Common::Error M4Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	stream->writeByte(SAVEGAME_VERSION);

	Common::Serializer s(nullptr, stream);
	return syncGame(s);
}

Common::Error M4Engine::loadGameStream(Common::SeekableReadStream *stream) {
	byte version = stream->readByte();
	if (version > SAVEGAME_VERSION)
		error("Tried to load unsupported savegame version");

	Common::Serializer s(stream, nullptr);
	s.setVersion(version);
	return syncGame(s);
}

Common::Error M4Engine::syncGame(Common::Serializer &s) {
	if (s.isSaving())
		_G(kernel).pause = false;

	// To match the original, we sync a set of fields from kernel and game
	if (getGameType() == GType_Riddle)
		s.syncAsByte(_G(kernel).going);

	s.syncAsByte(_G(kernel).pause);
	for (int i = 0; i < KERNEL_SCRATCH_SIZE; ++i)
		s.syncAsUint32LE(_G(kernel).scratch[i]);

	s.syncAsByte(_G(kernel).unused);
	s.syncAsSint16LE(_G(kernel).last_save);
	s.syncAsSint16LE(_G(game).room_id);
	s.syncAsSint16LE(_G(game).new_room);
	s.syncAsSint16LE(_G(game).previous_section);
	s.syncAsSint16LE(_G(game).section_id);
	s.syncAsSint16LE(_G(game).new_section);
	s.syncAsSint16LE(_G(game).previous_room);

	if (getGameType() == GType_Riddle) {
		// TODO: Determinate if the extra fields need to be synced
		s.skip(22);

	} else {
		s.syncAsByte(_G(kernel).restore_game);
		s.syncAsSint32LE(_G(game).digi_overall_volume_percent);
		s.syncAsSint32LE(_G(game).midi_overall_volume_percent);
		s.syncAsByte(_G(kernel).camera_pan_instant);
	}

	_G(player).syncGame(s);

	_G(player_info).syncGame(s);

	syncFlags(s);

	player_been_sync(s);

	_G(conversations).syncGame(s);
	_G(inventory)->syncGame(s);

	Room *room = s.isSaving() ? g_engine->_activeRoom :
		g_engine->getRoom(_G(game).room_id);
	room->syncGame(s);

	if (s.isLoading()) {
		// set up variables so everyone knows we've teleported
		_G(kernel).restore_game = true;
		_G(between_rooms) = true;
		_G(game).previous_room = KERNEL_RESTORING_GAME;

		digi_set_overall_volume(_G(game).digi_overall_volume_percent);
		midi_set_overall_volume(_G(game).midi_overall_volume_percent);
		interface_show();
	}

	return Common::kNoError;
}

bool M4Engine::autosaveExists() const {
	Common::String slotName = getSaveStateName(getAutosaveSlot());
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != nullptr;
	delete saveFile;

	return result;
}

SaveStateList M4Engine::listSaves() const {
	return getMetaEngine()->listSaves(_targetName.c_str());
}

bool M4Engine::savesExist() const {
	return !listSaves().empty();
}

bool M4Engine::loadSaveThumbnail(int slotNum, M4sprite *thumbnail) const {
	SaveStateDescriptor desc = getMetaEngine()->querySaveMetaInfos(_targetName.c_str(), slotNum);
	if (!desc.isValid())
		return false;

	// Gert the thumbnail
	const Graphics::Surface *surf = desc.getThumbnail();
	assert(surf->format.bytesPerPixel == 2);

	// Set up output sprite
	thumbnail->w = surf->w;
	thumbnail->h = surf->h;
	thumbnail->encoding = NO_COMPRESS;

	byte *data = (byte *)malloc(surf->w * surf->h);
	thumbnail->sourceHandle = (MemHandle)malloc(sizeof(MemHandle));
	*thumbnail->sourceHandle = data;
	thumbnail->sourceOffset = 0;
	thumbnail->data = data;

	byte pal[PALETTE_SIZE];
	byte r, g, b;
	int proximity, minProximity;
	g_system->getPaletteManager()->grabPalette(pal, 0, PALETTE_COUNT);

	// Translate the 16-bit thumbnail to paletted
	for (int y = 0; y < surf->h; ++y) {
		const uint16 *srcLine = (const uint16 *)surf->getBasePtr(0, y);
		byte *destLine = data + surf->w * y;

		for (int x = 0; x < surf->w; ++x, ++srcLine, ++destLine) {
			proximity = minProximity = 0xffff;
			surf->format.colorToRGB(*srcLine, r, g, b);

			const byte *palP = pal;
			for (int palIdx = 0; palIdx < PALETTE_COUNT; ++palIdx, palP += 3) {
				proximity = ABS((int)r - (int)palP[0]) +
					ABS((int)g - (int)palP[1]) +
					ABS((int)b - (int)palP[2]);

				if (proximity < minProximity) {
					minProximity = proximity;
					*destLine = (byte)palIdx;
				}
			}
		}
	}

	return true;
}

bool M4Engine::saveGameFromMenu(int slotNum, const Common::String &desc,
		Graphics::Surface &thumbnail) {
	M4MetaEngine *metaEngine = static_cast<M4MetaEngine *>(getMetaEngine());
	metaEngine->_thumbnail = &thumbnail;

	bool result = saveGameState(slotNum, desc).getCode() == Common::kNoError;

	metaEngine->_thumbnail = nullptr;
	return result;
}

} // End of namespace M4
