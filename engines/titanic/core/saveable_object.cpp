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

#include "titanic/core/saveable_object.h"
#include "titanic/core/file_item.h"
#include "titanic/core/link_item.h"
#include "titanic/core/list.h"
#include "titanic/core/message_target.h"
#include "titanic/core/movie_clip.h"
#include "titanic/core/node_item.h"
#include "titanic/core/project_item.h"
#include "titanic/core/saveable_object.h"
#include "titanic/core/tree_item.h"
#include "titanic/core/view_item.h"
#include "titanic/game/announce.h"
#include "titanic/game/pet_position.h"
#include "titanic/game/room_item.h"
#include "titanic/game/service_elevator_door.h"
#include "titanic/game/sub_glass.h"
#include "titanic/npcs/barbot.h"
#include "titanic/npcs/bellbot.h"
#include "titanic/npcs/deskbot.h"
#include "titanic/npcs/doorbot.h"
#include "titanic/npcs/liftbot.h"
#include "titanic/npcs/maitre_d.h"
#include "titanic/npcs/mobile.h"
#include "titanic/npcs/parrot.h"
#include "titanic/npcs/starlings.h"
#include "titanic/npcs/succubus.h"
#include "titanic/npcs/titania.h"

namespace Titanic {

Common::HashMap<Common::String, CSaveableObject::CreateFunction> * 
	CSaveableObject::_classList = nullptr;

#define DEFFN(T) CSaveableObject *Function##T() { return new T(); }
#define ADDFN(T) (*_classList)[#T] = Function##T

DEFFN(CAnnounce);
DEFFN(CBarbot);
DEFFN(CBellBot);
DEFFN(CDeskbot);
DEFFN(CDoorbot);
DEFFN(CFileItem);
DEFFN(CFileListItem);
DEFFN(CLiftBot);
DEFFN(CLinkItem);
DEFFN(CMaitreD);
DEFFN(CMessageTarget);
DEFFN(CMobile);
DEFFN(CMovieClip);
DEFFN(CMovieClipList);
DEFFN(CNodeItem);
DEFFN(CParrot);
DEFFN(CPETPosition);
DEFFN(CProjectItem);
DEFFN(CRoomItem);
DEFFN(CServiceElevatorDoor);
DEFFN(CStarlings);
DEFFN(CSUBGlass);
DEFFN(CSuccUBus);
DEFFN(CTitania);
DEFFN(CTreeItem);
DEFFN(CViewItem);

void CSaveableObject::initClassList() {
	_classList = new Common::HashMap<Common::String, CreateFunction>();
	ADDFN(CAnnounce);
	ADDFN(CBarbot);
	ADDFN(CBellBot);
	ADDFN(CDeskbot);
	ADDFN(CDoorbot);
	ADDFN(CFileItem);
	ADDFN(CFileListItem);
	ADDFN(CLiftBot);
	ADDFN(CLinkItem);
	ADDFN(CMaitreD);
	ADDFN(CMessageTarget);
	ADDFN(CMobile);
	ADDFN(CMovieClip);
	ADDFN(CMovieClipList);
	ADDFN(CNodeItem);
	ADDFN(CParrot);
	ADDFN(CPETPosition);
	ADDFN(CProjectItem);
	ADDFN(CRoomItem);
	ADDFN(CServiceElevatorDoor);
	ADDFN(CStarlings);
	ADDFN(CSUBGlass);
	ADDFN(CSuccUBus);
	ADDFN(CTitania);
	ADDFN(CTreeItem);
	ADDFN(CViewItem);
}

void CSaveableObject::freeClassList() {
	delete _classList;
}

CSaveableObject *CSaveableObject::createInstance(const Common::String &name) {
	return (*_classList)[name]();
}

void CSaveableObject::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
}

void CSaveableObject::load(SimpleFile *file) {
	file->readNumber();
}

void CSaveableObject::saveHeader(SimpleFile *file, int indent) const {
	file->writeClassStart(getClassName(), indent);
}

void CSaveableObject::saveFooter(SimpleFile *file, int indent) const {
	file->writeClassEnd(indent);
}

} // End of namespace Titanic
