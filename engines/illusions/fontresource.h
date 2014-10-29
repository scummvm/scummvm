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

#ifndef ILLUSIONS_FONTRESOURCE_H
#define ILLUSIONS_FONTRESOURCE_H

#include "illusions/graphics.h"
#include "illusions/resourcesystem.h"

namespace Illusions {

class IllusionsEngine;

class FontResourceLoader : public BaseResourceLoader {
public:
	FontResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	virtual ~FontResourceLoader() {}
	virtual void load(Resource *resource);
	virtual void unload(Resource *resource);
	virtual void buildFilename(Resource *resource);
	virtual bool isFlag(int flag);
protected:
	IllusionsEngine *_vm;
};

} // End of namespace Illusions

#endif // ILLUSIONS_FONTRESOURCE_H
