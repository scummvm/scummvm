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

#include "engines/grim/imuse/imuse_engine.h"
#include "engines/grim/imuse/imuse_fades.h"

namespace Grim {

IMuseFadesHandler::IMuseFadesHandler(IMuse *engine, Common::Mutex *mutex) {
	_engine = engine;
	_mutex = mutex;
}

IMuseFadesHandler::~IMuseFadesHandler() {}

int IMuseFadesHandler::init() {
	clearAllFades();
	return 0;
}

int IMuseFadesHandler::fadeParam(int soundId, int opcode, int destinationValue, int fadeLength) {
	Common::StackLock lock(*_mutex);

	if (!soundId || fadeLength < 0)
		return -5;

	if (opcode != IMUSE_P_PRIORITY && opcode != IMUSE_P_VOLUME && opcode != IMUSE_P_PAN && opcode != IMUSE_P_DETUNE)
		return -5;

	clearFadeStatus(soundId, opcode);

	int len50HzTicks = 5 * fadeLength / 6;

	if (!len50HzTicks) {
		debug(5, "IMuseFadesHandler::fadeParam(): WARNING: allocated fade with zero length for sound %d", soundId);
		if (opcode != IMUSE_P_VOLUME || destinationValue) {
			_engine->imSetParam(soundId, opcode, destinationValue);
		} else {
			_engine->imStopSound(soundId);
		}

		return 0;
	}

	for (int l = 0; l < IMUSE_MAX_FADES; l++) {
		if (!_fades[l].status) {
			_fades[l].sound = soundId;
			_fades[l].param = opcode;
			_fades[l].currentVal = _engine->imGetParam(soundId, opcode);
			_fades[l].length = len50HzTicks;
			_fades[l].counter = len50HzTicks;
			_fades[l].slope = (destinationValue - _fades[l].currentVal) / len50HzTicks;
			_fades[l].modOvfloCounter = 0;
			_fades[l].status = 1;
			_fadesOn = 1;

			if ((destinationValue - _fades[l].currentVal) < 0) {
				_fades[l].nudge = -1;
				_fades[l].slopeMod = (-(destinationValue - _fades[l].currentVal) % len50HzTicks);
			} else {
				_fades[l].nudge = 1;
				_fades[l].slopeMod = (destinationValue - _fades[l].currentVal) % len50HzTicks;
			}

			return 0;
		}
	}

	debug(5, "IMuseFadesHandler::fadeParam(): unable to allocate fade for sound %d", soundId);
	return -6;
}

void IMuseFadesHandler::clearFadeStatus(int soundId, int opcode) {
	for (int l = 0; l < IMUSE_MAX_FADES; l++) {
		if (_fades[l].status
			&& _fades[l].sound == soundId
			&& (_fades[l].param == opcode || opcode == -1)) {
			_fades[l].status = 0;
		}
	}
}

void IMuseFadesHandler::loop() {
	if (!_fadesOn)
		return;

	_fadesOn = 0;

	for (int l = 0; l < IMUSE_MAX_FADES; l++) {
		if (_fades[l].status) {
			_fadesOn = 1;
			if (--_fades[l].counter == 0) {
				_fades[l].status = 0;
			}

			int currentVolume = _fades[l].currentVal + _fades[l].slope;
			int currentSlopeMod = _fades[l].modOvfloCounter + _fades[l].slopeMod;
			_fades[l].modOvfloCounter += _fades[l].slopeMod;

			if (_fades[l].length <= currentSlopeMod) {
				_fades[l].modOvfloCounter = currentSlopeMod - _fades[l].length;
				currentVolume += _fades[l].nudge;
			}

			if (_fades[l].currentVal != currentVolume) {
				_fades[l].currentVal = currentVolume;

				if (!(_fades[l].counter % 5)) {
					debug(5, "IMuseFadesHandler::loop(): running fade for sound %d with id %d, currently at volume %d", _fades[l].sound, l, currentVolume);
					if ((_fades[l].param != IMUSE_P_VOLUME) || currentVolume) {
						_engine->imSetParam(_fades[l].sound, _fades[l].param, currentVolume);
					} else {
						_engine->imStopSound(_fades[l].sound);
					}
				}
			}
		}
	}
}

void IMuseFadesHandler::saveLoad(Common::Serializer &ser) {
	for (int l = 0; l < IMUSE_MAX_FADES; l++) {
		ser.syncAsSint32LE(_fades[l].status, VER(30));
		ser.syncAsSint32LE(_fades[l].sound, VER(30));
		ser.syncAsSint32LE(_fades[l].param, VER(30));
		ser.syncAsSint32LE(_fades[l].currentVal, VER(30));
		ser.syncAsSint32LE(_fades[l].counter, VER(30));
		ser.syncAsSint32LE(_fades[l].length, VER(30));
		ser.syncAsSint32LE(_fades[l].slope, VER(30));
		ser.syncAsSint32LE(_fades[l].slopeMod, VER(30));
		ser.syncAsSint32LE(_fades[l].modOvfloCounter, VER(30));
		ser.syncAsSint32LE(_fades[l].nudge, VER(30));
	}

	if (ser.isLoading())
		_fadesOn = 1;
}

void IMuseFadesHandler::clearAllFades() {
	Common::StackLock lock(*_mutex);

	for (int l = 0; l < IMUSE_MAX_FADES; l++) {
		_fades[l].status = 0;
		_fades[l].sound = 0;
		_fades[l].param = 0;
		_fades[l].currentVal = 0;
		_fades[l].counter = 0;
		_fades[l].length = 0;
		_fades[l].slope = 0;
		_fades[l].slopeMod = 0;
		_fades[l].modOvfloCounter = 0;
		_fades[l].nudge = 0;
	}

	_fadesOn = 0;
}

} // End of namespace Grim
