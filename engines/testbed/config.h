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
	kTestbedSelectAll = 'sAll',
	kTestbedDeselectAll = 'dAll'
};

class TestbedConfigManager {
public:
	TestbedConfigManager(Common::Array<Testsuite *> &tList) : _testsuiteList(tList) {}
	~TestbedConfigManager() {}
	void selectTestsuites();
private:
	Common::Array<Testsuite *> &_testsuiteList;
	void parseConfigFile() {}
};

class TestbedListWidget : public GUI::ListWidget {
public:
	TestbedListWidget(GUI::Dialog *boss, const Common::String &name, Common::Array<Testsuite *> tsArray) : GUI::ListWidget(boss, name), _testSuiteArray(tsArray) {}

	void markAsSelected(int i) {
		if (!_list[i].contains("selected")) {
			_list[i] += " (selected)";
		}
		_listColors[i] = GUI::ThemeEngine::kFontColorNormal;
		draw();
	}
	
	void markAsDeselected(int i) {
		if (_list[i].contains("selected")) {
			_list[i] = _testSuiteArray[i]->getDescription();
		}
		_listColors[i] = GUI::ThemeEngine::kFontColorAlternate;
		draw();
	}
	
	void setColor(uint32 indx, GUI::ThemeEngine::FontColor color) {
		assert(indx < _listColors.size());
		_listColors[indx] = color;
		draw();
	}

private:
	Common::Array<Testsuite *>	_testSuiteArray;
};

class TestbedOptionsDialog : public GUI::Dialog {
public:
	TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan);
	~TestbedOptionsDialog();
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

private:
	GUI::ListWidget::ColorList _colors;
	GUI::ButtonWidget	*_selectButton;
	Common::Array<Testsuite *> _testSuiteArray;
	Common::StringArray _testSuiteDescArray;
	TestbedListWidget *_testListDisplay;
};

} // End of namespace Testbed

#endif // TESTBED_CONFIG_H
