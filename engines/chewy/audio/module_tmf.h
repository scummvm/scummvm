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

#ifndef CHEWY_MUSIC_MODULE_TMF_H
#define CHEWY_MUSIC_MODULE_TMF_H

#include "audio/mods/module.h"

namespace Chewy {

// Module variant for the MOD-like TMF format used by Chewy.
class Module_TMF : public Modules::Module {
protected:
	static const uint8 TMF_MOD_SONG_NAME[21];
	static const uint8 TMF_MOD_INSTRUMENT_NAME[23];
	static const uint16 TMF_MOD_PERIODS[36];

public:
	bool load(Common::SeekableReadStream &stream, int offs) override;
};

} // End of namespace Chewy

#endif
