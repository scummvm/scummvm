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

#include "titanic/simple_file.h"

namespace Titanic {

class SoundManager {
protected:
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
public:
	SoundManager();
	
	virtual int proc3() const { return 0; }
	virtual int proc4() const { return 0; }
	virtual int proc5() const { return 0; }
	virtual void proc6() = 0;
	virtual void proc7() = 0;
	virtual void proc8() = 0;
	virtual void proc9() {}
	virtual void proc10() = 0;
	virtual void proc11() = 0;
	virtual void proc12() {}
	virtual void proc13() {}
	virtual void proc14() = 0;
	virtual int proc15() const { return 0; }
	virtual int proc16() const { return 0; }
	virtual void WaveMixPump() {}
	virtual int proc18() const { return 0; }
	virtual void proc19(int v) { _field4 = v; }
	virtual void proc20(int v) { _field8 = v; }
	virtual void proc21(int v) { _fieldC = v; }
	virtual void proc22(int v) { _field10 = v; }

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

	virtual void proc26() {}

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

class QSoundManager : public SoundManager {
public:
	int _field18;
	int _field1C;

	int _field4A0[16];
public:
	QSoundManager();

	virtual int proc3();
	virtual int proc4();
	virtual int proc5();
	virtual void proc6();
	virtual void proc7();
	virtual void proc8();
	virtual void proc9();
	virtual void proc10();
	virtual void proc11();
	virtual void proc12();
	virtual void proc13();
	virtual void proc14();
	virtual int proc15();
	virtual int proc16();
	virtual void WaveMixPump();
	virtual int proc18() const;
	virtual void proc19(int v);
	virtual void proc20(int v);
	virtual void proc21(int v);

	virtual void proc29();
	virtual void proc30();
};

} // End of namespace Titanic

#endif /* TITANIC_QSOUND_MANAGER_H */
