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

#ifndef ILLUSIONS_SPECIALCODE_H
#define ILLUSIONS_SPECIALCODE_H

#include "illusions/resourcesystem.h"

namespace Illusions {

class IllusionsEngine;
struct OpCall;

class SpecialCodeLoader : public BaseResourceLoader {
public:
	SpecialCodeLoader(IllusionsEngine *vm) : _vm(vm) {}
	~SpecialCodeLoader() override {}
	void load(Resource *resource) override;
	virtual void unload(Resource *resource);
	virtual void buildFilename(Resource *resource);
	bool isFlag(int flag) override;
protected:
	IllusionsEngine *_vm;
};

class SpecialCode {
public:
	virtual ~SpecialCode() {}
	virtual void init() = 0;
	virtual void run(uint32 specialCodeId, OpCall &opCall) = 0;
	virtual void resetBeforeResumeSavegame() {};
};

} // End of namespace Illusions

#endif // ILLUSIONS_SPECIALCODE_H
