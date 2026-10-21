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

#include "scooby/sound/gems_audio.h"

#include "scooby/scooby.h"
#include "scooby/sound/gems_driver.h"

namespace Scooby {

namespace {

// Ghidra RunStartupSequence 0x000008E2-0x000008F9 pushes the bank pointers for gemsInit
// (0x001F6EA8) in reverse argument order.
const uint32 kPatchBank = 0x001BD1D0;
const uint32 kEnvelopeBank = 0x001BE31F;
const uint32 kSequenceBank = 0x001BE496;
const uint32 kSampleBank = 0x001C70C5;

GemsDriver *getDriver() {
	return g_engine ? g_engine->getGemsDriver() : nullptr;
}

} // namespace

void gemsInit() {
	if (GemsDriver *driver = getDriver()) {
		driver->init(kPatchBank, kEnvelopeBank, kSequenceBank, kSampleBank);
	}
}

void gemsStartSong(int song) {
	if (GemsDriver *driver = getDriver()) {
		driver->startSong(static_cast<uint8>(song));
	}
}

void gemsStopSong(int song) {
	if (GemsDriver *driver = getDriver()) {
		driver->stopSong(static_cast<uint8>(song));
	}
}

void gemsStopAll() {
	if (GemsDriver *driver = getDriver()) {
		driver->stopAll();
	}
}

void gemsPauseAll() {
	if (GemsDriver *driver = getDriver()) {
		driver->pauseAll();
	}
}

void gemsResumeAll() {
	if (GemsDriver *driver = getDriver()) {
		driver->resumeAll();
	}
}

} // namespace Scooby
