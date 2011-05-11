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
 */

// this is the object manager. our equivalent to protocol.c and coredata.c

#ifndef SWORD1_OBJECTMAN_H
#define SWORD1_OBJECTMAN_H

#include "sword1/resman.h"
#include "sword1/sworddefs.h"
#include "sword1/object.h"

namespace Sword1 {

class ObjectMan {
public:
	ObjectMan(ResMan *pResourceMan);
	~ObjectMan();
	void initialize();

	Object *fetchObject(uint32 id);
	uint32 fetchNoObjects(int section);
	bool sectionAlive(uint16 section);
	void megaEntering(uint16 section);
	void megaLeaving(uint16 section, int id);

	uint8 fnCheckForTextLine(uint32 textId);
	char *lockText(uint32 textId);
	void unlockText(uint32 textId);
	uint32 lastTextNumber(int section);

	void closeSection(uint32 screen);

	void saveLiveList(uint16 *dest); // for loading/saving
	void loadLiveList(uint16 *src);
private:
	ResMan *_resMan;
	static const uint32 _objectList[TOTAL_SECTIONS];	//a table of pointers to object files
	static const uint32 _textList[TOTAL_SECTIONS][7];	//a table of pointers to text files
	uint16	_liveList[TOTAL_SECTIONS];					//which sections are active
	uint8 *_cptData[TOTAL_SECTIONS];
	static char _missingSubTitleStr[];
	static const char *_translationId2950145[7];		//translation for textId 2950145 (missing from cluster file for some langages)
};

} // End of namespace Sword1

#endif //OBJECTMAN_H
