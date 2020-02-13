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

#ifndef ADL_ADL_V3_H
#define ADL_ADL_V3_H

#include "adl/adl_v2.h"

namespace Adl {

class AdlEngine_v3 : public AdlEngine_v2 {
public:
	~AdlEngine_v3() override { }

protected:
	AdlEngine_v3(OSystem *syst, const AdlGameDescription *gd);

	// AdlEngine
	Common::String getItemDescription(const Item &item) const override;

	void loadItemDescriptions(Common::SeekableReadStream &stream, byte count);

	int o_isNounNotInRoom(ScriptEnv &e) override;

	Common::Array<Common::String> _itemDesc;
};

} // End of namespace Adl

#endif
