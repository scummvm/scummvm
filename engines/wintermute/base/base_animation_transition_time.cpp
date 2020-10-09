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

#include "engines/wintermute/base/base_animation_transition_time.h"
#include "engines/wintermute/persistent.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseAnimationTransitionTime::BaseAnimationTransitionTime(const Common::String &from, const Common::String &to, uint32 time) :
	_animFrom(from), _animTo(to), _time(time) {

}

//////////////////////////////////////////////////////////////////////////
BaseAnimationTransitionTime::BaseAnimationTransitionTime() {
	_time = 0;
}

//////////////////////////////////////////////////////////////////////////
BaseAnimationTransitionTime::~BaseAnimationTransitionTime() {
}


//////////////////////////////////////////////////////////////////////////
bool BaseAnimationTransitionTime::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferString(TMEMBER(_animFrom));
	persistMgr->transferString(TMEMBER(_animTo));
	persistMgr->transferUint32(TMEMBER(_time));

	return true;
}

}
