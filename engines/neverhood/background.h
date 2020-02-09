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

#ifndef NEVERHOOD_BACKGROUND_H
#define NEVERHOOD_BACKGROUND_H

#include "neverhood/neverhood.h"
#include "neverhood/entity.h"
#include "neverhood/graphics.h"
#include "neverhood/resource.h"

namespace Neverhood {

class Background : public Entity {
public:
	Background(NeverhoodEngine *vm, int objectPriority);
	Background(NeverhoodEngine *vm, uint32 fileHash, int objectPriority, int surfacePriority);
	~Background() override;
	BaseSurface *getSurface() { return _surface; }
	void createSurface(int surfacePriority, int16 width, int16 height);
	void load(uint32 fileHash);
	SpriteResource& getSpriteResource() { return _spriteResource; }
protected:
	BaseSurface *_surface;
	SpriteResource _spriteResource;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_BACKGROUND_H */
