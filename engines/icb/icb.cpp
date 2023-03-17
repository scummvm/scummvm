/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/icb.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "audio/mixer.h"

namespace ICB {

IcbEngine *g_icb;

IcbEngine::IcbEngine(OSystem *syst, const IcbGameDescription *gameDesc) : Engine(syst) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_randomSource = new Common::RandomSource("icb");
	g_icb = this;
	_gameDescription = &gameDesc->desc;
	_gameType = gameDesc->gameType;
	(void)_gameDescription; // silence warning
}

IcbEngine::~IcbEngine() {
	delete _randomSource;
	g_icb = nullptr;
}

Common::KeymapArray IcbEngine::initKeymapsIcb(const char *target) {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "icb", "In Cold Blood");
	Action *act;

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_UP);
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_DOWN);
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_LEFT);
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_RIGHT);
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("BFIR", _("Fire")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_SPACE));
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("BUSE", _("Interact")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_LCTRL));
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("BINV", _("Inventory")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_RETURN));
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("BARM", _("Arm")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_LALT));
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("BREM", _("Remora")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_r));
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("BCRU", _("Crouch")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_x));
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action("BSID", _("Side Step")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_LSHIFT));
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	engineKeyMap->addAction(act);

	act = new Action("BRUN", _("Run")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_z));
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	engineKeyMap->addAction(act);

	act = new Action("BPAS", _("Pause")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_BACK");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

Common::KeymapArray IcbEngine::initKeymapsEldorado(const char *target) {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "eldorado", "The Road to El Dorado");
	Action *act;

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_UP);
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_DOWN);
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_LEFT);
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_RIGHT);
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("BUSE", _("Interact")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_LCTRL));
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("BINV", _("Inventory")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_RETURN));
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("BCRU", _("Crouch")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_x));
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action("BSID", _("Side Step")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_LSHIFT));
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	engineKeyMap->addAction(act);

	act = new Action("BRUN", _("Run")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_z));
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	engineKeyMap->addAction(act);

	act = new Action("BPAS", _("Pause")); // I18N: Action in ICB
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_BACK");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

bool IcbEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher);
}

// TODO: Refactor, this is currently implemented in p4_pc.cpp
void InitEngine(const char *cmdLine);
bool mainLoopIteration();
void quitEngine();

Common::Error IcbEngine::run() {
	initGfx(false, false);
	InitEngine("");
	mainLoop();
	return Common::kNoError;
}

// TODO: This should be refactored
void IcbEngine::initGfx(bool hwAccel, bool fullscreen) {
}

void IcbEngine::mainLoop() {
	while (mainLoopIteration()) {
		if (shouldQuit()) {
			quitEngine();
			break;
		}
	}
}

} // End of namespace ICB
