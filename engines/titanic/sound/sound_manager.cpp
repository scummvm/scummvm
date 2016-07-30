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

#include "titanic/sound/sound_manager.h"

namespace Titanic {

CSoundManager::CSoundManager() : _musicPercent(75.0), _speechPercent(75.0),
	_masterPercent(75.0), _parrotPercent(75.0), _field14(1) {
}

/*------------------------------------------------------------------------*/

QSoundManager::QSoundManager() : _field18(0), _field1C(0) {
	Common::fill(&_field4A0[0], &_field4A0[16], 0);
}

int QSoundManager::loadSound(const CString &name) {
	warning("TODO");
	return 0;
}

int QSoundManager::proc4() const {
	warning("TODO");
	return 0;
}

int QSoundManager::proc5() const {
	warning("TODO");
	return 0;
}

void QSoundManager::proc6() {
	warning("TODO");
}

void QSoundManager::proc7() {
	warning("TODO");
}

void QSoundManager::proc8(int v) {
	warning("TODO");
}

void QSoundManager::proc9() {
	warning("TODO");
}

void QSoundManager::proc10() {
	warning("TODO");
}

void QSoundManager::proc11() {
	warning("TODO");
}

void QSoundManager::proc12() {
	warning("TODO");
}

void QSoundManager::proc13() {
	warning("TODO");
}

bool QSoundManager::proc14() {
	warning("TODO");
	return false;
}

bool QSoundManager::isActive(int handle) const {
	warning("TODO");
	return false;
}

int QSoundManager::proc16() const {
	warning("TODO");
	return 0;
}

void QSoundManager::WaveMixPump() {
	warning("TODO");
}

bool QSoundManager::movieStarted() const {
	// TODO
	return 0;
}

void QSoundManager::proc19(int v) {
	warning("TODO");
}

void QSoundManager::proc20(int v) {
	warning("TODO");
}

void QSoundManager::proc21(int v) {
	warning("TODO");
}

void QSoundManager::proc29() {
	warning("TODO");
}

void QSoundManager::proc30() {
	warning("TODO");
}

} // End of namespace Titanic z
