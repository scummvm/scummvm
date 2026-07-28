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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/input.h"

#include <cxxtest/TestSuite.h>

class RipperInputTestSuite : public CxxTest::TestSuite {
public:
	void testFunctionKeysUseBiosCommands() {
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(Common::KeyState(Common::KEYCODE_F1)), 0x3b00);
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(Common::KeyState(Common::KEYCODE_F5)), 0x3f00);
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(Common::KeyState(Common::KEYCODE_F10)), 0x4400);
	}

	void testNavigationKeycodeOverridesBackendAscii() {
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(
			Common::KeyState(Common::KEYCODE_RIGHT, 0x0113)), 0x4d00);
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(
			Common::KeyState(Common::KEYCODE_DELETE, 0x7f)), 0x5300);
	}

	void testAltHUsesBiosPreviewCommand() {
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(
			Common::KeyState(Common::KEYCODE_h, 'h', Common::KBD_ALT)),
			0x2300);
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(
			Common::KeyState(Common::KEYCODE_h, 'h')), (uint16)'h');
	}

	void testPrintableAndControlKeysRetainAscii() {
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(
			Common::KeyState(Common::KEYCODE_a, 'a')), (uint16)'a');
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(
			Common::KeyState(Common::KEYCODE_ESCAPE, Common::ASCII_ESCAPE)),
			(uint16)Common::ASCII_ESCAPE);
		TS_ASSERT_EQUALS(Ripper::translateKeyToCommand(Common::KeyState()), 0);
	}
};
