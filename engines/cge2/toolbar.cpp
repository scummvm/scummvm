/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "sound.h"
#include "common/config-manager.h"
#include "cge2/cge2.h"
#include "cge2/events.h"
#include "cge2/vmenu.h"
#include "cge2/text.h"
#include "cge2/cge2_main.h"

namespace CGE2 {

#define kSoundNumToStateRate  25.7
// == 257 / 10; where 10 equals to the volume switches' number of states [0..9]
// and ScummVM has a scale of 257 different values for setting sounds.

#define kSoundStateToNumRate  28.45
// == 256 / 9 + 0.1; where 256 is the positive range of numbers we can set the volume to
// and the 10 states of a switch cut this range up to 9 equally big parts.
// We don't take into account 0 regarding the 256 different values (it would be the 257th),
// since 0 * x == 0.
// 0.1 is only for correct rounding at the 10th state.

void CGE2Engine::optionTouch(int opt, uint16 mask) {
	bool notMuted = !ConfMan.getBool("mute");
	switch (opt) {
	case 1:
		if (mask & kMouseLeftUp)
			switchColorMode();
		break;
	case 2:
		if ((mask & kMouseLeftUp) && notMuted)
			switchMusic();
		break;
	case 3:
		if (mask & kMouseLeftUp)
			quit();
		break;
	case 4:
		if ((mask & (kMouseLeftUp | kMouseRightUp)) && notMuted)
			setVolume(opt - 4, (mask & kMouseLeftUp) ? 1 : -1);
		break;
	case 5:
		if ((mask & (kMouseLeftUp | kMouseRightUp)) && notMuted)
			setVolume(opt - 4, (mask & kMouseLeftUp) ? 1 : -1);
		break;
	case 8:
		if ((mask & kMouseLeftUp) && notMuted)
			switchCap();
		break;
	case 9:
		if ((mask & kMouseLeftUp) && notMuted)
			switchVox();
		break;
	default:
		break;
	}
}

void CGE2Engine::switchColorMode() {
	_commandHandlerTurbo->addCommand(kCmdSeq, 121, _vga->_mono = !_vga->_mono, nullptr);
	ConfMan.setBool("enable_color_blind", _vga->_mono);
	ConfMan.flushToDisk();
	keyClick();
	_vga->setColors(_vga->_sysPal, 64);
}

void CGE2Engine::switchMusic() {
	_music = !_music;
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, !_music);
	_commandHandlerTurbo->addCommand(kCmdSeq, kMusicRef, _music, nullptr);
	keyClick();
	_commandHandlerTurbo->addCommand(kCmdMidi, -1, _music ? (_now << 8) : -1, nullptr);
}

void CGE2Engine::quit() {
	if (_commandHandler->idle()) {
		if (VMenu::_addr) {
			_commandHandlerTurbo->addCommand(kCmdKill, -1, 0, VMenu::_addr);
			ReturnToGameChoice rqsChoice(this);
			rqsChoice.proc();
		} else {
			Common::Array<Choice *> quitMenu; // Deleted in VMenu's destructor.
			quitMenu.push_back(new ExitGameChoice(this));
			quitMenu.push_back(new ReturnToGameChoice(this));
			(new VMenu(this, quitMenu, V2D(this, -1, -1), kCBMnu))->setName(_text->getText(kQuitTitle));
			_commandHandlerTurbo->addCommand(kCmdSeq, kPowerRef, 0, nullptr);
			keyClick();
		}
	}
}

void CGE2Engine::setVolume(int idx, int cnt) {
	if (cnt && _vol[idx]) {
		int p = _vol[idx]->_seqPtr + cnt;
		if ((p >= 0) && (p < _vol[idx]->_seqCnt)) {
			_vol[idx]->step(p);
			int newVolume = (int)(p * kSoundStateToNumRate);
			switch (idx) {
			case 0:
				_oldSfxVolume = ConfMan.getInt("sfx_volume");
				ConfMan.setInt("sfx_volume", newVolume);
				break;
			case 1:
				_oldMusicVolume = ConfMan.getInt("music_volume");
				ConfMan.setInt("music_volume", newVolume);
				break;
			default:
				break;
			}
		}
	}
}

void CGE2Engine::checkVolumeSwitches() {
	int musicVolume = ConfMan.getInt("music_volume");
	if (musicVolume != _oldMusicVolume)
		_vol[1]->step((int)(musicVolume / kSoundNumToStateRate));

	int sfxVolume = ConfMan.getInt("sfx_volume");
	if (sfxVolume != _oldSfxVolume)
		_vol[0]->step((int)(sfxVolume / kSoundNumToStateRate));
}

void CGE2Engine::switchCap() {
	_sayCap = !_sayCap;
	if (!_sayCap)
		_sayVox = true;
	keyClick();
	switchSay();
}

void CGE2Engine::switchVox() {
	_sayVox = !_sayVox;
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, _sayVox);
	if (!_sayVox)
		_sayCap = true;
	keyClick();
	switchSay();
}

void CGE2Engine::switchSay() {
	_commandHandlerTurbo->addCommand(kCmdSeq, 129, _sayVox, nullptr);
	_commandHandlerTurbo->addCommand(kCmdSeq, 128, _sayCap, nullptr);
}

void CGE2Engine::initToolbar() {
	selectPocket(-1);

	_commandHandlerTurbo->addCommand(kCmdSeq, kMusicRef, _music, nullptr);
	if (!_music)
		_midiPlayer->killMidi();

	switchSay();

	_infoLine->gotoxyz(V3D(kInfoX, kInfoY, 0));
	_infoLine->setText(nullptr);
	_vga->_showQ->insert(_infoLine);

	_gamePhase = kPhaseInGame;
	_mouse->center();
	_mouse->off();
	_mouse->on();

	_keyboard->setClient(_sys);
	_commandHandler->addCommand(kCmdSeq, kPowerRef, 1, nullptr);

	_busyPtr = _vga->_showQ->locate(kBusyRef);

	_vol[0] = _vga->_showQ->locate(kDvolRef);
	_vol[1] = _vga->_showQ->locate(kMvolRef);

	if (_vol[0]) {
		int val = ConfMan.getInt("sfx_volume");
		initVolumeSwitch(_vol[0], val);
	}

	if (_vol[1]) {
		int val = ConfMan.getInt("music_volume");
		initVolumeSwitch(_vol[1], val);
	}
}

void CGE2Engine::initVolumeSwitch(Sprite *volSwitch, int val) {
	int state = 0;
	state = (int)(val / kSoundNumToStateRate);
	volSwitch->step(state);
}

void CGE2Engine::checkMute() {
	bool mute = ConfMan.getBool("mute");
	if (mute != _muteAll) {
		switchMusic();
		switchVox();
		_muteAll = mute;
	}
}

} // End of namespace CGE2
