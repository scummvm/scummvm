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

#ifndef AGS_LIB_ALLEGRO_ALCONFIG_H
#define AGS_LIB_ALLEGRO_ALCONFIG_H

namespace AGS3 {

/* which color depths to include? */
#define ALLEGRO_COLOR8
#define ALLEGRO_COLOR16
#define ALLEGRO_COLOR24
#define ALLEGRO_COLOR32

#ifndef INLINE
#define INLINE
#endif

#ifndef RET_VOLATILE
#define RET_VOLATILE   volatile
#endif

#ifndef ZERO_SIZE_ARRAY
#define ZERO_SIZE_ARRAY(type, name)             type name[]
#endif

#ifndef AL_CONST
#define AL_CONST                                const
#endif

#ifndef AL_VAR
#define AL_VAR(type, name)                      extern type name
#endif

#ifndef AL_ARRAY
#define AL_ARRAY(type, name)                    extern type name[]
#endif

#ifndef AL_FUNC
#define AL_FUNC(type, name, args)               type name args
#endif

#ifndef AL_PRINTFUNC
#define AL_PRINTFUNC(type, name, args, a, b)    AL_FUNC(type, name, args)
#endif

#ifndef AL_METHOD
#define AL_METHOD(type, name, args)             type (*name) args
#endif

#ifndef AL_FUNCPTR
#define AL_FUNCPTR(type, name, args)            extern type (*name) args
#endif

#ifndef AL_FUNCPTRARRAY
#define AL_FUNCPTRARRAY(type, name, args)       extern type (*name[]) args
#endif

#ifndef AL_INLINE
#define AL_INLINE(type, name, args, code)       type name args;
#endif

} // namespace AGS3

#endif
