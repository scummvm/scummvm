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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_LOGICTIMER_H
#define HPL_LOGICTIMER_H

namespace hpl {

class LowLevelSystem;

class cLogicTimer {
public:
	/**
	 *
	 * \param alUpdatesPerSec Number of updates per second.
	 * \param *apLowLevelSystem
	 * \return
	 */
	cLogicTimer(int alUpdatesPerSec, LowLevelSystem *apLowLevelSystem);
	~cLogicTimer();

	/**
	 * Reset the time. Do this when the logical update of the game has been ide, ie while loading.
	 */
	void Reset();
	/**
	 * Check with the time if it is time for another logical update
	 * \return true if logic should be updated, else false.
	 */
	bool WantUpdate();

	/**
	 * Resets various variables that makes the graphics is never frozen.
	 */
	void EndUpdateLoop();

	/**
	 * Set the number of times per second to update
	 * \param alUpdatesPerSec
	 */
	void SetUpdatesPerSec(int alUpdatesPerSec);

	/**
	 * Sets the maximum updates in a row.
	 * \param alUpdatesPerSec
	 */
	void SetMaxUpdates(int alMax);

	/**
	 * Get the number of updates per second.
	 */
	int GetUpdatesPerSec();
	/**
	 * Get the size of each step in seconds.
	 */
	float GetStepSize();

private:
	void Update();

	double mlLocalTime;
	double mlLocalTimeAdd;

	int mlMaxUpdates;
	int mlUpdateCount;

	LowLevelSystem *mpLowLevelSystem;
};

} // namespace hpl

#endif // HPL_LOGICTIMER_H
