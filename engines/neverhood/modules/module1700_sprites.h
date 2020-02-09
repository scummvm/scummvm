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

#ifndef NEVERHOOD_MODULES_MODULE1700_SPRITES_H
#define NEVERHOOD_MODULES_MODULE1700_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class SsScene1705WallSymbol : public StaticSprite {
public:
	SsScene1705WallSymbol(NeverhoodEngine *vm, uint32 fileHash, int symbolIndex);
};

class SsScene1705Tape : public StaticSprite {
public:
	SsScene1705Tape(NeverhoodEngine *vm, Scene *parentScene, uint32 tapeIndex, int surfacePriority, int16 x, int16 y, uint32 fileHash);
protected:
	Scene *_parentScene;
	uint32 _tapeIndex;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene1705 : public Klaymen {
public:
	KmScene1705(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1700_SPRITES_H */
