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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/system.h"
#include "sword25/gfx/framecounter.h"

namespace Sword25 {

Framecounter::Framecounter(int updateFrequency) :
	_FPS(0),
	_FPSCount(0),
	_lastUpdateTime(-1) {
	setUpdateFrequency(updateFrequency);
}

void Framecounter::update() {
	// Aktuellen Systemtimerstand auslesen
	uint64 timer = g_system->getMillis() * 1000;

	// Falls m_LastUpdateTime == -1 ist, wird der Frame-Counter zum ersten Mal aufgerufen und der aktuelle Systemtimer als erster
	// Messzeitpunkt genommen.
	if (_lastUpdateTime == -1)
		_lastUpdateTime = timer;
	else {
		// Die Anzahl der Frames im aktuellen Messzeitraum wird erhöht.
		_FPSCount++;

		// Falls der Messzeitraum verstrichen ist, wird die durchschnittliche Framerate berechnet und ein neuer Messzeitraum begonnen.
		if (timer - _lastUpdateTime >= _updateDelay) {
			_FPS = static_cast<int>((1000000 * (uint64)_FPSCount) / (timer - _lastUpdateTime));
			_lastUpdateTime = timer;
			_FPSCount = 0;
		}
	}
}

} // End of namespace Sword25
