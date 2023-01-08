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

#ifndef TETRAEDGE_TE_TE_OBJECT_H
#define TETRAEDGE_TE_TE_OBJECT_H

#include <common/array.h>

namespace Tetraedge {

class TeObject {
public:
	TeObject();

	virtual ~TeObject() {}

	/** Add this object to the pending deletion queue (deleted at the end of a frame) */
	void deleteLater();

	/** Delete all pending objects now */
	static void deleteNow();

	static void cleanup();

private:
	static Common::Array<TeObject *> *_pendingDeleteList;
	static Common::Array<TeObject *> *pendingDeleteList();

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_OBJECT_H
