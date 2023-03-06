/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "common/stream.h"
#include "common/config-manager.h"

#include "engines/engine.h"

#include "testbed/config.h"
#include "testbed/fs.h"

#include "gui/widgets/scrollcontainer.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"

namespace Testbed {

TestbedOptionsDialog::TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan) :
	GUI::Dialog(0, 0, 0, 0),
		_testbedConfMan(tsConfMan) {
	_testContainerDisplay = new GUI::ScrollContainerWidget(this, 0, 0, 0, 0);
	_testContainerDisplay->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);

	// Construct a String Array
	Common::Array<Testsuite *>::const_iterator iter;
	Common::U32String description;
	uint selected = 0;
	for (iter = tsList.begin(); iter != tsList.end(); iter++) {
		_testSuiteArray.push_back(*iter);
		description = (*iter)->getDescription();
		GUI::CheckboxWidget *checkbox;
		checkbox = new GUI::CheckboxWidget(_testContainerDisplay, 0, 0, 0, 0, description, Common::U32String());
		if ((*iter)->isEnabled()) {
			checkbox->setState(true);
			selected++;
		} else {
			checkbox->setState(false);
		}
		_testSuiteCheckboxArray.push_back(checkbox);
	}

	_messageText = new GUI::StaticTextWidget(this, 0, 0, 0, 0, Common::U32String("Select Testsuites to Execute"), Graphics::kTextAlignLeft);

	if (selected > (tsList.size() - selected)) {
		_selectButton = new GUI::ButtonWidget(this, 0, 0, 0, 0, Common::U32String("Deselect All"), Common::U32String(), kTestbedDeselectAll, 0);
	} else {
		_selectButton = new GUI::ButtonWidget(this, 0, 0, 0, 0, Common::U32String("Select All"), Common::U32String(), kTestbedSelectAll, 0);
	}
	_runTestButton = new GUI::ButtonWidget(this, 0, 0, 0, 0, Common::U32String("Run tests"), Common::U32String(), GUI::kCloseCmd);
	_quitButton = new GUI::ButtonWidget(this, 0, 0, 0, 0, Common::U32String("Exit Testbed"), Common::U32String(), kTestbedQuitCmd);

	reflowLayout();
}

TestbedOptionsDialog::~TestbedOptionsDialog() {}

void TestbedOptionsDialog::reflowLayout() {

	// Calculate all sizes for widgets
	uint16 lineHeight = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int16 overlayWidth = g_system->getOverlayWidth();
	int16 overlayHeight = g_system->getOverlayHeight();
	uint16 yPos = lineHeight * 2;
	uint16 xPos = lineHeight;
	uint16 padding = 32;

	// handle the low res
	if (overlayHeight < 500) {
		xPos = xPos / 2;
		padding = 16;
	}

	uint16 buttonHeight = lineHeight * 2;
	uint16 buttonWidth = lineHeight * 5;
	uint16 dialogWidth = overlayWidth - padding * 2;
	uint16 dialogHeight = overlayHeight - padding * 2;
	uint16 buttonPosY = dialogHeight - buttonHeight - lineHeight;
	uint16 containerWidth = dialogWidth - padding;
	uint16 containerHeight = dialogHeight - padding - lineHeight * 4 - buttonHeight;

	this->resize(padding, padding, dialogWidth, dialogHeight, false);

	_testContainerDisplay->setSize(containerWidth, containerHeight);
	_testContainerDisplay->setPos(0, lineHeight * 3);

	for (auto &iter : _testSuiteCheckboxArray) {
		iter->setPos(xPos, (&iter - _testSuiteCheckboxArray.begin()) * lineHeight * 2);
		iter->setSize(containerWidth - padding - xPos, lineHeight * 1.5f);
	}

	_messageText->setPos(xPos, lineHeight);
	_messageText->setSize(containerWidth - padding - xPos, lineHeight);
	_selectButton->setPos(xPos, buttonPosY);
	_selectButton->setSize(buttonWidth, buttonHeight);
	_runTestButton->setPos(dialogWidth - padding * 2 - buttonWidth * 2, buttonPosY);
	_runTestButton->setSize(buttonWidth, buttonHeight);
	_quitButton->setPos(dialogWidth - padding - buttonWidth, buttonPosY);
	_quitButton->setSize(buttonWidth, buttonHeight);

	Dialog::reflowLayout();
}

void TestbedOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	Testsuite *ts;
	Common::WriteStream *ws;
	switch (cmd) {
	case kTestbedQuitCmd:
		Engine::quitGame();
		close();
		break;

	case kTestbedDeselectAll:
		_selectButton->setLabel("Select All");
		_selectButton->setCmd(kTestbedSelectAll);
		for (uint i = 0; i < _testSuiteArray.size(); i++) {
			_testSuiteCheckboxArray[i]->setState(false);
		}
		break;

	case kTestbedSelectAll:
		_selectButton->setLabel("Deselect All");
		_selectButton->setCmd(kTestbedDeselectAll);
		for (uint i = 0; i < _testSuiteArray.size(); i++) {
			_testSuiteCheckboxArray[i]->setState(true);
		}
		break;

	case GUI::kCloseCmd:
		// This is final selected state, write it to config file.
		for (uint i = 0; i < _testSuiteCheckboxArray.size(); i++) {
			ts = _testSuiteArray[i];
			if (_testSuiteCheckboxArray[i]->getState()) {
				if (ts) {
					ts->enable(true);
				}
			} else {
				if (ts) {
					ts->enable(false);
				}
			}
		}
		ws = _testbedConfMan->getConfigWriteStream();
		if (ws) {
			_testbedConfMan->writeTestbedConfigToStream(ws);
			delete ws;
		}
		break;

	default:
		break;
	}

	GUI::Dialog::handleCommand(sender, cmd, data);
}

