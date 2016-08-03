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

#ifndef RIVEN_CARD_H
#define RIVEN_CARD_H

#include "mohawk/riven_scripts.h"

#include "common/system.h"

namespace Mohawk {

class RivenCard {
public:
	RivenCard(MohawkEngine_Riven *vm, uint16 id);
	~RivenCard();

	void open();
	void initializeZipMode();
	void runScript(uint16 scriptType);

	uint16 getId() const;

private:
	void loadCardResource(uint16 id);

	MohawkEngine_Riven *_vm;

	uint16 _id;
	int16 _name;
	uint16 _zipModePlace;
	RivenScriptList _scripts;
};

} // End of namespace Mohawk

#endif
