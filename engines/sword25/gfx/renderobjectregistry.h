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
