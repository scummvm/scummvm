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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
#include "dsoptions.h"
#include "dsmain.h"
#include "gui/dialog.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"
#include "osystem_ds.h"
#include "engines/scumm/scumm.h"
#include "touchkeyboard.h"

#ifdef DS_SCUMM_BUILD
namespace Scumm {
	extern Common::StringList generateSavegameList(Scumm::ScummEngine *scumm, bool saveMode);
	extern Scumm::ScummEngine *g_scumm;
}
#endif

namespace DS {

DSOptionsDialog::DSOptionsDialog() : GUI::Dialog(20, 0, 320 - 40, 230 - 20) {
	addButton(this, 10, 175, "Close", GUI::kCloseCmd, 'C');
	
#ifdef DS_SCUMM_BUILD
	if (!DS::isGBAMPAvailable()) {
//		addButton(this, 100, 140, "Delete Save", 'dels', 'D');
	}
#endif

	new GUI::StaticTextWidget(this, 80, 10, 130, 15, "ScummVM DS Options", GUI::kTextAlignCenter);

	_leftHandedCheckbox = new GUI::CheckboxWidget(this, 20, 25, 200, 20, "Left handed mode", 0, 'L');
	_indyFightCheckbox = new GUI::CheckboxWidget(this, 20, 40, 200, 20, "Indy fighting controls", 0, 'I');
	_unscaledCheckbox = new GUI::CheckboxWidget(this, 20, 55, 200, 20, "Unscaled main screen", 0, 'S');
	_twoHundredPercentCheckbox = new GUI::CheckboxWidget(this, 20, 70, 230, 20, "Zoomed screen at fixed 200% zoom", 0, 'T');
	_highQualityAudioCheckbox = new GUI::CheckboxWidget(this, 20, 85, 250, 20, "High quality audio (slower) (reboot)", 0, 'T');
	_disablePowerOff = new GUI::CheckboxWidget(this, 20, 100, 250, 20, "Disable power off on quit", 0, 'T');
//	_cpuScaler = new GUI::CheckboxWidget(this, 20, 115, 250, 20, "CPU scaler", 0, 'T');
	_showCursorCheckbox = new GUI::CheckboxWidget(this, 20, 115, 250, 20, "Show mouse cursor", 0, 'T');
	_snapToBorderCheckbox = new GUI::CheckboxWidget(this, 20, 130, 250, 20, "Snap to border", 0, 'T');

	new GUI::StaticTextWidget(this, 20, 145, 110, 15, "Touch X Offset", GUI::kTextAlignLeft);
	_touchX = new GUI::SliderWidget(this, 130, 145, 130, 12, 1);
	_touchX->setMinValue(-8);
	_touchX->setMaxValue(+8);
	_touchX->setValue(0);
	_touchX->setFlags(GUI::WIDGET_CLEARBG);

	new GUI::StaticTextWidget(this, 20, 160, 110, 15, "Touch Y Offset", GUI::kTextAlignLeft);
	_touchY = new GUI::SliderWidget(this, 130, 160, 130, 12, 2);
	_touchY->setMinValue(-8);
	_touchY->setMaxValue(+8);
	_touchY->setValue(0);
	_touchY->setFlags(GUI::WIDGET_CLEARBG);

	new GUI::StaticTextWidget(this, 130 + 65 - 10, 175, 20, 15, "0", GUI::kTextAlignCenter);
	new GUI::StaticTextWidget(this, 130 + 130 - 10, 175, 20, 15, "8", GUI::kTextAlignCenter);
	new GUI::StaticTextWidget(this, 130 - 10, 175, 20, 15, "-8", GUI::kTextAlignCenter);

#ifdef DS_SCUMM_BUILD
	_delDialog = new Scumm::SaveLoadChooser("Delete game:", "Delete", false, Scumm::g_scumm);
#endif

	if (ConfMan.hasKey("snaptoborder", "ds")) {
		_snapToBorderCheckbox->setState(ConfMan.getBool("snaptoborder", "ds"));
	} else {
#ifdef DS_BUILD_D
		_snapToBorderCheckbox->setState(true);
#else
		_snapToBorderCheckbox->setState(false);
#endif
	}

	if (ConfMan.hasKey("showcursor", "ds")) {
		_showCursorCheckbox->setState(ConfMan.getBool("showcursor", "ds"));
	} else {
		_showCursorCheckbox->setState(true);
	}

	if (ConfMan.hasKey("lefthanded", "ds")) {
		_leftHandedCheckbox->setState(ConfMan.getBool("lefthanded", "ds"));
	} else {
		_leftHandedCheckbox->setState(false);
	}

	if (ConfMan.hasKey("unscaled", "ds")) {
		_unscaledCheckbox->setState(ConfMan.getBool("unscaled", "ds"));
	} else {
		_unscaledCheckbox->setState(false);
	}

	if (ConfMan.hasKey("twohundredpercent", "ds")) {
		_twoHundredPercentCheckbox->setState(ConfMan.getBool("twohundredpercent", "ds"));
	} else {
		_twoHundredPercentCheckbox->setState(false);
	}

	if (ConfMan.hasKey("22khzaudio", "ds")) {
		_highQualityAudioCheckbox->setState(ConfMan.getBool("22khzaudio", "ds"));
	} else {
		_highQualityAudioCheckbox->setState(false);
	}

	if (ConfMan.hasKey("disablepoweroff", "ds")) {
		_disablePowerOff->setState(ConfMan.getBool("disablepoweroff", "ds"));
	} else {
		_disablePowerOff->setState(false);
	}
/*
	if (ConfMan.hasKey("cpu_scaler", "ds")) {
		_cpuScaler->setState(ConfMan.getBool("cpu_scaler", "ds"));
	} else {
		_cpuScaler->setState(false);
	}
*/
	_indyFightCheckbox->setState(DS::getIndyFightState());

	if (ConfMan.hasKey("xoffset", "ds")) {
		_touchX->setValue(ConfMan.getInt("xoffset", "ds"));
	} else {
		_touchX->setValue(0);
	}

	if (ConfMan.hasKey("yoffset", "ds")) {
		_touchY->setValue(ConfMan.getInt("yoffset", "ds"));
	} else {
		_touchY->setValue(0);
	}
	
}

DSOptionsDialog::~DSOptionsDialog() {
	ConfMan.setBool("lefthanded", _leftHandedCheckbox->getState(), "ds");
	ConfMan.setBool("unscaled", _unscaledCheckbox->getState(), "ds");
	ConfMan.setBool("twohundredpercent", _twoHundredPercentCheckbox->getState(), "ds");
	ConfMan.setBool("22khzaudio", _highQualityAudioCheckbox->getState(), "ds");
	ConfMan.setBool("disablepoweroff", _disablePowerOff->getState(), "ds");
//	ConfMan.setBool("cpu_scaler", _cpuScaler->getState(), "ds");	
	ConfMan.setInt("xoffset", _touchX->getValue(), "ds");
	ConfMan.setInt("yoffset", _touchY->getValue(), "ds");
	ConfMan.setBool("showcursor", _showCursorCheckbox->getState(), "ds");
	ConfMan.setBool("snaptoborder", _snapToBorderCheckbox->getState(), "ds");
	DS::setOptions();
	DS::setIndyFightState(_indyFightCheckbox->getState());
	ConfMan.flushToDisk();
}


void DSOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == GUI::kCloseCmd) {
		close();
	}

