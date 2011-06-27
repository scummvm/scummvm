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
 */

// TODO: I couldn't come up with a better name than 'Module' so far

#ifndef NEVERHOOD_GAMEMODULE_H
#define NEVERHOOD_GAMEMODULE_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"

namespace Neverhood {

class GameModule : public Module {
public:
	GameModule(NeverhoodEngine *vm);
	virtual ~GameModule();
protected:
	Entity *_prevChildObject;
	bool _someFlag1;
	bool _field2C;
	uint32 _counter;
	/* TODO
	ResourceTable _resourceTable1;
	ResourceTable _resourceTable2;
	ResourceTable _resourceTable3;
	ResourceTable _resourceTable4;
	*/
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void startup();
	void createModule1500(int which);
	void updateModule1500();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE_H */
