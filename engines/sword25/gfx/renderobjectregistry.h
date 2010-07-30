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

#ifndef SWORD25_RENDEROBJECTREGISTRY_H
#define SWORD25_RENDEROBJECTREGISTRY_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/objectregistry.h"

#include "sword25/kernel/memlog_off.h"
#include <memory>
#include "sword25/kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Forward Deklarationen
// -----------------------------------------------------------------------------

class BS_RenderObject;

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_RenderObjectRegistry : public BS_ObjectRegistry<BS_RenderObject>
{
public:
	static BS_RenderObjectRegistry & GetInstance()
	{
		if (!m_InstancePtr.get()) m_InstancePtr.reset(new BS_RenderObjectRegistry);
		return *m_InstancePtr.get();
	}

	virtual ~BS_RenderObjectRegistry() {}

private:
	virtual void LogErrorLn(const char * Message) const;
	virtual void LogWarningLn(const char * Message) const;

	static std::auto_ptr<BS_RenderObjectRegistry> m_InstancePtr;
};

#endif
