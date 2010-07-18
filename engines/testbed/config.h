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

#ifndef TESTBED_CONFIG_H
#define TESTBED_CONFIG_H


#include "common/array.h"
#include "common/str-array.h"
#include "common/tokenizer.h"

#include "gui/ListWidget.h"
#include "gui/options.h"
#include "gui/ThemeEngine.h"

#include "testbed/testsuite.h"

namespace Testbed {

enum {
	kTestbedQuitCmd = 'Quit',
	kTestbedSelectAll = 'sAll'
};

class TestbedConfigManager {
public:
	TestbedConfigManager(Common::Array<Testsuite *> &tList) : _testsuiteList(tList) {}
	~TestbedConfigManager() {}
	void selectTestsuites();
	Testsuite *getTestsuiteByName(const Common::String &name);
	bool isEnabled(const Common::String &tsName);
private:
	Common::Array<Testsuite *> &_testsuiteList;
	void enableTestsuite(const Common::String &name, bool enable);
	void parseConfigFile() {}
};

class TestbedListWidget : public GUI::ListWidget {
public:
	TestbedListWidget(GUI::Dialog *boss, const Common::String &name, Common::StringArray &tsDescArr) : GUI::ListWidget(boss, name), _testSuiteDescArray(tsDescArr) {}

	void changeColor() {
		// Using Font Color Mechanism to highlight selected entries.
		// Might not be detectable with some themes
		if (_listColors.size() >= 2) {
			if (GUI::ThemeEngine::kFontColorNormal == _listColors[_selectedItem]) {
				_listColors[_selectedItem] = GUI::ThemeEngine::kFontColorAlternate;
			} else if (GUI::ThemeEngine::kFontColorAlternate == _listColors[_selectedItem]) {
				_listColors[_selectedItem] = GUI::ThemeEngine::kFontColorNormal;
			}
		}

		// Also append (selected) to each selected entry
		if (_list[_selectedItem].contains("selected")) {
			_list[_selectedItem] = _testSuiteDescArray[_selectedItem];	
		} else {
			_list[_selectedItem] += " (selected)";
		}
		draw();
	}

	void setColorAll(GUI::ThemeEngine::FontColor color) {
		for (uint i = 0; i < _listColors.size(); i++) {
			_listColors[i] = color;
			_list[i] += " (selected)";
		}
		draw();;
	}
	
	void setColor(uint32 indx, GUI::ThemeEngine::FontColor color) {
		assert(indx < _listColors.size());
		_listColors[indx] = color;
		draw();
	}

private:
	const Common::StringArray	_testSuiteDescArray;
};

class TestbedOptionsDialog : public GUI::Dialog {
public:
	TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan);
	~TestbedOptionsDialog();
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

private:
	GUI::ListWidget::ColorList _colors;
	Common::StringArray _testSuiteArray;
	Common::StringArray _testSuiteDescArray;
	TestbedListWidget *_testListDisplay;
	TestbedConfigManager *_testbedConfMan;
};

} // End of namespace Testbed

#endif // TESTBED_CONFIG_H
