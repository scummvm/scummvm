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

#ifndef SWORD25_FRAMECOUNTER_H
#define SWORD25_FRAMECOUNTER_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/bs_stdint.h"

/**
	@brief Eine einfache Klasse die einen Framecounter implementiert.
*/
class BS_Framecounter
{
private:
	enum
	{
		DEFAULT_UPDATE_FREQUENCY = 10
	};

public:
	/**
		@brief Erzeugt ein neues BS_Framecounter Objekt.
		@param UpdateFrequency gibt an wie oft der Framecounter in einer Sekunde aktualisiert werden soll.<br>
			   Der Standardwert ist 10.
	*/
	BS_Framecounter(int UpdateFrequency = DEFAULT_UPDATE_FREQUENCY);

	/**
		@brief Bestimmt wie oft der Framecounter in einer Sekunde aktualisiert werden soll.
		@param UpdateFrequency gibt an wie oft der Framecounter in einer Sekunde aktualisiert werden soll.
	*/
	inline void SetUpdateFrequency(int UpdateFrequency);

	/**
		@brief Diese Methode muss einmal pro Frame aufgerufen werden.
	*/
	void Update();

	/**
		@brief Gibt den aktuellen FPS-Wert zurück.
	*/
	int GetFPS() const { return m_FPS; }

private:
	int	m_FPS;
	int m_FPSCount;
	uint64_t m_LastUpdateTime;
	uint64_t m_UpdateDelay;
};

// Inlines
void BS_Framecounter::SetUpdateFrequency(int UpdateFrequency)
{
	// Frequenz in Laufzeit (in Microsekunden) umrechnen.
	m_UpdateDelay = 1000000 / UpdateFrequency;
}

#endif
