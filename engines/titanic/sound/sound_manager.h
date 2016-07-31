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

#ifndef TITANIC_SOUND_MANAGER_H
#define TITANIC_SOUND_MANAGER_H

#include "titanic/support/simple_file.h"

namespace Titanic {

class CSoundManager {
protected:
	double _musicPercent;
	double _speechPercent;
	double _masterPercent;
	double _parrotPercent;
	int _field14;
public:
	CSoundManager();
	virtual ~CSoundManager() {}
	
	/**
	 * Loads a sound
	 * @param name		Name of sound resource
	 * @returns			Loaded sound handle
	 */
	virtual int loadSound(const CString &name) { return 0; }

	virtual int proc4() const { return 0; }
	virtual int proc5() const { return 0; }
	virtual void proc6() = 0;
	virtual void proc7() = 0;
	virtual void proc8(int v) = 0;
	virtual void proc9() {}
	virtual void proc10() = 0;
	virtual void proc11() = 0;
	virtual void proc12() {}
	virtual void proc13() {}
	virtual bool proc14() = 0;
	virtual bool isActive(int handle) const { return false; }
	virtual int proc16() const { return 0; }
	virtual void WaveMixPump() {}
	
	/**
	 * Called when a movie with audio is started
	 */
	virtual bool movieStarted() const { return false; }

	virtual void setMusicPercent(double percent) { _musicPercent = percent; }
	virtual void setSpeechPercent(double percent) { _speechPercent = percent; }
	virtual void setMasterPercent(double percent) { _masterPercent = percent; }
	virtual void setParrotPercent(double percent) { _parrotPercent = percent; }

	/**
	 * Called when a game is about to be loaded
	 */
	virtual void preLoad() { proc10(); }

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) {}

	/**
	 * Called after loading of a game is completed
	 */
	virtual void postLoad() {}

	/**
	 * Called when a game is about to be saved
	 */
	virtual void preSave() {}

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file) const {}

	/**
	 * Called after saving is complete
	 */
	virtual void postSave() {}
	
	virtual void proc29() {}
};

class QSoundManager : public CSoundManager {
public:
	int _field18;
	int _field1C;

	int _field4A0[16];
public:
	QSoundManager();

	/**
	 * Loads a sound
	 * @param name		Name of sound resource
	 * @returns			Loaded sound handle
	 */
	virtual int loadSound(const CString &name);

	virtual int proc4() const;
	virtual int proc5() const;
	virtual void proc6();
	virtual void proc7();
	virtual void proc8(int v);
	virtual void proc9();
	virtual void proc10();
	virtual void proc11();
	virtual void proc12();
	virtual void proc13();
	virtual bool proc14();
	virtual bool isActive(int handle) const;
	virtual int proc16() const;
	virtual void WaveMixPump();

	
	/**
	 * Called when a movie with audio is started
	 */
	virtual bool movieStarted() const;

	virtual void proc19(int v);
	virtual void proc20(int v);
	virtual void proc21(int v);

	virtual void proc29();
	virtual void proc30();
};

} // End of namespace Titanic

#endif /* TITANIC_QSOUND_MANAGER_H */
