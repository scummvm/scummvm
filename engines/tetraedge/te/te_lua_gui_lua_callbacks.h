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

#ifndef TETRAEDGE_TE_TE_LUA_GUI_LUA_CALLBACKS_H
#define TETRAEDGE_TE_TE_LUA_GUI_LUA_CALLBACKS_H

struct lua_State;

namespace Tetraedge {

int layoutBindings(lua_State *state);
int listLayoutBindings(lua_State *state);
int spriteLayoutBindings(lua_State *state);
int buttonLayoutBindings(lua_State *state);
int checkboxLayoutBindings(lua_State *state);
int layoutPositionLinearAnimationBindings(lua_State *state);
int layoutAnchorLinearAnimationBindings(lua_State *state);
int textLayoutBindings(lua_State *state);
int clipLayoutBindings(lua_State *state);
int colorLinearAnimationBindings(lua_State *state);
int rotationLinearAnimationBindings(lua_State *state);
int scrollingLayoutBindings(lua_State *state);
int extendedTextLayoutBindings(lua_State *state);


} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_LUA_GUI_LUA_CALLBACKS_H
