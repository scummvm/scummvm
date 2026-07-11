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

#include "comfy/comfy.h"
#include "comfy/midiplyr/midiplyr.h"

#include "common/ptr.h"

namespace Comfy {

void ComfyEngine::inputQueuePushKey(uint16 key) {
	_inputQueue.words[_inputQueue.writeIndex % COMFY_INPUT_QUEUE_CAPACITY] = 0;
	_inputQueue.writeIndex = (_inputQueue.writeIndex + 1) % COMFY_INPUT_QUEUE_CAPACITY;
	_inputQueue.words[_inputQueue.writeIndex % COMFY_INPUT_QUEUE_CAPACITY] = key;
	_inputQueue.writeIndex = (_inputQueue.writeIndex + 1) % COMFY_INPUT_QUEUE_CAPACITY;
}

void ComfyEngine::inputQueuePushChar(uint16 key) {
	_inputQueue.words[_inputQueue.writeIndex % COMFY_INPUT_QUEUE_CAPACITY] = key;
	_inputQueue.writeIndex = (_inputQueue.writeIndex + 1) % COMFY_INPUT_QUEUE_CAPACITY;
}

bool ComfyEngine::inputQueueHasItems() {
	return _inputQueue.writeIndex != _inputQueue.readIndex;
}

uint16 ComfyEngine::inputQueueDequeue() {
	if (!inputQueueHasItems())
		return 0xFFFF;

	uint16 value = _inputQueue.words[_inputQueue.readIndex % COMFY_INPUT_QUEUE_CAPACITY];
	_inputQueue.readIndex = (_inputQueue.readIndex + 1) % COMFY_INPUT_QUEUE_CAPACITY;
	return value;
}

void ComfyEngine::inputQueueReset() {
	_inputQueue.tailIndex = 0;
}

uint16 ComfyEngine::lptReadKeyOrNext() {
	uint16 key = inputQueueDequeue();
	if (!key)
		key = inputQueueDequeue() + 0x100;

	return key;
}

void ComfyEngine::lptKeyToFlags(uint16 key) {
	uint32 flags = 0;
	uint16 next = key;

	for (;;) {
		uint16 current = next != 0xFFFF ? next : lptReadKeyOrNext();
		next = 0xFFFF;

		switch (current) {
		case 331:
			flags |= 0x01;
			break;
		case 333:
			flags |= 0x02;
			break;
		case 328:
			flags |= 0x04;
			break;
		case 336:
			flags |= 0x08;
			break;
		case 13:
			flags |= 0x10;
			break;
		case 27:
			flags |= 0x20;
			break;
		default:
			break;
		}

		if (!inputQueueHasItems()) {
			keyBitCopyRange(0x38, 8, flags);
			return;
		}
	}
}

void ComfyEngine::hostKeyboardResetMap() {
	memset(_keyboardKeyToBit, 0xFF, sizeof(_keyboardKeyToBit));
	_keyboardMapLoaded = false;
	_keyboardActiveMask = 0;
	_keyboardLatchedMask = 0;
}

bool ComfyEngine::hostKeyboardLoadDatMap() {
	hostKeyboardResetMap();
	Common::ScopedPtr<Common::SeekableReadStream> stream(pathFOpen(Common::Path("KEYBOARD.DAT"), false));
	if (!stream)
		stream.reset(pathFOpen(Common::Path("KEYBOARD.DAT"), true));

	if (!stream)
		return false;

	byte value;
	do {
		if (stream->read(&value, 1) != 1)
			return false;
	} while (value);

	bool loaded = false;
	while (stream->pos() < stream->size()) {
		byte key;
		byte bit;
		if (stream->read(&key, 1) != 1 || stream->read(&bit, 1) != 1 || bit >= COMFY_KEYBOARD_CONTACT_COUNT) {
			hostKeyboardResetMap();
			return false;
		}

		_keyboardKeyToBit[key] = bit;
		loaded = true;
	}

	_keyboardMapLoaded = loaded;
	return loaded;
}

void ComfyEngine::hostKeyboardSetKeyState(uint16 key, bool pressed) {
	if (!_keyboardMapLoaded || key > 0xFF)
		return;

	byte bit = _keyboardKeyToBit[key];
	if (bit >= COMFY_KEYBOARD_CONTACT_COUNT)
		return;

	uint32 mask = 1 << bit;
	if (pressed) {
		_keyboardActiveMask |= mask;
		_keyboardLatchedMask &= ~mask;
	} else {
		_keyboardLatchedMask |= mask;
	}
}

uint16 ComfyEngine::hostKeyboardVirtualKey(Common::KeyCode key) {
	if (key >= Common::KEYCODE_a && key <= Common::KEYCODE_z)
		return 'A' + key - Common::KEYCODE_a;

	if (key >= Common::KEYCODE_0 && key <= Common::KEYCODE_9)
		return '0' + key - Common::KEYCODE_0;

	switch (key) {
	case Common::KEYCODE_UP:
		return 0x26;
	case Common::KEYCODE_DOWN:
		return 0x28;
	case Common::KEYCODE_LEFT:
		return 0x25;
	case Common::KEYCODE_RIGHT:
		return 0x27;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		return 0x0D;
	case Common::KEYCODE_ESCAPE:
		return 0x1B;
	case Common::KEYCODE_SPACE:
		return 0x20;
	default:
		return key <= 0xFF ? key : 0xFFFF;
	}
}

void ComfyEngine::setKeyboardContact(uint16 contact, bool pressed, bool keymapper) {
	if (contact >= COMFY_KEYBOARD_CONTACT_COUNT)
		return;

	uint32 mask = 1 << contact;
	uint32 *activeMask = keymapper ? &_keymapperActiveMask : &_toyKeyboardActiveMask;
	uint32 *latchedMask = keymapper ? &_keymapperLatchedMask : &_toyKeyboardLatchedMask;
	if (pressed) {
		*activeMask |= mask;
		*latchedMask &= ~mask;
	} else {
		*activeMask &= ~mask;
		*latchedMask |= mask;
	}
}

uint32 ComfyEngine::lptKeyboardScan() {
	// The original wrote eight successive column strobes to the LPT data port, waited 100 busy-loop
	// iterations after each write, and sampled three bits from the status port. Concatenating those
	// eight 3-bit samples produced the 24-contact scan word. ScummVM supplies that same word from the
	// keymapper, KEYBOARD.DAT mappings, and the toy-keyboard UI instead of accessing an I/O port.
	uint32 keyboardReleased = (_keyboardLatchedMask | _keymapperLatchedMask) & 0x00FFFFFF;
	uint32 keyboardState = (_keyboardActiveMask | _keymapperActiveMask | keyboardReleased) & 0x00FFFFFF;
	uint32 comfyboardState = (_toyKeyboardActiveMask | _toyKeyboardHoldMask |
		_toyKeyboardLatchedMask) & 0x00FFFFFF;
	if (_midiPlyrDriver)
		comfyboardState |= _midiPlyrDriver->comfyboardGetButtons() & 0x00FFFFFF;

	uint32 scanState;
	if (_engineVersion == kEngineVersion3 && _inputDeviceMode == 1)
		scanState = comfyboardState;
	else if (_engineVersion == kEngineVersion3 && _inputDeviceMode == 2)
		scanState = keyboardState;
	else
		scanState = comfyboardState | keyboardState;

	_keyboardActiveMask &= ~_keyboardLatchedMask;
	_keymapperActiveMask &= ~_keymapperLatchedMask;
	_toyKeyboardActiveMask &= ~_toyKeyboardLatchedMask;
	_keyboardLatchedMask = 0;
	_keymapperLatchedMask = 0;
	_toyKeyboardLatchedMask = 0;
	return scanState;
}

void ComfyEngine::lptKeyboardDispatchEvents(uint32 scanState) {
	keyBitCopyRange(8, 0x18, scanState);

	uint32 pressedThisFrame = (~_lptPrevScanState & scanState) & 0x00FFFFFF;
	keyBitCopyRange(0x20, 0x18, pressedThisFrame);

	if (pressedThisFrame & 0x00FFDFFF)
		keyBitSet(6);
	else
		keyBitClear(6);

	if ((scanState & 0x2000) != (_lptPrevScanState & 0x2000))
		keyBitSet(0x41);
	else
		keyBitClear(0x41);

	if ((_lptPrevScanState & 0x2000) && ((~scanState & 0x00FFFFFF) & 0x2000))
		keyBitSet(0x44);
	else
		keyBitClear(0x44);

	if (keyBitTest(0x15))
		keyBitClear(0x45);
	else
		keyBitSet(0x45);

	if ((_lptPrevScanState & 0x80) && ((~scanState & 0x00FFFFFF) & 0x80))
		keyBitSet(0x43);
	else
		keyBitClear(0x43);

	if (pressedThisFrame & 0x24924)
		keyBitSet(7);
	else
		keyBitClear(7);

	_lptPrevScanState = scanState;
}

void ComfyEngine::lptKeyboardInit() {
	if (_lptKeyboardInitialized)
		return;

	inputQueueReset();
	hostKeyboardLoadDatMap();
	_lptPrevScanState = 0;
	_lptKeyboardInitialized = true;
	_keyboardUiVisible = true;
	if (_midiPlyrDriver) {
		if (_engineVersion == kEngineVersion3 && !_inputDeviceMode)
			_midiPlyrDriver->comfyboardStartSleepUse();

		_midiPlyrDriver->comfyboardStartReading(0x006D96DB, 0);
	}

#ifdef USE_IMGUI
	ImGuiCallbacks imGuiCallbacks;
	imGuiCallbacks.init = onImGuiInit;
	imGuiCallbacks.render = onImGuiRender;
	imGuiCallbacks.cleanup = onImGuiCleanup;
	_system->setImGuiCallbacks(imGuiCallbacks);
	_keyboardUiInitialized = true;
#endif
}

void ComfyEngine::lptKeyboardScanAndProcess() {
	if (_lptKeyboardInitialized)
		lptKeyboardDispatchEvents(lptKeyboardScan());
}

void ComfyEngine::lptKeyboardShutdown() {
#ifdef USE_IMGUI
	if (_keyboardUiInitialized)
		_system->setImGuiCallbacks(ImGuiCallbacks());
#endif

	_keyboardUiInitialized = false;
	if (_midiPlyrDriver) {
		_midiPlyrDriver->comfyboardStopReading();
		_midiPlyrDriver->comfyboardStopSleepUse();
	}

	hostKeyboardResetMap();
	_keymapperActiveMask = 0;
	_keymapperLatchedMask = 0;
	_toyKeyboardActiveMask = 0;
	_toyKeyboardLatchedMask = 0;
	_toyKeyboardHoldMask = 0;
	_lptKeyboardInitialized = false;
}

} // End of namespace Comfy
