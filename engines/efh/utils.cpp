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

#include "common/config-manager.h"
#include "common/system.h"
#include "common/random.h"
#include "efh/efh.h"

namespace Efh {

void EfhEngine::setDefaultNoteDuration() {
	// Original implementation is based on the int1C, which is triggered at 18.2065Hz.
	// Every 4 times, it sets a flag (thus, approx every 220ms)
	// The function was checking the keyboard in a loop, incrementing a counter and setting the last character read
	// The "_defaultNoteDuration" was then set to 7 times this counter
	//
	// No implementation required in ScummVM
}

void EfhEngine::decryptImpFile(bool techMapFl) {
	debugC(1, kDebugUtils, "decryptImpFile %s", techMapFl ? "True" : "False");
	uint16 counter = 0;
	uint16 target;
	uint8 *curPtr;

	if (!techMapFl) {
		_imp2PtrArray[counter++] = curPtr = _imp2;
		target = 431;
	} else {
		_imp1PtrArray[counter++] = curPtr = _imp1;
		target = 99;
	}

	do {
		*curPtr = (*curPtr - 3) ^ 0xD7;
		if (*curPtr == 0x40) {
			curPtr += 3;
			if (!techMapFl)
				_imp2PtrArray[counter++] = curPtr;
			else
				_imp1PtrArray[counter++] = curPtr;
		} else
			++curPtr;
	} while (*curPtr != 0x60 && counter <= target);

	if (ConfMan.getBool("dump_scripts")) {
		// Dump the decompressed IMP file
		Common::DumpFile dump;
		if (!techMapFl) {
			dump.open("imp2_unc.dump");
			dump.write(_imp2, curPtr - _imp2);
		} else {
			dump.open("imp1_unc.dump");
			dump.write(_imp1, curPtr - _imp1);
		}
		dump.flush();
		dump.close();
	}
}

void EfhEngine::loadImageSet(int16 imageSetId, uint8 *buffer, uint8 **subFilesArray, uint8 *destBuffer) {
	debugC(1, kDebugUtils, "loadImageSet %d", imageSetId);
	Common::String fileName = Common::String::format("imageset.%d", imageSetId);
	rImageFile(fileName, buffer, subFilesArray, destBuffer);
}

uint32 EfhEngine::uncompressBuffer(uint8 *compressedBuf, uint8 *destBuf) {
	debugC(1, kDebugUtils, "uncompressBuffer");
	if (compressedBuf == nullptr || destBuf == nullptr)
		error("uncompressBuffer - Invalid pointer used in parameter list");

	uint8 *curPtrDest = destBuf;

	uint16 compSize = READ_LE_UINT16(compressedBuf) + 1;
	uint8 *curPtrCompressed = compressedBuf + 2;

	// Not in the original. This has been added for debug purposes (the original function doesn't return a value)
	uint32 decompSize = 0;

	for (;;) {
		uint8 next = *curPtrCompressed++;
		if (--compSize <= 0)
			break;

		if (next != 0xC3) {
			*curPtrDest++ = next;
			++decompSize;
			continue;
		}

		next = *curPtrCompressed++;
		if (--compSize <= 0)
			break;

		if (next == 0) {
			*curPtrDest++ = 0xC3;
			++decompSize;
			continue;
		}

		uint8 loopVal = next;
		next = *curPtrCompressed++;

		for (int i = 0; i < loopVal; ++i) {
			*curPtrDest++ = next;
			++decompSize;
		}

		--compSize;
		if (compSize == 0)
			break;
	}

	curPtrDest[0] = curPtrDest[1] = 0;
	decompSize += 2;

	return decompSize;
}

int16 EfhEngine::getRandom(int16 maxVal) {
	debugC(1, kDebugUtils, "getRandom %d", maxVal);
	if (maxVal <= 0)
		return 0;

	return 1 + _rnd->getRandomNumber(maxVal - 1);
}

Common::KeyCode EfhEngine::getLastCharAfterAnimCount(int16 delay) {
	debugC(1, kDebugUtils, "getLastCharAfterAnimCount %d", delay);
	if (delay == 0)
		return Common::KEYCODE_INVALID;

	Common::KeyCode lastChar = Common::KEYCODE_INVALID;

	uint32 lastMs = _system->getMillis();
	while (delay > 0 && lastChar == Common::KEYCODE_INVALID && !shouldQuit()) {
		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 200) {
			lastMs = newMs;
			--delay;
			handleAnimations();
		}

		lastChar = handleAndMapInput(false);
	}

	return lastChar;
}

