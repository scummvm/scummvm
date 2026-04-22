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
#include "engines/util.h"
#include "mads/mads.h"
#include "mads/madsv2/engine.h"
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/phantom/main.h"
#include "mads/core/sound.h"

namespace MADS {
namespace MADSV2 {

constexpr int GAME_FRAME_RATE = 50;
constexpr int GAME_FRAME_TIME = 1000 / GAME_FRAME_RATE;

MADSV2Engine *g_engine;

static const Common::KeyCode KEYBINDING_ACTIONS[kActionRestartAnimation + 1] = {
	Common::KEYCODE_INVALID, Common::KEYCODE_ESCAPE, Common::KEYCODE_F1,
	Common::KEYCODE_F5, Common::KEYCODE_7, Common::KEYCODE_PAGEUP,
	Common::KEYCODE_PAGEDOWN, Common::KEYCODE_F1, Common::KEYCODE_F2,
	Common::KEYCODE_F3, Common::KEYCODE_F4, Common::KEYCODE_F5,
	Common::KEYCODE_INVALID
};


MADSV2Engine::MADSV2Engine(OSystem *syst, const MADSGameDescription *gameDesc) :
	MADSEngine(syst, gameDesc) {
	g_engine = this;
	_speechFlag = true;
}

MADSV2Engine::~MADSV2Engine() {
	g_engine = nullptr;
	delete _screen;
	delete _soundManager;
}

void MADSV2Engine::readConfigFile() {
	read_config_file();
	_musicFlag = config_file.music_flag;
	_soundFlag = config_file.sound_flag;
	_speechFlag = config_file.speech_flag;
}

bool MADSV2Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return game.going && !win_status && !kernel.activate_menu &&
		inter_input_mode == INTER_BUILDING_SENTENCES &&
		conv_control.running == -1;
}

Common::Error MADSV2Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	// Sync main game data
	Common::Serializer s(nullptr, stream);
	syncGame(s);

	// Save conversation data
	conv_append(stream);

	return Common::kNoError;
}

Common::Error MADSV2Engine::loadGameStream(Common::SeekableReadStream *stream) {
	int save = player.walker_is_loaded;

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
	game.synchronize(s);
	s.syncAsSint16LE(new_room);
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

	for (int i = 0; i < OMR; ++i)
		s.syncAsSint16LE(room_state[i]);
	s.syncAsSint16LE(previous_room);
}

void MADSV2Engine::pollEvents() {
	// Check for screen update time
	uint32 time = g_system->getMillis();
	if (time >= _nextFrameTime) {
		// Because the screen is accessed directly via Buffer objects,
		// we need to do a full screen update each frame
		_screen->markAllDirty();
		_screen->update();
		_nextFrameTime = time + GAME_FRAME_TIME;
	}

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

		if (e.type == Common::EVENT_KEYDOWN)
			_keyEvents.push(e.kbd);
		if (e.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
				KEYBINDING_ACTIONS[e.customType] != Common::KEYCODE_INVALID)
			_keyEvents.push(Common::KeyState(KEYBINDING_ACTIONS[e.customType]));
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
		return (ks.ascii && !ks.flags) ? ks.ascii : (ks.flags << 16) | ks.keycode;
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

SaveStateList MADSV2Engine::listSaves() const {
	return getMetaEngine()->listSaves(_targetName.c_str());
}

} // namespace MADSV2
} // namespace MADS
