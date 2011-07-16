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

#ifndef NEVERHOOD_MODULE2300_H
#define NEVERHOOD_MODULE2300_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module2300

class Module2300 : public Module {
public:
	Module2300(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2300();
protected:
	// TODO ResourceTable _resourceTable1;
	// TODO ResourceTable _resourceTable2;
	bool _flag;
	int _volume;
	void createScene2301(int which);			
	void createScene2302(int which);			
	void createScene2303(int which);			
	void createScene2304(int which);			
	void createScene2305(int which);			
	void updateScene2301();			
	void updateScene2302();			
	void updateScene2303();			
	void updateScene2304();			
	void updateScene2305();			
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2300_H */
