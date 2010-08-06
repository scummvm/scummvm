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

BS_Framecounter::BS_Framecounter(int UpdateFrequency) :
	m_FPS(0),
	m_FPSCount(0),
	m_LastUpdateTime(-1) {
	SetUpdateFrequency(UpdateFrequency);
}

void BS_Framecounter::Update() {
	// Aktuellen Systemtimerstand auslesen
	uint64_t Timer = g_system->getMillis() * 1000;

	// Falls m_LastUpdateTime == -1 ist, wird der Frame-Counter zum ersten Mal aufgerufen und der aktuelle Systemtimer als erster
	// Messzeitpunkt genommen.
	if (m_LastUpdateTime == -1)
		m_LastUpdateTime = Timer;
	else {
		// Die Anzahl der Frames im aktuellen Messzeitraum wird erhöht.
		m_FPSCount++;

		// Falls der Messzeitraum verstrichen ist, wird die durchschnittliche Framerate berechnet und ein neuer Messzeitraum begonnen.
		if (Timer - m_LastUpdateTime >= m_UpdateDelay) {
			m_FPS = static_cast<int>((1000000 * (uint64_t)m_FPSCount) / (Timer - m_LastUpdateTime));
			m_LastUpdateTime = Timer;
			m_FPSCount = 0;
		}
	}
}

} // End of namespace Sword25
