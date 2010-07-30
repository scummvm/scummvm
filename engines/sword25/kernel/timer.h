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

/*
	BS_Timer
	--------
	Eine Klasse zum Auslesen des Systemtimers.

	Autor: Malte Thiesen
*/

#ifndef SWORD25_TIMER_H
#define SWORD25_TIMER_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/bs_stdint.h"

/**
	@brief Eine Klasse zum Auslesen des Systemtimers.

	Vor der Benutzung sollte immer mit IsTimerAvaliable() getestet werden, ob der Timer von der benutzten
	Hardware unterstützt wird.
*/
class BS_Timer
{
public:
	/**
		@brief List den Systemtimer in Microsekunden aus.
		@return Die Zahl vergangener Microsekunden seit dem Systemstart.<br>
	*/
	static uint64_t GetMicroTicks();

	/**
		@brief List den Systemtimer in Millisekunden aus.
		@return Die Zahl vergangener Millisekunden seit dem Systemstart.<br>
	*/
	static unsigned int GetMilliTicks();

	/**
		@brief Prüft, ob der Timer auf der vorhandenen Hardware existiert.
		@return Gibt false zurück, fals der Timer auf der vorhandenen Hardware nicht existiert.
	*/
	static bool IsTimerAvaliable();

private:
	/**
		@brief Initialisiert den Timer.
	*/
	static void Init();
};

#endif
