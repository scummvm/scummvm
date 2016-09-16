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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"

namespace Fullpipe {

bool PicAniInfo::save(Common::WriteStream *file) {
	debugC(5, kDebugLoading, "PicAniInfo::save()");

	file->writeUint32LE(type);
	file->writeUint16LE(objectId);
	file->writeUint16LE(field_6);
	file->writeUint32LE(field_8);
	file->writeUint16LE(sceneId);
	file->writeUint16LE(field_E);
	file->writeSint32LE(ox);
	file->writeSint32LE(oy);
	file->writeUint32LE(priority);
	file->writeUint16LE(staticsId);
	file->writeUint16LE(movementId);
	file->writeUint16LE(dynamicPhaseIndex);
	file->writeUint16LE(flags);
	file->writeUint32LE(field_24);
	file->writeUint32LE(someDynamicPhaseIndex);

	return true;
}

} // End of namespace Fullpipe