void TestbedInteractionDialog::addText(uint w, uint h, const Common::String text, Graphics::TextAlign textAlign, uint xOffset, uint yPadding) {
	if (!xOffset) {
		xOffset = _xOffset;
	}
	_yOffset += yPadding;
	GUI::StaticTextWidget *widget = new GUI::StaticTextWidget(this, xOffset, _yOffset, w, h, text, textAlign);
	widget->resize(xOffset, _yOffset, w, h);
	_yOffset += h;
}

void TestbedInteractionDialog::addButton(uint w, uint h, const Common::String name, uint32 cmd, uint xOffset, uint yPadding) {
	if (!xOffset) {
		xOffset = _xOffset;
	}
	_yOffset += yPadding;
	_buttonArray.push_back(new GUI::ButtonWidget(this, xOffset, _yOffset, w, h, name, Common::U32String(), cmd));
	_buttonArray.back()->resize(xOffset, _yOffset, w, h);
	_yOffset += h;
}

void TestbedInteractionDialog::addList(uint x, uint y, uint w, uint h, const Common::Array<Common::U32String> &strArray, uint yPadding) {
	_yOffset += yPadding;
	GUI::ListWidget *list = new GUI::ListWidget(this, x, y, w, h);
	list->resize(x, y, w, h);
	list->setEditable(false);
	list->setNumberingMode(GUI::kListNumberingOff);
	list->setList(strArray);
	_yOffset += h;
}

void TestbedInteractionDialog::addButtonXY(uint x, uint y, uint w, uint h, const Common::String name, uint32 cmd) {
	_buttonArray.push_back(new GUI::ButtonWidget(this, x, _yOffset, w, h, name, Common::U32String(), cmd));
	_buttonArray.back()->resize(x, y, w, h);
}

void TestbedInteractionDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	GUI::Dialog::handleCommand(sender, cmd, data);
}

void TestbedConfigManager::initDefaultConfiguration() {
	// Default Configuration
	// Add Global configuration Parameters here.
	_configFileInterface.setKey("isSessionInteractive", "Global", "true");
}

void TestbedConfigManager::writeTestbedConfigToStream(Common::WriteStream *ws) {
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

Common::SeekableReadStream *TestbedConfigManager::getConfigReadStream() const {
	// Look for config file using SearchMan
	Common::SeekableReadStream *rs = SearchMan.createReadStreamForMember(_configFileName);
	return rs;
}

Common::WriteStream *TestbedConfigManager::getConfigWriteStream() const {
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
	Common::INIFile::SectionList sections = _configFileInterface.getSections();
	Testsuite *currTS = 0;

	for (Common::INIFile::SectionList::const_iterator i = sections.begin(); i != sections.end(); i++) {
		if (i->name.equalsIgnoreCase("Global")) {
			// Global params may be directly queried, ignore them
		} else {
			// A testsuite, process it.
			currTS = getTestsuiteByName(i->name);
			Common::INIFile::SectionKeyList kList = i->getKeys();
			if (!currTS) {
				Testsuite::logPrintf("Warning! Error in config: Testsuite %s not found\n", i->name.c_str());
				continue;
			}

			for (Common::INIFile::SectionKeyList::const_iterator j = kList.begin(); j != kList.end(); j++) {
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
		ConfParams.setSessionAsInteractive(stringToBool(in));
	}

	if (!ConfParams.isSessionInteractive()) {
		// Non interactive sessions don't need to go beyond
		return;
	}

	// XXX: disabling these as of now for fastly testing other tests
	// Testsuite::isSessionInteractive = false;
	Common::String prompt("Welcome to the ScummVM testbed!\n"
						"It is a framework to test the various ScummVM subsystems namely GFX, Sound, FS, events etc.\n"
						"If you see this, it means interactive tests would run on this system :)\n");

	if (!ConfParams.isGameDataFound()) {
		prompt += "\nSeems like Game data files are not configured properly.\n"
		"Create Game data files using script ./create-testbed-data.sh in dists/engine-data\n"
		"Next, Configure the game path in launcher / command-line.\n"
		"Currently a few testsuites namely FS/AudioCD/MIDI would be disabled\n";
	}

	Testsuite::logPrintf("Info! : Interactive tests are also being executed.\n");

	if (Testsuite::handleInteractiveInput(prompt, "Proceed?", "Customize", kOptionRight)) {
		if (Engine::shouldQuit()) {
			return;
		}
		// Select testsuites using checkboxes
		TestbedOptionsDialog tbd(_testsuiteList, this);
		tbd.runModal();
	}

	// Clear it to remove entries before next rerun
	_configFileInterface.clear();
}

} // End of namespace Testbed
