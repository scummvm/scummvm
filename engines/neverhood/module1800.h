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

#ifndef NEVERHOOD_MODULE1800_H
#define NEVERHOOD_MODULE1800_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module1800

class Module1800 : public Module {
public:
	Module1800(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1800();
protected:
	// TODO ResourceTable _resourceTable;
	void createScene1801(int which);			
	void createScene1802(int which);			
	void createScene1803(int which);			
	void createScene1804(int which);
	void createScene1804b(int which);			
	void createScene1805(int which);			
	void createScene1806(int which);			
	void createScene1807(int which);			
	void createScene1808(int which);			
	void createScene1809(int which);			
	void updateScene1801();			
	void updateScene1802();			
	void updateScene1803();			
	void updateScene1804();			
	void updateScene1805();			
	void updateScene1806();			
	void updateScene1807();			
	void updateScene1808();			
	void updateScene1809();			
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1800_H */
