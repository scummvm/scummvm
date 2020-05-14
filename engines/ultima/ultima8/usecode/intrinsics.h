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

#ifndef ULTIMA8_USECODE_INTRINSICS_H
#define ULTIMA8_USECODE_INTRINSICS_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima8 {

typedef uint32(*Intrinsic)(const uint8 *args, unsigned int argsize);

#define INTRINSIC(x) static uint32 x (const uint8* args, unsigned int argsize)

// TODO: range checking on args

#define ARG_UINT8(x)   uint8  x = (*args++);
#define ARG_UINT16(x)  uint16 x = (*args++); x += ((*args++) << 8);
#define ARG_UINT32(x)  uint32 x = (*args++); x += ((*args++) << 8); \
	x+= ((*args++) << 16); x += ((*args++) << 24);
#define ARG_SINT8(x)   int8  x = (*args++);
#define ARG_SINT16(x)  int16 x = (*args++); x += ((*args++) << 8);
#define ARG_SINT32(x)  int32 x = (*args++); x += ((*args++) << 8); \
	x+= ((*args++) << 16); x += ((*args++) << 24);
#define ARG_UC_PTR(x)  uint32 x = (*args++); x += ((*args++) << 8); \
	x+= ((*args++) << 16); x += ((*args++) << 24);
#define ARG_OBJID(x)  ObjId   x = (*args++); x += ((*args++) << 8);
#define ARG_PROCID(x) ProcId  x = (*args++); x += ((*args++) << 8);

#define ARG_OBJECT_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
	uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
	Object* x = getObject(id_##x);
#define ARG_OBJECT_FROM_ID(x) ARG_OBJID(id_##x); \
	Object* x = getObject(id_##x);

#define ARG_ITEM_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
	uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
	Item* x = getItem(id_##x);
#define ARG_ITEM_FROM_ID(x)   ARG_OBJID(id_##x); \
	Item* x = getItem(id_##x);

#define ARG_CONTAINER_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
	uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
	Container* x = getContainer(id_##x);
#define ARG_CONTAINER_FROM_ID(x) ARG_OBJID(id_##x); \
	Container* x = getContainer(id_##x);

#define ARG_ACTOR_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
	uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
	Actor* x = getActor(id_##x);
#define ARG_ACTOR_FROM_ID(x)  ARG_OBJID(id_##x); \
	Actor* x = getActor(id_##x);

#define ARG_EGG_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
	uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
	Egg* x = dynamic_cast<Egg*>(getObject(id_##x));
#define ARG_EGG_FROM_ID(x)    ARG_OBJID(id_##x); \
	Egg* x = dynamic_cast<Egg*>(getObject(id_##x));

#define ARG_STRING(x) ARG_UC_PTR(ucptr_##x); \
	uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
	Std::string x = UCMachine::get_instance()->getString(id_##x);

#define ARG_LIST(x)   ARG_UINT16(id_##x); \
	UCList* x = UCMachine::get_instance()->getList(id_##x);

#define ARG_WORLDPOINT(x) ARG_UC_PTR(ucptr_##x); \
	WorldPoint x; \
	UCMachine::get_instance()->dereferencePointer(ucptr_##x, x._buf, 5);

#define ARG_NULL8()  args+=1;
#define ARG_NULL16() args+=2;
#define ARG_NULL32() args+=4;

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
