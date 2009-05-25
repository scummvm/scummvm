/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef LUA_HH
#define LUA_HH

#include "engines/grim/lua/lua.h"

namespace Graphics {
	class Vector3d;
};

namespace Grim {

// Register Residual builtin functions and structures
void register_lua();

// Like dofile, except it loads the file to execute from a bundle file
int bundle_dofile(const char *filename);

// Set system.frameTime
void setFrameTime(float frameTime);

// Set smush.movieTime
void setMovieTime(float movieTime);

// Get the event handler function with the given name, pushing the handler
// object if appropriate
lua_Object getEventHandler(const char *name);

// set the value for a table item
void setTableValue(lua_Object table, const char *name, int newvalue);
void setTableValue(lua_Object table, const char *name, lua_Object newvalue);

// get the value of a table item
lua_Object getTableValue(lua_Object table, const char *name);
lua_Object getIndexedTableValue(lua_Object table, int index);

// make a Vector3d object from coordinate table values
Graphics::Vector3d tableToVector(lua_Object table);

// get a function stored in a table
lua_Object getTableFunction(lua_Object table, const char *name);

} // end of namespace Grim

#endif
