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

#include "titanic/objects/saveable_object.h"
#include "titanic/objects/file_item.h"
#include "titanic/objects/list.h"
#include "titanic/objects/message_target.h"
#include "titanic/objects/movie_clip.h"
#include "titanic/objects/project_item.h"
#include "titanic/objects/tree_item.h"
#include "titanic/rooms/room_item.h"

namespace Titanic {

Common::HashMap<Common::String, CSaveableObject::CreateFunction> * 
	CSaveableObject::_classList = nullptr;

#define DEFFN(T) CSaveableObject *Function##T() { return new T(); }
#define ADDFN(T) (*_classList)[#T] = Function##T

DEFFN(CFileListItem);
DEFFN(CMessageTarget);
DEFFN(CTreeItem);
DEFFN(CFileItem);
DEFFN(CProjectItem);
DEFFN(CRoomItem);
DEFFN(CMovieClip);
DEFFN(CMovieClipItem);
DEFFN(CMovieClipList);

void CSaveableObject::initClassList() {
	_classList = new Common::HashMap<Common::String, CreateFunction>();
	ADDFN(CFileListItem);
	ADDFN(CMessageTarget);
	ADDFN(CTreeItem);
	ADDFN(CFileItem);
	ADDFN(CProjectItem);
	ADDFN(CRoomItem);
	ADDFN(CMovieClip);
	ADDFN(CMovieClipItem);
	ADDFN(CMovieClipList);
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
