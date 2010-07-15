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

#include "testbed/testsuite.h"
#include "common/array.h"
#include "gui/options.h"

namespace Testbed {

class TestbedConfigManager {
public:
	TestbedConfigManager(Common::Array<Testsuite *> &tList) : _testsuiteList(tList) {}
	~TestbedConfigManager() {}
	void selectTestsuites();
private:
	Common::Array<Testsuite *> &_testsuiteList;
	void enableTestsuite(const Common::String &name, bool enable);
	void parseConfigFile() {}
};

class TestbedOptionsDialog : public GUI::OptionsDialog {
public:
	TestbedOptionsDialog();
	~TestbedOptionsDialog();
	void addCheckbox(const Common::String &tsName);
	bool isEnabled(const Common::String &tsName);

private:
	Common::Array<GUI::CheckboxWidget *> _checkBoxes;
	const int _hOffset; // current offset from left
	int _vOffset; // current offset from top
	const int _boxWidth;
	const int _boxHeight;
};

} // End of namespace Testbed

#endif // TESTBED_CONFIG_H
