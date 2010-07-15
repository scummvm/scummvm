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
 * $URL$
 * $Id$
 */

#include "testbed/config.h"
#include "engines/engine.h"

namespace Testbed {

TestbedOptionsDialog::TestbedOptionsDialog() : GUI::OptionsDialog("Select", 120, 120, 360, 200), _hOffset(15), _vOffset(15), _boxWidth(300), _boxHeight(10) {
	new GUI::StaticTextWidget(this, _hOffset, _vOffset, _boxWidth, _boxHeight, "Select testsuites to Execute", Graphics::kTextAlignCenter);
	_vOffset += 20;
	addCheckbox("FS");
	addCheckbox("GFX");
	addCheckbox("Savegames");
	addCheckbox("Misc");
	addCheckbox("Events");
	new GUI::ButtonWidget(this, 80 , _vOffset + 10, 80, 25, "Continue", GUI::kOKCmd, 'C');
	new GUI::ButtonWidget(this, 200, _vOffset + 10, 80, 25, "Exit", GUI::kCloseCmd, 'X');
}

TestbedOptionsDialog::~TestbedOptionsDialog() {}

void TestbedOptionsDialog::addCheckbox(const Common::String &tsName) {
	_checkBoxes.push_back(new GUI::CheckboxWidget(this, _hOffset, _vOffset, _boxWidth, _boxHeight, tsName));
	_vOffset += 20;
}

bool TestbedOptionsDialog::isEnabled(const Common::String &tsName) {
	for (uint i = 0; i < _checkBoxes.size(); i++) {
		if (_checkBoxes[i]->getLabel().equalsIgnoreCase(tsName)) {
			return _checkBoxes[i]->getState();
		}
	}
	return false;
}

void TestbedConfigManager::enableTestsuite(const Common::String &name, bool enable) {
	Common::Array<Testsuite *>::const_iterator iter;

	for (iter = _testsuiteList.begin(); iter != _testsuiteList.end(); iter++) {
		if (name.equalsIgnoreCase((*iter)->getName())) {
			(*iter)->enable(enable);
			break;
		}
	}

	return;
}



void TestbedConfigManager::selectTestsuites() {


	// Parse the config file
	// Enable testsuites as per configuration.
	// If no config file is found pickup a default configuration.
	// TODO : Implement this method

	parseConfigFile();

	if (!Testsuite::isSessionInteractive) {
		// Non interactive sessions don't need to go beyond
		return;
	}

	// XXX: disabling these as of now for fastly testing other tests
	// Testsuite::isSessionInteractive = false;
	Common::String prompt("Welcome to the ScummVM testbed!\n"
						"It is a framework to test the various ScummVM subsystems namely GFX, Sound, FS, events etc.\n"
						"If you see this, it means interactive tests would run on this system :)");

	Testsuite::logPrintf("Info! : Interactive tests are also being executed.\n");
	
	if (Testsuite::handleInteractiveInput(prompt, "Proceed?", "Customize", kOptionRight)) {

		// Select testsuites using checkboxes
		TestbedOptionsDialog tbd;
		tbd.runModal();

		// check if user wanted to exit.
		if (Engine::shouldQuit()) {
			return;
		}

		// Enable selected testsuites
		Common::String tsName;
		for (uint i = 0; i < _testsuiteList.size(); i++) {
			tsName = _testsuiteList[i]->getName();
			if (tbd.isEnabled(tsName)) {
				enableTestsuite(tsName, true);
			}
		}

	}
}

}	// End of namespace Testbed