#ifdef DS_SCUMM_BUILD
/*	if (cmd == 'dels') {
		_delDialog->setList(Scumm::generateSavegameList(Scumm::g_scumm, false));
		_delDialog->handleCommand(NULL, GUI::kListSelectionChangedCmd, 0);
		
		Common::Event event;
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.ascii = SDLK_DOWN;
		event.kbd.keycode = SDLK_DOWN;
		OSystem_DS::instance()->addEvent(event);

		event.type = Common::EVENT_KEYUP;
		OSystem_DS::instance()->addEvent(event);
				
		int idx = _delDialog->runModal();
		
		if (idx >= 0) {
			char name[256];
			Scumm::g_scumm->makeSavegameName(name, idx, false);
			if (!DS::isGBAMPAvailable()) {
				((DSSaveFileManager *) (OSystem_DS::instance()->getSavefileManager()))->deleteFile(name);
			}
		}
		
	}*/
#endif
	

}

void togglePause() {
	// Toggle pause mode by simulating pressing 'p'.  Not a good way of doing things!

	if (getCurrentGame()->control == CONT_SCUMM_ORIGINAL) {
		Common::Event event;
		OSystem_DS* system = OSystem_DS::instance();

		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = 'p';		
		event.kbd.ascii = 'p';
		event.kbd.flags = 0;
		system->addEvent(event);
	
		event.type = Common::EVENT_KEYUP;
		system->addEvent(event);
	}
}

void showOptionsDialog() {

	togglePause();

	DS::displayMode16Bit();
	

	DSOptionsDialog* d = new DSOptionsDialog();
	d->runModal();
	delete d;
	
	
	DS::displayMode8Bit();

	togglePause();
}

void setOptions() {
	ConfMan.addGameDomain("ds");

	if (ConfMan.hasKey("lefthanded", "ds")) {
		DS::setLeftHanded(ConfMan.getBool("lefthanded", "ds"));
	} else {
		DS::setLeftHanded(false);
	}

	if (ConfMan.hasKey("showcursor", "ds")) {
		DS::setMouseCursorVisible(ConfMan.getBool("showcursor", "ds"));
	} else {
		DS::setMouseCursorVisible(true);
	}

	if (ConfMan.hasKey("snaptoborder", "ds")) {
		DS::setSnapToBorder(ConfMan.getBool("snaptoborder", "ds"));
	} else {
#ifdef DS_BUILD_D
		DS::setSnapToBorder(true);
#else
		DS::setSnapToBorder(false);
#endif
	}

	if (ConfMan.hasKey("unscaled", "ds")) {
		DS::setUnscaledMode(ConfMan.getBool("unscaled", "ds"));
	} else {
		DS::setUnscaledMode(false);
	}

	if (ConfMan.hasKey("twohundredpercent", "ds")) {
		DS::set200PercentFixedScale(ConfMan.getBool("twohundredpercent", "ds"));
	} else {
		DS::set200PercentFixedScale(false);
	}

	if (ConfMan.hasKey("xoffset", "ds")) {
		DS::setTouchXOffset(ConfMan.getInt("xoffset", "ds"));
	} else {
		DS::setTouchXOffset(0);
	}

	if (ConfMan.hasKey("yoffset", "ds")) {
		DS::setTouchYOffset(ConfMan.getInt("yoffset", "ds"));
	} else {
		DS::setTouchXOffset(0);
	}
	
}

}

