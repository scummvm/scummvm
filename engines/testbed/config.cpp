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

#include "common/fs.h"
#include "common/stream.h"
#include "common/config-manager.h"
#include "engines/engine.h"
#include "testbed/config.h"

namespace Testbed {

/**
 * Stores testbed setting to be accessed/modified from config file.
 * As of now there is only one entry "isSessionInteractive"
 */
struct TestbedSettings {
	const char *name;
	bool value;
} testbedSettings[] = {
	{"isSessionInteractive", true}
};

TestbedOptionsDialog::TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan) : GUI::Dialog("Browser"), _testbedConfMan(tsConfMan) {
	
	new GUI::StaticTextWidget(this, "Browser.Headline", "Select Testsuites to Execute");
	new GUI::StaticTextWidget(this, "Browser.Path", "Use Doubleclick to select/deselect");

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
	new GUI::ButtonWidget(this, "Browser.Cancel", "Run tests", GUI::kCloseCmd);
	new GUI::ButtonWidget(this, "Browser.Choose", "Exit Testbed", kTestbedQuitCmd);
}

TestbedOptionsDialog::~TestbedOptionsDialog() {}

void TestbedOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	Testsuite *ts;
	Common::WriteStream *ws;
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
	case GUI::kCloseCmd:
		// This is final selected state, write it to config file.
		ws = _testbedConfMan->getConfigWriteStream();
		_testbedConfMan->writeTestbedConfigToStream(ws);
		delete ws;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
	
	}
}

void TestbedConfigManager::writeTestbedConfigToStream(Common::WriteStream *ws) {
	Common::String wStr;
	for (Common::Array<Testsuite *>::const_iterator i = _testsuiteList.begin(); i < _testsuiteList.end(); i++) {
		// Construct the string
		wStr = "[";
		wStr += (*i)->getName();
		wStr += " = ";
		wStr += (*i)->isEnabled() ? "true" : "false";
		wStr += "]\n";
		ws->writeString(wStr);
		// TODO : for every test
		const Common::Array<Test *> &testList = (*i)->getTestList();
		for (Common::Array<Test *>::const_iterator j = testList.begin(); j != testList.end(); j++) {
			wStr = (*j)->featureName;
			wStr += " = ";
			wStr += (*j)->enabled ? "true\n": "false\n";
			ws->writeString(wStr);
		}
		ws->writeString("\n");
	}
}

Common::SeekableReadStream *TestbedConfigManager::getConfigReadStream() {
	// Look for config file in game-path
	const Common::String &path = ConfMan.get("path");
	Common::FSDirectory gameRoot(path);
	Common::SeekableReadStream *rs = gameRoot.createReadStreamForMember(_configFileName);
	return rs;
}

Common::WriteStream *TestbedConfigManager::getConfigWriteStream() {
	// Look for config file in game-path
	const Common::String &path = ConfMan.get("path");
	Common::FSNode gameRoot(path);
	Common::FSNode config = gameRoot.getChild(_configFileName);
	if (!config.exists()) {
		Testsuite::logPrintf("Info! No config file found, creating new one");
	}
	return config.createWriteStream();
}

void TestbedConfigManager::editSettingParam(Common::String param, bool value) {
	for (int i = 0; i < ARRAYSIZE(testbedSettings); i++) {
		if (param.equalsIgnoreCase(testbedSettings[i].name)) {
			testbedSettings[i].value = value;
		}
	}
}

void TestbedConfigManager::parseConfigFile() {	
	Common::SeekableReadStream *rs = getConfigReadStream();
	if (!rs) {
		Testsuite::logPrintf("Info! No config file found, using default configuration.\n");
		return;
	}
	Testsuite *currTS = 0;
	bool globalSettings = true;

	int lineno = 0;
	while (!rs->eos() && !rs->err()) {
		Common::String line = rs->readLine();
		lineno++;
		// Trim leading / trailing whitespaces
		line.trim();

		if (0 ==line.size() || '#' == line[0]) {
			// Skip blank lines and comments
			continue;
		}

		if (line.contains("[Global]") || line.contains("[global]")) {
			// Global settings.
			globalSettings = true;
			continue;
		}

		if (globalSettings) {
			const char* t = line.c_str();
			const char *p = strchr(t, '=');
			Common::String key(t, p);
			Common::String value(p + 1);
			// trim both of spaces
			key.trim();
			value.trim();
			if (value.equalsIgnoreCase("true")) {
				editSettingParam(key, true);
			} else {
				editSettingParam(key, false);
			}
			continue;
		}

		// Check testsuites first
		if ('[' == line[0]) {
			// This is a testsuite, extract key value
			const char* t = line.c_str() + 1;
			const char *p = strchr(t, '=');
			
			if (!p) {
				Testsuite::logPrintf("Error! Parsing : Malformed config file, token '=' missing at line %d\n", lineno);
				break;
			}

			Common::String tsName(t, p);
			Common::String toEnable(p + 1);
			// trim both of spaces
			tsName.trim();
			toEnable.trim();
			
			currTS = getTestsuiteByName(tsName);
			globalSettings = false;
			if (!currTS) {	
				Testsuite::logPrintf("Error! Parsing : Unrecognized testsuite name at line %d\n", lineno);
				break;
			}
			toEnable.toLowercase();
			if (toEnable.contains("true")) {
				currTS->enable(true);
			} else {
				currTS->enable(false);
			}
		} else {
			// A test under "currTS" testsuite
			if (!currTS) {
				Testsuite::logPrintf("Error! Parsing : Malformed config file, No testsuite corresponding to test at line %d\n", lineno);
				break;
			}
			// Extract key value
			const char* t = line.c_str();
			const char *p = strchr(t, '=');
			Common::String key(t, p);
			Common::String value(p + 1);
			// trim both of spaces
			key.trim();
			value.trim();
			bool isValid = true;
			if (value.equalsIgnoreCase("true")) {
				isValid = currTS->enableTest(key, true);
			} else {
				isValid = currTS->enableTest(key, false);
			}
			if (!isValid) {
				Testsuite::logPrintf("Error! Parsing : Unrecognized test for testsuite %s at line %d\n", currTS->getName(), lineno);
			}
		}
	}
	delete rs;
}

Testsuite *TestbedConfigManager::getTestsuiteByName(const Common::String &name) {
	for (uint i = 0; i < _testsuiteList.size(); i++) {
		if (name.equalsIgnoreCase(_testsuiteList[i]->getName())) {
			return _testsuiteList[i];
		}
	}
	return 0;
}

bool TestbedConfigManager::getConfigParamValue(const Common::String param) {
	for (int i = 0; i < ARRAYSIZE(testbedSettings); i++) {
		if (param.equalsIgnoreCase(testbedSettings[i].name)) {
			return testbedSettings[i].value;
		}
	}
	return false;
}

void TestbedConfigManager::selectTestsuites() {


	// Parse the config file
	// Enable testsuites as per configuration.
	// If no config file is found pickup a default configuration.
	// TODO : Implement this method

	parseConfigFile();
	Testsuite::isSessionInteractive = getConfigParamValue("isSessionInteractive");
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
