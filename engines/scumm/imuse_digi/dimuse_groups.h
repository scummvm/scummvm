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

#if !defined(SCUMM_IMUSE_DIGI_GROUPS_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_GROUPS_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/serializer.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "scumm/imuse_digi/dimuse_engine.h"

namespace Scumm {

class IMuseDigiGroupsHandler {

private:
	IMuseDigital *_engine;
	int _effVols[DIMUSE_MAX_GROUPS] = {};
	int _vols[DIMUSE_MAX_GROUPS] = {};
public:
	IMuseDigiGroupsHandler(IMuseDigital *engine);
	~IMuseDigiGroupsHandler();
	int init();
	int setGroupVol(int id, int volume);
	int getGroupVol(int id);
};

} // End of namespace Scumm
#endif
