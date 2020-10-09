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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_NAMED_OBJECT_H
#define WINTERMUTE_BASE_NAMED_OBJECT_H


#include "engines/wintermute/base/base.h"

namespace Wintermute {

class BaseNamedObject : public BaseClass {
#ifndef ENABLE_WME3D
	char *_name;
#endif
public:
	BaseNamedObject(BaseGame *inGame);
	BaseNamedObject();
	~BaseNamedObject(void) override;
	BaseNamedObject(TDynamicConstructor, TDynamicConstructor);

#ifdef ENABLE_WME3D
	// making this public is a quick hack for loading/saving the name variable
	// via the persistence manager in the AdSceneGeometry class
	// we could make this class here persistable as well,
	// but the relevant subclasses don't persist their bases at the moment
	char *_name;
#endif

	const char *getName() const { return _name; }
	void setName(const char *name);
};

} // End of namespace Wintermute

#endif
