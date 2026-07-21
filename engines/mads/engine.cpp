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
#include "common/config-manager.h"
#include "common/memstream.h"
#include "engines/util.h"
#include "mads/mads.h"
#include "mads/engine.h"
#include "mads/core/anim.h"
#include "mads/core/attr.h"
#include "mads/core/camera.h"
#include "mads/core/config.h"
#include "mads/core/conv.h"
#include "mads/core/cursor.h"
#include "mads/core/cycle.h"
#include "mads/core/dialog.h"
#include "mads/core/font.h"
#include "mads/core/game.h"
#include "mads/core/global.h"
#include "mads/core/hspot.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/keys.h"
#include "mads/core/matte.h"
#include "mads/core/mcga.h"
#include "mads/core/object.h"
#include "mads/core/pal.h"
#include "mads/core/player.h"
#include "mads/core/popup.h"
#include "mads/core/quote.h"
#include "mads/core/rail.h"
#include "mads/core/room.h"
#include "mads/core/speech.h"
#include "mads/core/sprite.h"
#include "mads/core/timer.h"
#include "mads/core/vocab.h"
#include "mads/phantom/main.h"
#include "mads/forest/extra.h"
#include "mads/forest/global.h"
#include "mads/core/sound_manager.h"

namespace MADS {

constexpr int SAVEGAME_VERSION = 1;
constexpr int GAME_FRAME_RATE = 50;
constexpr int GAME_FRAME_TIME = 1000 / GAME_FRAME_RATE;

MADSV2Engine *g_engine;

static const Common::KeyCode KEYBINDING_ACTIONS[kActionRestartAnimation + 1] = {
	Common::KEYCODE_INVALID, Common::KEYCODE_ESCAPE, Common::KEYCODE_F1,
	Common::KEYCODE_F5, Common::KEYCODE_F7, Common::KEYCODE_PAGEUP,
	Common::KEYCODE_PAGEDOWN, Common::KEYCODE_F1, Common::KEYCODE_F2,
	Common::KEYCODE_F3, Common::KEYCODE_F4, Common::KEYCODE_F5,
	Common::KEYCODE_INVALID
};


MADSV2Engine::MADSV2Engine(OSystem *syst, const MADSGameDescription *gameDesc) :
	MADSEngine(syst, gameDesc) {
	g_engine = this;
	_speechFlag = true;

	initGlobals();
}

MADSV2Engine::~MADSV2Engine() {
	g_engine = nullptr;
	delete _screen;
	delete _soundManager;
}

void MADSV2Engine::initializePath(const Common::FSNode &gamePath) {
	MADSEngine::initializePath(gamePath);
	Common::FSNode folder = gamePath.getChild("resource");
	if (folder.exists())
		SearchMan.addDirectory(folder);
}

void MADSV2Engine::initGlobals() {
	init_anim();
	init_attr();
	init_camera();
	init_conv();
	init_cursor();
	init_cycle();
	init_dialog();
	init_font();
	init_game();
	init_global();
	init_hspot();
	init_inter();
	init_kernel();
	init_keys();
	init_matte();
	init_object();
	init_pal();
	init_player();
	init_popup();
	init_quote();
	init_rail();
	init_room();
	init_speech();
	init_sprite();
	init_timer();
	init_vocab();
	Forest::init_extra();
}

void MADSV2Engine::readConfigFile() {
	read_config_file();
	_musicFlag = config_file.music_flag;
	_soundFlag = config_file.sound_flag;
	_speechFlag = config_file.speech_flag;

	if (ConfMan.hasKey("save_slot"))
		savegame_slot = ConfMan.getInt("save_slot");
}

bool MADSV2Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return game.going && !win_status && !kernel.activate_menu &&
		inter_input_mode == INTER_BUILDING_SENTENCES &&
		conv_control.running == -1;
}

Common::Error MADSV2Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	stream->writeByte(SAVEGAME_VERSION);
	Common::Serializer s(nullptr, stream);
	s.setVersion(SAVEGAME_VERSION);

	// Sync main game data
	syncGame(s);

	// Save conversation data
	conv_append(stream);

	return Common::kNoError;
}

Common::Error MADSV2Engine::loadGameStream(Common::SeekableReadStream *stream) {
	int save = player.walker_is_loaded;

	byte version = stream->readByte();
	if (version != SAVEGAME_VERSION)
		error("Invalid savegame version");


	// Sync main game data
	Common::Serializer s(stream, nullptr);
	syncGame(s);

	// Load conversation data
	if (conv_expand(stream))
		goto done;

	if (inven_num_objects > 0) {
		active_inven = 0;
	} else {
		active_inven = -1;
	}

	first_inven = 0;

	section_id = KERNEL_RESTORING_GAME;
	room_id = KERNEL_RESTORING_GAME;

	new_section = new_room / 100;

	kernel.clock = timer_read();
	game.going = true;

done:
	player.walker_is_loaded = save;
	return Common::kNoError;
}
	
