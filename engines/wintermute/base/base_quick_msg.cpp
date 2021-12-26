/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_quick_msg.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseQuickMsg::BaseQuickMsg(uint32 startTime, const char *text) {
	_text = text;
	_startTime = startTime;
}


//////////////////////////////////////////////////////////////////////////
BaseQuickMsg::~BaseQuickMsg() {
}


//////////////////////////////////////////////////////////////////////////
const char *BaseQuickMsg::getText() const {
	return _text.c_str();
}

//////////////////////////////////////////////////////////////////////////
uint32 BaseQuickMsg::getStartTime() const {
	return _startTime;
}

} // End of namespace Wintermute
