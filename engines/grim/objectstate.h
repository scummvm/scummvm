/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_OSTATE_H
#define GRIM_OSTATE_H

#include "common/endian.h"

#include "engines/grim/pool.h"
#include "engines/grim/bitmap.h"

namespace Grim {

class SaveGame;

class ObjectState : public PoolObject<ObjectState> {
public:
	enum Position {
		OBJSTATE_BACKGROUND = 0,
		OBJSTATE_UNDERLAY = 1,
		OBJSTATE_OVERLAY = 2,
		OBJSTATE_STATE = 3
	};

	ObjectState(int setupID, ObjectState::Position pos, const char *bitmap, const char *zbitmap, bool visible);
	ObjectState();
	~ObjectState();

	static int32 getStaticTag() { return MKTAG('S', 'T', 'A', 'T'); }

	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);

	int getSetupID() const { return _setupID; }
	Position getPos() const { return _pos; }
	void setPos(Position position) { _pos = position; }

	const Common::String &getBitmapFilename() const;

	void setActiveImage(int val);
	void draw();

private:
	bool _visibility;
	int _setupID;
	Position _pos;
	Bitmap::Ptr _bitmap, _zbitmap;
};

} // end of namespace Grim

#endif
