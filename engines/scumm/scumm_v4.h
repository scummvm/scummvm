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
 * $URL$
 * $Id$
 *
 */

#ifndef SCUMM_SCRIPT_V4_H
#define SCUMM_SCRIPT_V4_H

#include "scumm/scumm_v5.h"

namespace Scumm {

/**
 * Engine for version 4 SCUMM games; GF_SMALL_HEADER is always set for these.
 */
class ScummEngine_v4 : public ScummEngine_v5 {
public:
	ScummEngine_v4(OSystem *syst, const DetectorResult &dr);

	virtual void resetScumm();

protected:
	virtual void readResTypeList(int id);
	virtual void readIndexFile();
	virtual void loadCharset(int no);
	virtual void resetRoomObjects();
	virtual void readMAXS(int blockSize);
	virtual void readGlobalObjects();

	virtual void resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);
};


} // End of namespace Scumm

#endif
