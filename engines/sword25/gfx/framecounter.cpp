// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#include "framecounter.h"
#include "kernel/timer.h"

BS_Framecounter::BS_Framecounter(int UpdateFrequency) :
	m_FPS(0),
	m_FPSCount(0),
	m_LastUpdateTime(-1)
{
	SetUpdateFrequency(UpdateFrequency);
}

void BS_Framecounter::Update()
{
	// Aktuellen Systemtimerstand auslesen
	uint64_t Timer = BS_Timer::GetMicroTicks();

	// Falls m_LastUpdateTime == -1 ist, wird der Frame-Counter zum ersten Mal aufgerufen und der aktuelle Systemtimer als erster
	// Messzeitpunkt genommen.
	if (m_LastUpdateTime == -1)
		m_LastUpdateTime = Timer;
	else
	{
		// Die Anzahl der Frames im aktuellen Messzeitraum wird erhöht.
		m_FPSCount++;

		// Falls der Messzeitraum verstrichen ist, wird die durchschnittliche Framerate berechnet und ein neuer Messzeitraum begonnen.
		if (Timer - m_LastUpdateTime >= m_UpdateDelay)
		{
			m_FPS = static_cast<int>((1000000 * (uint64_t)m_FPSCount) / (Timer - m_LastUpdateTime));
			m_LastUpdateTime = Timer;
			m_FPSCount = 0;
		}
	}
}
