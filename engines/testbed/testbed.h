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
 */

#ifndef TESTBED_TESTBED_H
#define TESTBED_TESTBED_H

#include "common/array.h"

#include "engines/engine.h"

#include "testbed/config.h"
#include "testbed/testsuite.h"

namespace Testbed {

class TestbedConfigManager;

enum {
	kTestbedLogOutput = 1 << 0,
	kTestbedEngineDebug = 1 << 2,
	kCmdRerunTestbed = 'crtb'
};

class TestbedEngine : public Engine {
public:
	TestbedEngine(OSystem *syst);
	~TestbedEngine() override;

	Common::Error run() override;

	/**
	 * Invokes configured testsuites.
	 */
	static void pushTestsuites(Common::Array<Testsuite *> &testsuiteList);

	/**
	 * Invokes configured testsuites.
	 */
	void invokeTestsuites(TestbedConfigManager &cfMan);

	bool hasFeature(EngineFeature f) const override;

private:
	void checkForAllAchievements();
	void videoTest();

	Common::Array<Testsuite *> _testsuiteList;
};

class TestbedExitDialog : public TestbedInteractionDialog {
public:
	TestbedExitDialog(Common::Array<Testsuite *> &testsuiteList) : TestbedInteractionDialog(80, 40, 500, 330),
	_testsuiteList(testsuiteList) {}
	~TestbedExitDialog() override {}
	void init();
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void run() { runModal(); }
private:
	Common::Array<Testsuite *> &_testsuiteList;
};

} // End of namespace Testbed

#endif // TESTBED_TESTBED_H
