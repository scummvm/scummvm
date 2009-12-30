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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "mohawk/mohawk.h"
#include "mohawk/myst.h"
#include "mohawk/riven.h"
#include "mohawk/dialogs.h"

#include "gui/GuiManager.h"
#include "common/savefile.h"

namespace Mohawk {

// This used to have GUI::Dialog("MohawkDummyDialog"), but that doesn't work with the gui branch merge :P (Sorry, Tanoku!)
InfoDialog::InfoDialog(MohawkEngine *vm, Common::String message) : _vm(vm), GUI::Dialog(0, 0, 1, 1), _message(message) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
	
	_text = new GUI::StaticTextWidget(this, 4, 4, 10, 10, _message, Graphics::kTextAlignCenter);
}

void InfoDialog::setInfoText(Common::String message) {
	_message = message;
	_text->setLabel(_message);
}

void InfoDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();
	
	int width = g_gui.getStringWidth(_message) + 16;
	int height = g_gui.getFontHeight() + 8;
	
	_w = width;
	_h = height;
	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;
	
	_text->setSize(_w - 8, _h);
}

PauseDialog::PauseDialog(MohawkEngine *vm, Common::String message) : InfoDialog(vm, message) {
}

void PauseDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == ' ')
		close();
	else
		InfoDialog::handleKeyDown(state);
}

enum {
	kCloseCmd = 'CLOS',
	kZipCmd = 'ZIPM',
	kTransCmd = 'TRAN',
	kWaterCmd = 'WATR'
};

MystOptionsDialog::MystOptionsDialog(MohawkEngine_Myst* vm) : GUI::OptionsDialog("", 120, 120, 360, 200), _vm(vm) {	
	_zipModeCheckbox = new GUI::CheckboxWidget(this, 15, 10, 300, 15, "Zip Mode Activated", kZipCmd, 'Z');
	_transistionsCheckbox = new GUI::CheckboxWidget(this, 15, 30, 300, 15, "Transistions Enabled", kTransCmd, 'T');

	new GUI::ButtonWidget(this, 95, 160, 120, 25, "OK", GUI::OptionsDialog::kOKCmd, 'O');
	new GUI::ButtonWidget(this, 225, 160, 120, 25, "Cancel", kCloseCmd, 'C');
}

MystOptionsDialog::~MystOptionsDialog() {
}

void MystOptionsDialog::open() {
	Dialog::open();

	_zipModeCheckbox->setState(_vm->_zipMode);
	_transistionsCheckbox->setState(_vm->_transitionsEnabled);
}

void MystOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kZipCmd:
		_vm->_zipMode = _zipModeCheckbox->getState();
		break;
	case kTransCmd:
		_vm->_transitionsEnabled = _transistionsCheckbox->getState();
		break;
	case kCloseCmd:
		close();
		break;
	default:
		GUI::OptionsDialog::handleCommand(sender, cmd, data);
	}
}

RivenOptionsDialog::RivenOptionsDialog(MohawkEngine_Riven* vm) : GUI::OptionsDialog("", 120, 120, 360, 200), _vm(vm) {	
	_zipModeCheckbox = new GUI::CheckboxWidget(this, 15, 10, 300, 15, "Zip Mode Activated", kZipCmd, 'Z');
	_waterEffectCheckbox = new GUI::CheckboxWidget(this, 15, 30, 300, 15, "Water Effect Enabled", kWaterCmd, 'W');

	new GUI::ButtonWidget(this, 95, 160, 120, 25, "OK", GUI::OptionsDialog::kOKCmd, 'O');
	new GUI::ButtonWidget(this, 225, 160, 120, 25, "Cancel", kCloseCmd, 'C');
}

RivenOptionsDialog::~RivenOptionsDialog() {
}

void RivenOptionsDialog::open() {
	Dialog::open();

	_zipModeCheckbox->setState(*_vm->matchVarToString("azip") != 0);
	_waterEffectCheckbox->setState(*_vm->matchVarToString("waterenabled") != 0);
}

void RivenOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kZipCmd:
		*_vm->matchVarToString("azip") = _zipModeCheckbox->getState() ? 1 : 0;
		break;
	case kWaterCmd:
		*_vm->matchVarToString("waterenabled") = _waterEffectCheckbox->getState() ? 1 : 0;
		break;
	case kCloseCmd:
		close();
		break;
	default:
		GUI::OptionsDialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace Mohawk