void MADSV2Engine::syncGame(Common::Serializer &s) {
	if (s.isSaving()) {
		// Write the room specific locals to the game scratchpad
		Common::fill(game.scratch, game.scratch + KERNEL_SCRATCH_SIZE, 0);
		Common::MemoryWriteStream scratchStream(game.scratch, KERNEL_SCRATCH_SIZE);
		Common::Serializer s2(nullptr, &scratchStream);
		syncRoom(s2);
	}

	game.synchronize(s);
	s.syncAsSint16LE(new_room);

	if (s.isLoading()) {
		// Unpack the loaded scratch data for the room
		Common::MemoryReadStream scratchStream(game.scratch, KERNEL_SCRATCH_SIZE);
		Common::Serializer s2(&scratchStream, nullptr);
		syncRoom(s2);
	}

	player2.synchronize(s);

	s.syncAsSint16LE(inven_num_objects);
	for (int i = 0; i < inven_num_objects; ++i)
		s.syncAsSint16LE(inven[i]);

	player.synchronize(s);
	for (int i = 0; i < global_list_size; ++i)
		s.syncAsSint16LE(global[i]);

	for (int i = 0; i < num_objects; ++i)
		object[i].synchronize(s);

	s.syncAsSint16LE(conv_restore_running);

	if (s.isSaving()) {
		s.syncAsSint16LE(picture_view_x);
		s.syncAsSint16LE(picture_view_y);
	} else {
		s.syncAsSint16LE(camera_old_x_target);
		s.syncAsSint16LE(camera_old_y_target);
	}

	if (getGameID() == GType_Forest)
		s.syncMultipleLE(Forest::flags);

	for (int i = 0; i < OMR; ++i)
		s.syncAsSint16LE(room_state[i]);
	s.syncAsSint16LE(previous_room);
}

void MADSV2Engine::pollEvents() {
	// Check for screen update time
	uint32 time = g_system->getMillis();
	if (time >= _nextFrameTime) {
		updateScreen();
		_nextFrameTime = time + GAME_FRAME_TIME;
	}

	// Handle calling any set timer function
	checkForTimerFunction();

	// Poll for events
	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		bool isMouse = false;
		switch (e.type) {
		case Common::EVENT_LBUTTONDOWN:
			_mouseButtons |= 1;
			isMouse = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseButtons &= ~1;
			isMouse = true;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons |= 2;
			isMouse = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseButtons &= ~2;
			isMouse = true;
			break;
		case Common::EVENT_MBUTTONDOWN:
			_mouseButtons |= 4;
			isMouse = true;
			break;
		case Common::EVENT_MBUTTONUP:
			_mouseButtons &= ~4;
			isMouse = true;
			break;
		case Common::EVENT_MOUSEMOVE:
			isMouse = true;
			break;
		case Common::EVENT_RETURN_TO_LAUNCHER:
		case Common::EVENT_QUIT:
			game.going = false;
			break;
		default:
			break;
		}

		if (isMouse)
			_mousePos = e.mouse;

		if (e.type == Common::EVENT_KEYDOWN && !isSpecialKey(e.kbd.keycode))
			_keyEvents.push(e.kbd);
		if (e.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
				KEYBINDING_ACTIONS[e.customType] != Common::KEYCODE_INVALID)
			_keyEvents.push(Common::KeyState(KEYBINDING_ACTIONS[e.customType]));
	}
}

void MADSV2Engine::updateScreen() {
	// Handle any screen shaking
	if (mcga_shakes) {
		_shakeRandom = _shakeRandom * 5 + 1;
		int offset = (_shakeRandom >> 8) & 3;
		if (--mcga_shakes == 0)
			offset = 0;

		// Manually copy the screen with the left hand hide side of the screen of a given offset width shown
		// at the very right. The offset changes to give an effect of shaking the screen
		offset *= 4;
		const byte *buf = (const byte *)_screen->getBasePtr(offset, 0);
		g_system->copyRectToScreen(buf, 320, 0, 0, 320 - offset, 200);
		if (offset > 0)
			g_system->copyRectToScreen(_screen->getPixels(), 320, 320 - offset, 0, offset, 200);
		g_system->updateScreen();

	} else {
		// Because the screen is accessed directly via Buffer objects, we need to do a full screen update each frame
		_screen->markAllDirty();
		_screen->update();
	}
}

bool MADSV2Engine::isSpecialKey(Common::KeyCode key) const {
	static const Common::KeyCode KEYS[] = {
		Common::KEYCODE_LCTRL, Common::KEYCODE_LALT, Common::KEYCODE_RSHIFT, Common::KEYCODE_RALT,
	};

	for (const Common::KeyCode &kc : KEYS) {
		if (kc == key)
			return true;
	}

	return false;
}

