/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_BASE_ANIMATION_TRANSITION_TIME_H
#define WINTERMUTE_BASE_ANIMATION_TRANSITION_TIME_H

#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

class BaseAnimationTransitionTime {
public:
	BaseAnimationTransitionTime(const Common::String &from, const Common::String &to, uint32 Time);
	BaseAnimationTransitionTime();
	virtual ~BaseAnimationTransitionTime();
	bool persist(BasePersistenceManager *persistMgr);

	Common::String _animFrom;
	Common::String _animTo;
	uint32 _time;
};

}

#endif
