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

#ifndef MADS_CONVERSATIONS_H
#define MADS_CONVERSATIONS_H

namespace MADS {

class MADSEngine;

class GameConversation {
private:
	MADSEngine *_vm;

public:
	/**
	 * Constructor
	 */
	GameConversation(MADSEngine *vm);

	/**
	 * Destructor
	 */
	virtual ~GameConversation();

	int* getVariable(int idx);

	void get(int id);
	void run(int id);
	void stop();
	void exportPointer(int *val);
	void exportValue(int val);
	void setHeroTrigger(int val);
	void setInterlocutorTrigger(int val);
	void hold();
	void release();

	int _running;
	int _restoreRunning;
};

} // End of namespace MADS

#endif /* MADS_CONVERSATIONS_H */
