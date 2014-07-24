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
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "sound.h"
#include "common/config-manager.h"
#include "cge2/cge2.h"
#include "cge2/events.h"
#include "cge2/vmenu.h"
#include "cge2/text.h"
#include "cge2/cge2_main.h"

namespace CGE2 {

void CGE2Engine::optionTouch(int opt, uint16 mask) {
	switch (opt) {
	case 1:
		if (mask & kMouseLeftUp)
			switchColorMode();
		break;
	case 2:
		if ((mask & kMouseLeftUp) && !ConfMan.getBool("mute")) {
			switchMusic(_music = !_music);

			switch (_music) {
			case false:
				_oldMusicVolume = ConfMan.getInt("music_volume");
				ConfMan.setInt("music_volume", 0);
				break;
			case true:
				ConfMan.setInt("music_volume", _oldMusicVolume);
				break;
			}
		}
		break;
	case 3:
		if (mask & kMouseLeftUp)
			quit();
		break;
	case 4:
		if (mask & (kMouseLeftUp | kMouseRightUp))
			setVolume(opt - 4, (mask & kMouseLeftUp) ? 1 : -1);
		break;
	case 5:
		if (mask & (kMouseLeftUp | kMouseRightUp))
			setVolume(opt - 4, (mask & kMouseLeftUp) ? 1 : -1);
		break;
	case 8:
		if (mask & kMouseLeftUp)
			switchCap();
		break;
	case 9:
		if ((mask & kMouseLeftUp) && !ConfMan.getBool("mute")) {
			switchVox();

			switch (_sayVox) {
			case false:
				_oldSpeechVolume = ConfMan.getInt("speech_volume");
				ConfMan.setInt("speech_volume", 0);
				break;
			case true:
				ConfMan.setInt("speech_volume", _oldSpeechVolume);
				break;
			}
		}
		break;
	default:
		break;
	}
}

void CGE2Engine::switchColorMode() {
	_commandHandlerTurbo->addCommand(kCmdSeq, 121, _vga->_mono = !_vga->_mono, nullptr);
	keyClick();
	_vga->setColors(_vga->_sysPal, 64);
}

void CGE2Engine::switchMusic(bool on) {
	_commandHandlerTurbo->addCommand(kCmdSeq, kMusicRef, on, nullptr);
	keyClick();
	_commandHandlerTurbo->addCommand(kCmdMidi, -1, on ? (_now << 8) : -1, nullptr);
}

void CGE2Engine::checkMusicSwitch() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");
	bool musicMuted = mute;
	int musicVolume = ConfMan.getInt("music_volume");
	if (!musicMuted)
		musicMuted = musicVolume == 0;

	if (!musicMuted && !_music) {
		switchMusic(_music = true);
	}
	if (musicMuted && _music) {
		switchMusic(_music = false);
	}
}

void CGE2Engine::quit() {
	Common::Array<Choice *> quitMenu; // Deleted in VMenu's destructor.
	quitMenu.push_back(new StartCountDownChoice(this));
	quitMenu.push_back(new ResetQSwitchChoice(this));

	if (_commandHandler->idle()) {
		if (VMenu::_addr) {
			_commandHandlerTurbo->addCommand(kCmdKill, -1, 0, VMenu::_addr);
			ResetQSwitchChoice rqsChoice(this);
			rqsChoice.proc();
		} else {
			quitMenu[0]->_text = _text->getText(kQuitText);
			quitMenu[1]->_text = _text->getText(kNoQuitText);
			(new VMenu(this, quitMenu, V2D(this, -1, -1), kCBMnu))->setName(_text->getText(kQuitTitle));
			_commandHandlerTurbo->addCommand(kCmdSeq, kPowerRef, 0, nullptr);
			keyClick();
		}
	}
}

void CGE2Engine::setVolume(int idx, int cnt) {
	warning("STUB: CGE2Engine::setVolume()");
}

void CGE2Engine::switchCap() {
	if (_enaCap && _enaVox) {
		_sayCap = !_sayCap;
		if (!_sayCap && _enaVox)
			_sayVox = true;
		keyClick();
		switchSay();
	}
}

void CGE2Engine::switchVox() {
	if (_enaVox && _enaCap) {
		_sayVox = !_sayVox;
		if (!_sayVox && _enaCap)
			_sayCap = true;
		keyClick();
		switchSay();
	}
}

void CGE2Engine::switchSay() {
	_commandHandlerTurbo->addCommand(kCmdSeq, 129, _sayVox, nullptr);
	_commandHandlerTurbo->addCommand(kCmdSeq, 128, _sayCap, nullptr);
}

void CGE2Engine::checkSaySwitch() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");
	bool speechMuted = mute;
	if (!speechMuted) {
		int speechVolume = ConfMan.getInt("speech_volume");
		speechMuted = speechVolume == 0;
	}

	if (!speechMuted && !_sayVox) {
		switchVox();
	}
	if (speechMuted && _sayVox) {
		switchVox();
	}
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

	_startupMode = 0;
	_mouse->center();
	_mouse->off();
	_mouse->on();

	_keyboard->setClient(_sys);
	_commandHandler->addCommand(kCmdSeq, kPowerRef, 1, nullptr);

	_busyPtr = _vga->_showQ->locate(kBusyRef);

	_vol[0] = _vga->_showQ->locate(kDvolRef);
	_vol[1] = _vga->_showQ->locate(kMvolRef);

	// these sprites are loaded with SeqPtr==0 (why?!)
	if (_vol[0])
		_vol[0]->step((/*(int)SNDDrvInfo.VOL4.DL * */ _vol[0]->_seqCnt + _vol[0]->_seqCnt / 2) >> 4);
	if (_vol[1])
		_vol[1]->step((/*(int)SNDDrvInfo.VOL4.ML * */ _vol[1]->_seqCnt + _vol[1]->_seqCnt / 2) >> 4);
	// TODO: Recheck these! ^
}

} // End of namespace CGE2