Common::KeyCode EfhEngine::getInput(int16 delay) {
	debugC(1, kDebugUtils, "getInput %d", delay);
	if (delay == 0)
		return Common::KEYCODE_INVALID;

	Common::KeyCode lastChar = Common::KEYCODE_INVALID;
	Common::KeyCode retVal = Common::KEYCODE_INVALID;

	uint32 lastMs = _system->getMillis();
	while (delay > 0 && !shouldQuit()) {
		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 200) {
			lastMs = newMs;
			--delay;
			handleAnimations();
		}

		lastChar = handleAndMapInput(false);
		if (lastChar != Common::KEYCODE_INVALID)
			retVal = lastChar;
	}

	return retVal;
}

Common::KeyCode EfhEngine::getKeyCode(const Common::Event &event) {
	Common::KeyCode retVal = event.kbd.keycode;
	if (retVal == Common::KEYCODE_LCTRL || retVal == Common::KEYCODE_RCTRL || retVal == Common::KEYCODE_RALT || retVal == Common::KEYCODE_LALT)
		retVal = Common::KEYCODE_INVALID;
	else  if (event.kbd.flags & Common::KBD_CTRL) {
		switch (retVal) {
		case Common::KEYCODE_l:
			retVal = Common::KEYCODE_F7;
			break;
		case Common::KEYCODE_s:
			retVal = Common::KEYCODE_F5;
			break;
		case Common::KEYCODE_x:
		case Common::KEYCODE_q:
			_shouldQuit = true;
			retVal = Common::KEYCODE_INVALID;
			break;
		default:
			break;
		}
	} else if (event.kbd.flags & Common::KBD_ALT && retVal == Common::KEYCODE_F4) {
		_shouldQuit = true;
	}

	return retVal;
}

Common::KeyCode EfhEngine::waitForKey() {
	debugC(1, kDebugUtils, "waitForKey");
	Common::KeyCode retVal = Common::KEYCODE_INVALID;
	Common::Event event;

	uint32 lastMs = _system->getMillis();
	while (retVal == Common::KEYCODE_INVALID && !shouldQuit()) {
		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 200) {
			lastMs = newMs;
			handleAnimations();
		}

		_system->getEventManager()->pollEvent(event);
		if (event.type == Common::EVENT_KEYUP)
			retVal = getKeyCode(event);
	}

	return retVal;
}

Common::KeyCode EfhEngine::mapInputCode(Common::KeyCode input) {
	// Original is doing:
	// if input < a or > z : return input
	// else return (input + 0xE0)
	// ex: 'a' = 0x61 + 0xE0 = 0x0141, but it's a uint8 so it's 0x41 which is 'A'.
	// So basically the original works with uppercase letters and do not alter the other inputs.
	// => no implementation needed.
	return input;
}

Common::KeyCode EfhEngine::handleAndMapInput(bool animFl) {
	debugC(1, kDebugUtils, "handleAndMapInput %s", animFl ? "True" : "False");
	// The original checks for the joystick input
	Common::Event event;
	_system->getEventManager()->pollEvent(event);
	Common::KeyCode retVal = Common::KEYCODE_INVALID;

	uint32 lastMs = _system->getMillis();
	while (retVal == Common::KEYCODE_INVALID && !shouldQuit()) {
		_system->getEventManager()->pollEvent(event);

		if (event.type == Common::EVENT_KEYUP)
			retVal = getKeyCode(event);

		if (animFl) {
			_system->delayMillis(20);
			uint32 newMs = _system->getMillis();

			if (newMs - lastMs >= 200) {
				lastMs = newMs;
				handleAnimations();
			}
		} else
			break;
	}
	return retVal;
}

Common::KeyCode EfhEngine::getInputBlocking() {
	debugC(1, kDebugUtils, "getInputBlocking");
	// The original checks for the joystick input
	Common::Event event;
	_system->getEventManager()->pollEvent(event);
	Common::KeyCode retVal = Common::KEYCODE_INVALID;

	uint32 lastMs = _system->getMillis();
	while (retVal == Common::KEYCODE_INVALID && !shouldQuit()) {
		_system->getEventManager()->pollEvent(event);

		if (event.type == Common::EVENT_KEYUP)
			retVal = getKeyCode(event);

		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 220) {
			lastMs = newMs;
			handleAnimations();
		}
	}
	return retVal;
}

void EfhEngine::setNumLock() {
	// No implementation in ScummVM
}

bool EfhEngine::getValidationFromUser() {
	debugC(1, kDebugUtils, "getValidationFromUser");
	Common::KeyCode input = handleAndMapInput(true);
	if (input == Common::KEYCODE_y) // or if joystick button 1
		return true;

	return false;
}

uint32 EfhEngine::ROR(uint32 val, uint8 shiftVal) {
	return val >> shiftVal | val << (32 - shiftVal);
}

Common::String EfhEngine::getArticle(int pronoun) {
	if (pronoun == 2)
		return "The ";

	return "";
}
} // End of namespace Efh
