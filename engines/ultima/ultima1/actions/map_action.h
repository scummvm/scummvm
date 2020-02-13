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

#ifndef ULTIMA_ULTIMA1_ACTIONS_MAP_ACTION_H
#define ULTIMA_ULTIMA1_ACTIONS_MAP_ACTION_H

namespace Ultima {
namespace Ultima1 {
namespace Actions {

#define MAP_ACTION(NAME, ACTION_NUM, MAP_METHOD) \
	using Shared::C##NAME##Msg; \
	class NAME : public Action { DECLARE_MESSAGE_MAP; bool NAME##Msg(C##NAME##Msg &msg) { \
	addInfoMsg(getRes()->ACTION_NAMES[ACTION_NUM], false); \
	getMap()->MAP_METHOD(); \
	return true; } \
	public: \
	CLASSDEF; \
	NAME(TreeItem *parent) : Action(parent) {} \
	}; \
	BEGIN_MESSAGE_MAP(NAME, Action) ON_MESSAGE(NAME##Msg) END_MESSAGE_MAP()

#define MAP_ACTION_END_TURN(NAME, ACTION_NUM, MAP_METHOD) \
	using Shared::C##NAME##Msg; \
	class NAME : public Action { DECLARE_MESSAGE_MAP; bool NAME##Msg(C##NAME##Msg &msg) { \
	addInfoMsg(getRes()->ACTION_NAMES[ACTION_NUM], false); \
	getMap()->MAP_METHOD(); \
	endOfTurn(); \
	return true; } \
	public: \
	CLASSDEF; \
	NAME(TreeItem *parent) : Action(parent) {} \
	}; \
	BEGIN_MESSAGE_MAP(NAME, Action) ON_MESSAGE(NAME##Msg) END_MESSAGE_MAP()

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
