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

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "ANIMATIONTEMPLATEREGISTRY"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/gfx/animationtemplateregistry.h"
#include "sword25/gfx/animationtemplate.h"

namespace Sword25 {

Common::ScopedPtr<AnimationTemplateRegistry> AnimationTemplateRegistry::m_InstancePtr;

void AnimationTemplateRegistry::LogErrorLn(const char *Message) const {
	BS_LOG_ERRORLN(Message);
}

// -----------------------------------------------------------------------------

void AnimationTemplateRegistry::LogWarningLn(const char *Message) const {
	BS_LOG_WARNINGLN(Message);
}

// -----------------------------------------------------------------------------

bool AnimationTemplateRegistry::Persist(OutputPersistenceBlock &Writer) {
	bool Result = true;

	// Das nächste zu vergebene Handle schreiben.
	Writer.Write(m_NextHandle);

	// Anzahl an BS_AnimationTemplates schreiben.
	Writer.Write(m_Handle2PtrMap.size());

	// Alle BS_AnimationTemplates persistieren.
	HANDLE2PTR_MAP::const_iterator Iter = m_Handle2PtrMap.begin();
	while (Iter != m_Handle2PtrMap.end()) {
		// Handle persistieren.
		Writer.Write(Iter->_key);

		// Objekt persistieren.
		Result &= Iter->_value->Persist(Writer);

		++Iter;
	}

	return Result;
}

// -----------------------------------------------------------------------------

bool AnimationTemplateRegistry::Unpersist(InputPersistenceBlock &Reader) {
	bool Result = true;

	// Das nächste zu vergebene Handle wieder herstellen.
	Reader.Read(m_NextHandle);

	// Alle vorhandenen BS_AnimationTemplates zerstören.
	while (!m_Handle2PtrMap.empty())
		delete m_Handle2PtrMap.begin()->_value;

	// Anzahl an BS_AnimationTemplates einlesen.
	unsigned int AnimationTemplateCount;
	Reader.Read(AnimationTemplateCount);

	// Alle gespeicherten BS_AnimationTemplates wieder herstellen.
	for (unsigned int i = 0; i < AnimationTemplateCount; ++i) {
		// Handle lesen.
		unsigned int Handle;
		Reader.Read(Handle);

		// BS_AnimationTemplate wieder herstellen.
		Result &= (AnimationTemplate::Create(Reader, Handle) != 0);
	}

	return Reader.IsGood() && Result;
}

} // End of namespace Sword25
