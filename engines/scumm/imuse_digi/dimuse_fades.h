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

#if !defined(SCUMM_IMUSE_DIGI_FADES_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_FADES_H

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Scumm {

class IMuseDigiFadesHandler {

private:
	IMuseDigital *_engine;
	IMuseDigiFade _fades[DIMUSE_MAX_FADES] = {};
	int _fadesOn = 0;

	void clearAllFades();
public:
	IMuseDigiFadesHandler(IMuseDigital *engine);
	~IMuseDigiFadesHandler();

	int init();
	void deinit();
	void saveLoad(Common::Serializer &ser);
	int fadeParam(int soundId, int opcode, int destinationValue, int fadeLength);
	void clearFadeStatus(int soundId, int opcode);
	void loop();

};

} // End of namespace Scumm
#endif
