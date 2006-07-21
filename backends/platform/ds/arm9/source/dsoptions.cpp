/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 Neil Millstone
 * Copyright (C) 2006 The ScummVM project
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

DSOptionsDialog::DSOptionsDialog() : GUI::Dialog(20, 20, 320 - 40, 200 - 40) {
	addButton(this, 10, 140, "Close", GUI::kCloseCmd, 'C');
	
#ifdef DS_SCUMM_BUILD
	if (!DS::isGBAMPAvailable()) {
//		addButton(this, 100, 140, "Delete Save", 'dels', 'D');
	}
#endif

	new GUI::StaticTextWidget(this, 0, 10, 280, 20, "ScummVM DS Options", GUI::kTextAlignCenter);

	_leftHandedCheckbox = new GUI::CheckboxWidget(this, 20, 30, 280, 20, "Left handed mode", 0, 'L');
	_indyFightCheckbox = new GUI::CheckboxWidget(this, 20, 50, 280, 20, "Indy fighting controls", 0, 'I');
	_unscaledCheckbox = new GUI::CheckboxWidget(this, 20, 70, 280, 20, "Unscaled lower screen", 0, 'S');

	new GUI::StaticTextWidget(this, 20, 90, 110, 20, "Touch X Offset", GUI::kTextAlignLeft);
	_touchX = new GUI::SliderWidget(this, 130, 90, 130, 12, 1);
	_touchX->setMinValue(-8);
	_touchX->setMaxValue(+8);
	_touchX->setValue(0);
	_touchX->setFlags(GUI::WIDGET_CLEARBG);

	new GUI::StaticTextWidget(this, 20, 110, 110, 20, "Touch Y Offset", GUI::kTextAlignLeft);
	_touchY = new GUI::SliderWidget(this, 130, 110, 130, 12, 2);
	_touchY->setMinValue(-8);
	_touchY->setMaxValue(+8);
	_touchY->setValue(0);
	_touchY->setFlags(GUI::WIDGET_CLEARBG);

	new GUI::StaticTextWidget(this, 130 + 65 - 10, 130, 20, 20, "0", GUI::kTextAlignCenter);
	new GUI::StaticTextWidget(this, 130 + 130 - 10, 130, 20, 20, "8", GUI::kTextAlignCenter);
	new GUI::StaticTextWidget(this, 130 - 10, 130, 20, 20, "-8", GUI::kTextAlignCenter);

#ifdef DS_SCUMM_BUILD
	_delDialog = new Scumm::SaveLoadChooser("Delete game:", "Delete", false, Scumm::g_scumm);
#endif

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
	ConfMan.setInt("xoffset", _touchX->getValue(), "ds");
	ConfMan.setInt("yoffset", _touchY->getValue(), "ds");
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
		
		OSystem::Event event;
		event.type = OSystem::EVENT_KEYDOWN;
		event.kbd.ascii = SDLK_DOWN;
		event.kbd.keycode = SDLK_DOWN;
		OSystem_DS::instance()->addEvent(event);

		event.type = OSystem::EVENT_KEYUP;
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

void showOptionsDialog() {
	OSystem_DS* system = OSystem_DS::instance();

	OSystem::Event event;
	event.type = OSystem::EVENT_KEYDOWN;
	event.kbd.keycode = 'P';		// F5
	event.kbd.ascii = 'P';
	event.kbd.flags = 0;
	system->addEvent(event);

	DS::displayMode16Bit();
	

	DSOptionsDialog* d = new DSOptionsDialog();
	d->runModal();
	delete d;
	
	
	DS::displayMode8Bit();

	event.type = OSystem::EVENT_KEYDOWN;
	event.kbd.keycode = 'P';		// F5
	event.kbd.ascii = 'P';
	event.kbd.flags = 0;
	system->addEvent(event);
}

void setOptions() {
	ConfMan.addGameDomain("ds");

	if (ConfMan.hasKey("lefthanded", "ds")) {
		DS::setLeftHanded(ConfMan.getBool("lefthanded", "ds"));
	} else {
		DS::setLeftHanded(false);
	}

	if (ConfMan.hasKey("unscaled", "ds")) {
		DS::setUnscaledMode(ConfMan.getBool("unscaled", "ds"));
	} else {
		DS::setUnscaledMode(false);
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

