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
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/kernel/resource.h"
#include "sword25/kernel/string.h"
#include "sword25/kernel/kernel.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

#define BS_LOG_PREFIX "RESOURCE"

BS_Resource::BS_Resource(const Common::String &FileName, RESOURCE_TYPES Type) :
		_Type(Type),
		_RefCount(0) {
	BS_ASSERT(BS_Kernel::GetInstance()->GetService("package"));

	_FileName = static_cast<BS_PackageManager *>(BS_Kernel::GetInstance()->GetService("package"))->GetAbsolutePath(FileName);
	_FileNameHash = BS_String::GetHash(FileName);
}

void BS_Resource::Release() {
	if (_RefCount) {
		--_RefCount;
	} else
		BS_LOG_WARNINGLN("Released unlocked resource \"%s\".", _FileName.c_str());
}

} // End of namespace Sword25
