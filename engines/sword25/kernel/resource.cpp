// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#include "sword25/kernel/resource.h"
#include "sword25/kernel/string.h"
#include "sword25/kernel/kernel.h"
#include "sword25/package/packagemanager.h"

#define BS_LOG_PREFIX "RESOURCE"

BS_Resource::BS_Resource(const std::string& FileName, RESOURCE_TYPES Type) :
	_Type(Type),
	_RefCount(0)
{
	BS_ASSERT(BS_Kernel::GetInstance()->GetService("package"));

	_FileName = static_cast<BS_PackageManager *>(BS_Kernel::GetInstance()->GetService("package"))->GetAbsolutePath(FileName);
	_FileNameHash = BS_String::GetHash(FileName);
};

void BS_Resource::Release()
{
	if (_RefCount)
	{
		--_RefCount;
	}
	else
		BS_LOG_WARNINGLN("Released unlocked resource \"%s\".", _FileName.c_str());
}
