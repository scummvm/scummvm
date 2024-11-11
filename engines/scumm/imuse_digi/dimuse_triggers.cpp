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

#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/imuse_digi/dimuse_triggers.h"

namespace Scumm {

IMuseDigiTriggersHandler::IMuseDigiTriggersHandler(IMuseDigital *engine) {
	_engine = engine;
	_emptyMarker[0] = '\0';
}

IMuseDigiTriggersHandler::~IMuseDigiTriggersHandler() {}

int IMuseDigiTriggersHandler::init() {
	return clearAllTriggers();
}

int IMuseDigiTriggersHandler::deinit() {
	return clearAllTriggers();
}

int IMuseDigiTriggersHandler::clearAllTriggers() {
	for (int l = 0; l < DIMUSE_MAX_TRIGGERS; l++) {
		_trigs[l].sound = 0;
		memset(_trigs[l].text, 0, sizeof(_trigs[l].text));
		_trigs[l].opcode = 0;
		_trigs[l].a = 0;
		_trigs[l].b = 0;
		_trigs[l].c = 0;
		_trigs[l].d = 0;
		_trigs[l].e = 0;
		_trigs[l].f = 0;
		_trigs[l].g = 0;
		_trigs[l].h = 0;
		_trigs[l].i = 0;
		_trigs[l].j = 0;
		_trigs[l].clearLater = 0;
	}

	for (int l = 0; l < DIMUSE_MAX_DEFERS; l++) {
		_defers[l].counter = 0;
		_defers[l].opcode = 0;
		_defers[l].a = 0;
		_defers[l].b = 0;
		_defers[l].c = 0;
		_defers[l].d = 0;
		_defers[l].e = 0;
		_defers[l].f = 0;
		_defers[l].g = 0;
		_defers[l].h = 0;
		_defers[l].i = 0;
		_defers[l].j = 0;
	}
	_defersOn = 0;
	_midProcessing = 0;
	return 0;
}

void IMuseDigiTriggersHandler::saveLoad(Common::Serializer &ser) {
	for (int l = 0; l < DIMUSE_MAX_TRIGGERS; l++) {
		ser.syncAsSint32LE(_trigs[l].sound, VER(103));
		ser.syncArray(_trigs[l].text, 256, Common::Serializer::SByte, VER(103));
		ser.syncAsSint32LE(_trigs[l].opcode, VER(103));
		ser.syncAsSint32LE(_trigs[l].a, VER(103));
		ser.syncAsSint32LE(_trigs[l].b, VER(103));
		ser.syncAsSint32LE(_trigs[l].c, VER(103));
		ser.syncAsSint32LE(_trigs[l].d, VER(103));
		ser.syncAsSint32LE(_trigs[l].e, VER(103));
		ser.syncAsSint32LE(_trigs[l].f, VER(103));
		ser.syncAsSint32LE(_trigs[l].g, VER(103));
		ser.syncAsSint32LE(_trigs[l].h, VER(103));
		ser.syncAsSint32LE(_trigs[l].i, VER(103));
		ser.syncAsSint32LE(_trigs[l].j, VER(103));
		ser.syncAsSint32LE(_trigs[l].clearLater, VER(103));
	}

	for (int l = 0; l < DIMUSE_MAX_DEFERS; l++) {
		ser.syncAsSint32LE(_defers[l].counter, VER(103));
		ser.syncAsSint32LE(_defers[l].opcode, VER(103));
		ser.syncAsSint32LE(_defers[l].a, VER(103));
		ser.syncAsSint32LE(_defers[l].b, VER(103));
		ser.syncAsSint32LE(_defers[l].c, VER(103));
		ser.syncAsSint32LE(_defers[l].d, VER(103));
		ser.syncAsSint32LE(_defers[l].e, VER(103));
		ser.syncAsSint32LE(_defers[l].f, VER(103));
		ser.syncAsSint32LE(_defers[l].g, VER(103));
		ser.syncAsSint32LE(_defers[l].h, VER(103));
		ser.syncAsSint32LE(_defers[l].i, VER(103));
		ser.syncAsSint32LE(_defers[l].j, VER(103));
	}

	if (ser.isLoading())
		_defersOn = 1;
}

int IMuseDigiTriggersHandler::setTrigger(int soundId, char *marker, int opcode, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	if (soundId == 0) {
		return -5;
	}

	if (marker == nullptr) {
		marker = _emptyMarker;
	}

	if (strlen(marker) >= 256) {
		debug(5, "IMuseDigiTriggersHandler::setTrigger(): ERROR: attempting to set trigger with oversized marker string");
		return -5;
	}

	for (int index = 0; index < DIMUSE_MAX_TRIGGERS; index++) {
		if (_trigs[index].sound == 0) {
			_trigs[index].sound = soundId;
			_trigs[index].clearLater = 0;
			_trigs[index].opcode = opcode;
			Common::strlcpy(_trigs[index].text, marker, sizeof(_trigs[index].text));
			_trigs[index].a = d;
			_trigs[index].b = e;
			_trigs[index].c = f;
			_trigs[index].d = g;
			_trigs[index].e = h;
			_trigs[index].f = i;
			_trigs[index].g = j;
			_trigs[index].h = k;
			_trigs[index].i = l;
			_trigs[index].j = m;

			debug(5, "IMuseDigiTriggersHandler::setTrigger(): Successfully set trigger for soundId %d and marker '%s'", soundId, marker);
			return 0;
		}
	}
	debug(5, "IMuseDigiTriggersHandler::setTrigger(): ERROR: unable to allocate trigger \"%s\" for sound %d, every slot is full", marker, soundId);
	return -6;
}

int IMuseDigiTriggersHandler::checkTrigger(int soundId, char *marker, int opcode) {
	int r = 0;
	for (int l = 0; l < DIMUSE_MAX_TRIGGERS; l++) {
		if (_trigs[l].sound != 0) {
			if (soundId == -1 || _trigs[l].sound == soundId) {
				if (!strcmp(marker, _emptyMarker) || !strcmp(marker, _trigs[l].text)) {
					if (opcode == -1 || _trigs[l].opcode == opcode)
						r++;
				}
			}
		}
	}

	return r;
}

int IMuseDigiTriggersHandler::clearTrigger(int soundId, char *marker, int opcode) {
	for (int l = 0; l < DIMUSE_MAX_TRIGGERS; l++) {
		if ((_trigs[l].sound != 0) && (soundId == -1 || _trigs[l].sound == soundId) &&
            (!strcmp(marker, _emptyMarker) || !strcmp(marker, _trigs[l].text)) &&
            (opcode == -1 || _trigs[l].opcode == opcode)) {

			if (_midProcessing) {
				_trigs[l].clearLater = 1;
			} else {
				_trigs[l].sound = 0;
			}
		}
	}
	return 0;
}

void IMuseDigiTriggersHandler::processTriggers(int soundId, char *marker) {
	char textBuffer[256];
	int r;
	if (strlen(marker) >= 256) {
		debug(5, "IMuseDigiTriggersHandler::processTriggers(): ERROR: the input marker string is oversized");
		return;
	}

	Common::strlcpy(_textBuffer, marker, sizeof(textBuffer));
	_midProcessing++;
	for (int l = 0; l < DIMUSE_MAX_TRIGGERS; l++) {
		if (!_trigs[l].sound || _trigs[l].sound != soundId || (_trigs[l].text[0] && strcmp(_textBuffer, _trigs[l].text))) {
			continue;
		}

		// Save the string into our local buffer for later
		r = 0;
		if (_textBuffer[0] != '\0') {
			do {
				textBuffer[r] = _textBuffer[r];
				r++;
			} while (_textBuffer[r] != '\0');
		}
		textBuffer[r] = '\0';

		_trigs[l].sound = 0;

		debug(5, "IMuseDigiTriggersHandler::processTriggers(): executing trigger for soundId %d and marker '%s'", soundId, marker);
		if (_trigs[l].opcode == DIMUSE_C_SCRIPT_CALLBACK) {
			// Call the script callback (a function which sets _stoppingSequence to 1)
			_engine->scriptTriggerCallback(_textBuffer);
		} else {
			if (_trigs[l].opcode < 30) {
				// Execute a command
				_engine->cmdsHandleCmd(_trigs[l].opcode, (uint8 *)textBuffer,
					_trigs[l].a, _trigs[l].b,
					_trigs[l].c, _trigs[l].d,
					_trigs[l].e, _trigs[l].f,
					_trigs[l].g, _trigs[l].h,
					_trigs[l].i, _trigs[l].j);
			}
		}

		// Restore the global textBuffer
		r = 0;
		if (textBuffer[0] != '\0') {
			do {
				_textBuffer[r] = textBuffer[r];
				r++;
			} while (textBuffer[r] != '\0');
		}
		_textBuffer[r] = '\0';
	}
	if (--_midProcessing == 0) {
		for (int l = 0; l < DIMUSE_MAX_TRIGGERS; l++) {
			if (_trigs[l].clearLater) {
				_trigs[l].sound = 0;
			}
		}
	}
}

int IMuseDigiTriggersHandler::deferCommand(int count, int opcode, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	if (!count) {
		return -5;
	}
	for (int index = 0; index < DIMUSE_MAX_DEFERS; index++) {
		if (!_defers[index].counter) {
			_defers[index].counter = count;
			_defers[index].opcode = opcode;
			_defers[index].a = c;
			_defers[index].b = d;
			_defers[index].c = e;
			_defers[index].d = f;
			_defers[index].e = g;
			_defers[index].f = h;
			_defers[index].g = i;
			_defers[index].h = j;
			_defers[index].i = k;
			_defers[index].j = l;
			_defersOn = 1;
			return 0;
		}
	}
	debug(5, "IMuseDigiTriggersHandler::deferCommand(): ERROR: couldn't allocate deferred command");
	return -6;
}

void IMuseDigiTriggersHandler::loop() {
	if (!_defersOn)
		return;

	_defersOn = 0;
	for (int l = 0; l < DIMUSE_MAX_DEFERS; l++) {
		if (_defers[l].counter == 0)
			continue;

		_defersOn = 1;
		_defers[l].counter--;

		if (_defers[l].counter == 1) {
			if (_defers[l].opcode == DIMUSE_C_SCRIPT_CALLBACK) {
				_engine->scriptTriggerCallback(_trigs[l].text);
			} else {
				if (_defers[l].opcode < 30) {
					_engine->cmdsHandleCmd(_trigs[l].opcode, nullptr,
						_trigs[l].a, _trigs[l].b,
						_trigs[l].c, _trigs[l].d,
						_trigs[l].e, _trigs[l].f,
						_trigs[l].g, _trigs[l].h,
						_trigs[l].i, _trigs[l].j);
				}
			}
		}
	}
}

int IMuseDigiTriggersHandler::countPendingSounds(int soundId) {
	int r = 0;
	for (int l = 0; l < DIMUSE_MAX_TRIGGERS; l++) {
		if (!_trigs[l].sound)
			continue;

		int opcode = _trigs[l].opcode;
		if ((opcode == DIMUSE_C_START_SND && _trigs[l].a == soundId) ||
			(opcode == DIMUSE_C_SWITCH_STREAM && _trigs[l].b == soundId)) {
			r++;
		}
	}

	for (int l = 0; l < DIMUSE_MAX_DEFERS; l++) {
		if (!_defers[l].counter)
			continue;

		int opcode = _defers[l].opcode;
		if ((opcode == DIMUSE_C_START_SND && _defers[l].a == soundId) ||
			(opcode == DIMUSE_C_SWITCH_STREAM && _defers[l].b == soundId)) {
			r++;
		}
	}

	return r;
}

} // End of namespace Scumm
