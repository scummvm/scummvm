// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef LUA_HH
#define LUA_HH

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <luadebug.h>
#include <lauxlib.h>
}

// Register Residual builtin functions and structures
void register_lua();

// Like dofile, except it loads the file to execute from a bundle file
int bundle_dofile(const char *filename);

// Set system.frameTime
void set_frameTime(float frameTime);

// Set smush.movieTime
void set_movieTime(float movieTime);

// Get the event handler function with the given name, pushing the handler
// object if appropriate
lua_Object getEventHandler(const char *name);

#endif
