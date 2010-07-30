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

TestbedOptionsDialog::TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan) : GUI::Dialog("Browser"), _testbedConfMan(tsConfMan) {
	
	new GUI::StaticTextWidget(this, "Browser.Headline", "Select Testsuites to Execute");
	new GUI::StaticTextWidget(this, "Browser.Path", "Use Doubleclick to select/deselect");

	// Construct a String Array
	Common::Array<Testsuite *>::const_iterator iter;
	Common::String description;
	uint selected = 0;

	for (iter = tsList.begin(); iter != tsList.end(); iter++) {
		_testSuiteArray.push_back(*iter);
		description = (*iter)->getDescription();
		if ((*iter)->isEnabled()) {
			_testSuiteDescArray.push_back(description + "(selected)");
			selected++;
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

	if (selected > (tsList.size() - selected)) {
		_selectButton = new GUI::ButtonWidget(this, "Browser.Up", "Deselect All", kTestbedDeselectAll, 0);
	} else {
		_selectButton = new GUI::ButtonWidget(this, "Browser.Up", "Select All", kTestbedSelectAll, 0);
	}
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

void TestbedInteractionDialog::addText(uint w, uint h, const Common::String text, Graphics::TextAlign textAlign, uint xOffset, uint yPadding) {
	if (!xOffset) {
		xOffset = _xOffset;
	}
	_yOffset += yPadding;
	new GUI::StaticTextWidget(this, xOffset, _yOffset, w, h, text, textAlign);
	_yOffset += h;
}

void TestbedInteractionDialog::addButton(uint w, uint h, const Common::String name, uint32 cmd, uint xOffset, uint yPadding) {
	if (!xOffset) {
		xOffset = _xOffset;
	}
	_yOffset += yPadding;
	_buttonArray.push_back(new GUI::ButtonWidget(this, xOffset, _yOffset, w, h, name, cmd));
	_yOffset += h;
}

void TestbedInteractionDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void TestbedConfigManager::initDefaultConfiguration() {
	// Default Configuration
	// Add Global configuration Parameters here.
	_configFileInterface.setKey("isSessionInteractive", "Global", "true");
}

void TestbedConfigManager::writeTestbedConfigToStream(Common::WriteStream *ws) {
	Common::String wStr;
	for (Common::Array<Testsuite *>::const_iterator i = _testsuiteList.begin(); i < _testsuiteList.end(); i++) {
		_configFileInterface.setKey("this", (*i)->getName(), boolToString((*i)->isEnabled()));
		const Common::Array<Test *> &testList = (*i)->getTestList();
		for (Common::Array<Test *>::const_iterator j = testList.begin(); j != testList.end(); j++) {
			_configFileInterface.setKey((*j)->featureName, (*i)->getName(), boolToString((*j)->enabled));
		}
	}
	_configFileInterface.saveToStream(*ws);
	ws->flush();
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
	Common::WriteStream *ws;
	Common::FSNode gameRoot(path);
	Common::FSNode config = gameRoot.getChild(_configFileName);
	ws = config.createWriteStream();
	return ws;
}

void TestbedConfigManager::parseConfigFile() {	
	Common::SeekableReadStream *rs = getConfigReadStream();
	
	if (!rs) {
		Testsuite::logPrintf("Info! No config file found, using default configuration.\n");
		initDefaultConfiguration();
		return;
	}
	_configFileInterface.loadFromStream(*rs);
	Common::ConfigFile::SectionList sections = _configFileInterface.getSections();
	Testsuite *currTS = 0;

	for (Common::ConfigFile::SectionList::const_iterator i = sections.begin(); i != sections.end(); i++) {
		if (i->name.equalsIgnoreCase("Global")) {
			// Global params may be directly queried, ignore them	
		} else {
			// A testsuite, process it.
			currTS = getTestsuiteByName(i->name);
			Common::ConfigFile::SectionKeyList kList = i->getKeys();
			if (!currTS) {
				Testsuite::logPrintf("Warning! Error in config: Testsuite %s not found\n", i->name.c_str());
			}

			for (Common::ConfigFile::SectionKeyList::const_iterator j = kList.begin(); j != kList.end(); j++) {
				if (j->key.equalsIgnoreCase("this")) {
					currTS->enable(stringToBool(j->value));
				} else {
					if (!currTS->enableTest(j->key, stringToBool(j->value))) {
						Testsuite::logPrintf("Warning! Error in config: Test %s not found\n", j->key.c_str());
					}
				}
			}
		}
	}
	delete rs;
}

int TestbedConfigManager::getNumSuitesEnabled() {
	int count = 0;
	for (uint i = 0; i < _testsuiteList.size(); i++) {
        if (_testsuiteList[i]->isEnabled()) {
        	count++;
		}
    }
    return count;
}

Testsuite *TestbedConfigManager::getTestsuiteByName(const Common::String &name) {
	for (uint i = 0; i < _testsuiteList.size(); i++) {
		if (name.equalsIgnoreCase(_testsuiteList[i]->getName())) {
			return _testsuiteList[i];
		}
	}
	return 0;
}

void TestbedConfigManager::selectTestsuites() {

	parseConfigFile();
	
	if (_configFileInterface.hasKey("isSessionInteractive", "Global")) {
		Common::String in;
		_configFileInterface.getKey("isSessionInteractive", "Global", in);
		Testsuite::isSessionInteractive = stringToBool(in);
	}

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
