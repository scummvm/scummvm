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

#include "engines/engine.h"
#include "testbed/config.h"

namespace Testbed {

TestbedOptionsDialog::TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan) : GUI::Dialog("Browser") {
	
	new GUI::StaticTextWidget(this, "Browser.Headline", "Select Testsuites to Execute");
	new GUI::StaticTextWidget(this, "Browser.Path", "Use Double click to select / deselect");

	// Construct a String Array
	Common::Array<Testsuite *>::const_iterator iter;
	Common::String description;

	for (iter = tsList.begin(); iter != tsList.end(); iter++) {
		_testSuiteArray.push_back(*iter);
		description = (*iter)->getDescription();
		if ((*iter)->isEnabled()) {
			_testSuiteDescArray.push_back(description + "(selected)");
			_colors.push_back(GUI::ThemeEngine::kFontColorNormal);
		} else {
			_testSuiteDescArray.push_back(description);
			_colors.push_back(GUI::ThemeEngine::kFontColorAlternate);
		}
	}
	
	_testListDisplay = new TestbedListWidget(this, "Browser.List", _testSuiteArray);
	_testListDisplay->setNumberingMode(GUI::kListNumberingOff);
	_testListDisplay->setList(_testSuiteDescArray, &_colors);

	// This list shouldn't be editable
	_testListDisplay->setEditable(false);

	_selectButton = new GUI::ButtonWidget(this, "Browser.Up", "Deselect All", kTestbedDeselectAll, 0);
	new GUI::ButtonWidget(this, "Browser.Cancel", "Continue", GUI::kCloseCmd);
	new GUI::ButtonWidget(this, "Browser.Choose", "Exit Testbed", kTestbedQuitCmd);
}

TestbedOptionsDialog::~TestbedOptionsDialog() {}

void TestbedOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	Testsuite *ts;
	switch (cmd) {
	case GUI::kListItemDoubleClickedCmd:
		ts  = _testSuiteArray[_testListDisplay->getSelected()];
		if (ts) {
			if (ts->isEnabled()) {
				ts->enable(false);
				_testListDisplay->markAsDeselected(_testListDisplay->getSelected());
			} else {
				ts->enable(true);
				_testListDisplay->markAsSelected(_testListDisplay->getSelected());
			}
			ts->enable(!ts->isEnabled());
		}
		break;

	case kTestbedQuitCmd:
		Engine::quitGame();
		close();
		break;

	case kTestbedDeselectAll:
		_selectButton->setLabel("Select All");
		_selectButton->setCmd(kTestbedSelectAll);
		for (uint i = 0; i < _testSuiteArray.size(); i++) {
			_testListDisplay->markAsDeselected(i);
			ts  = _testSuiteArray[i];
			if (ts) {
				ts->enable(false);
			}
		}
		break;

	case kTestbedSelectAll:
		_selectButton->setLabel("Deselect All");
		_selectButton->setCmd(kTestbedDeselectAll);
		for (uint i = 0; i < _testSuiteArray.size(); i++) {
			_testListDisplay->markAsSelected(i);
			ts  = _testSuiteArray[i];
			if (ts) {
				ts->enable(true);
			}
		}
		break;

	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
	
	}
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
		TestbedOptionsDialog tbd(_testsuiteList, this);
		tbd.runModal();

		

	}
}

}	// End of namespace Testbed