void MADSV2Engine::checkForTimerFunction() {
	if (_timerFunction && _nextTimerTime != (uint32)-1) {
		uint32 time = g_system->getMillis();
		if (time >= _nextTimerTime) {
			// Flag the timer as disabled to prevent recursive calls
			_nextTimerTime = (uint32)-1;

			// Call the timer
			_timerFunction();

			// Determine the next time to call the function at 60Hz
			_nextTimerTime = time + (1000 / 60);
		}
	}
}

bool MADSV2Engine::hasPendingKey() {
	pollEvents();

	return !_keyEvents.empty();
}

int MADSV2Engine::getKey() {
	pollEvents();

	if (!_keyEvents.empty()) {
		Common::KeyState ks = _keyEvents.pop();
		return ks.ascii && (ks.flags == 0) ? ks.ascii : (ks.flags << 16) | ks.keycode;
	}

	return 0;
}

void MADSV2Engine::flushKeys() {
	pollEvents();

	_keyEvents.clear();
}

int MADSV2Engine::getMouseState(int &x, int &y) {
	pollEvents();

	x = _mousePos.x;
	y = _mousePos.y;
	return _mouseButtons;
}

uint32 MADSV2Engine::getMillis() {
	pollEvents();
	return g_system->getMillis();
}

void MADSV2Engine::playSpeech(Audio::AudioStream *stream) {
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream);
}

void MADSV2Engine::stopSpeech() {
	_mixer->stopHandle(_speechHandle);
}

bool MADSV2Engine::isSpeechPlaying() const {
	return _mixer->isSoundHandleActive(_speechHandle);
}

SaveStateList MADSV2Engine::listSaves() const {
	return getMetaEngine()->listSaves(_targetName.c_str());
}

void MADSV2Engine::player_keep_walking() {
	int at_x, at_y;
	int walk_code;
	int id;
	int new_facing = false;
	int temp_velocity;
	int angle_scale;
	int angle_range;

	while (player.walking && !player.walk_off_edge && (player.x == player.target_x) && (player.y == player.target_y)) {
		if (rail_solution_stack_pointer == 0) {
			if (player.walk_off_edge_to_room) {
				player.walk_off_edge = player.walk_off_edge_to_room;
				player.walk_anywhere = true;
				player.walk_off_edge_to_room = 0;
				player.commands_allowed = false;
				new_facing = false;
			} else {
				player.walking = false;
				player_set_final_facing();
				new_facing = true;
			}
		} else {
			id = rail_solution_stack[--rail_solution_stack_pointer];
			player.target_x = room->rail[id].x;
			player.target_y = room->rail[id].y;
			new_facing = true;
		}
	}

	if (new_facing) {
		if (player.walking) player_set_facing();
	}

	if (player.facing != player.turn_to_facing) {
		player_keep_turning();
	} else {
		if (!player.walking) {
			player_new_stop_walker();
			player_activate_trigger();
		}
	}

	temp_velocity = player.velocity;

	if (player.scaling_velocity && (player.total_distance > 0)) {
		angle_range = 100 - player.scale;
		angle_scale = player.scale + ((angle_range * (player.x_count - 1)) / player.total_distance);
		temp_velocity = (int)(((long)temp_velocity * ((long)player.scale * (long)angle_scale)) / 10000L);
		temp_velocity = MAX(temp_velocity, 1);
	}

	if (player.walking && (player.facing == player.turn_to_facing)) {
		at_x = player.x;
		at_y = player.y;
		walk_code = false;
		player.special_code = 0;

		if (player.dist_accum < temp_velocity) {

			do {
				if (player.pixel_accum < player.x_count) {
					player.pixel_accum += player.y_count;
				}
				if (player.pixel_accum >= player.x_count) {
					if ((player.y_counter > 0) || player.walk_off_edge) at_y += player.sign_y;
					player.y_counter--;
					player.pixel_accum -= player.x_count;
				}
				if (player.pixel_accum < player.x_count) {
					if ((player.x_counter > 0) || player.walk_off_edge) at_x += player.sign_x;
					player.x_counter--;
				}

				if (!player.walk_anywhere && !(player.walk_off_edge || player.walk_off_edge_to_room)) {
					walk_code |= attr_walk(&scr_walk, at_x, at_y);
					if (!player.special_code) {
						player.special_code = (g_engine->getGameID() == GType_RexNebular) ?
							rex_attr_special(&scr_special, at_x, at_y) :
							attr_special(&scr_special, at_x, at_y);
					}
				}

				player.dist_accum += player.delta_distance;
			} while ((player.dist_accum < temp_velocity) && (!walk_code) &&
				((player.x_counter > 0) || (player.y_counter > 0) || (player.walk_off_edge)));

		}

		player.dist_accum -= temp_velocity;

		if (walk_code) {
			player_cancel_command();
		} else {
			if (!player.walk_off_edge) {
				if (player.x_counter <= 0) at_x = player.target_x;
				if (player.y_counter <= 0) at_y = player.target_y;
			}
			player.x = at_x;
			player.y = at_y;
		}
	}
}

} // namespace MADS
