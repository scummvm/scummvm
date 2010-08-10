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

#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "testbed/midi.h"

namespace Testbed {

TestExitStatus MidiTests::playMidiMusic() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	// Create a driver instance
	MidiDriver *driver = MidiDriver::createMidi(dev);
	// Create a SMF parser
	MidiParser *smfParser = MidiParser::createParser_SMF();
	smfParser->setMidiDriver(driver);
	smfParser->setTimerRate(driver->getBaseTempo());
	// Open the driver
	int errCode = driver->open();

	if (errCode) {
		Testsuite::logPrintf("Error! %s", driver->getErrorName(errCode));
		return kTestFailed;
	}
	
	Testsuite::logDetailedPrintf("Midi: Succesfully opened the driver\n");
	return kTestPassed;
}

MidiTestSuite::MidiTestSuite() {
	addTest("MidiTests", &MidiTests::playMidiMusic);
}

}
