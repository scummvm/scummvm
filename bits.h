// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef BITS_H
#define BITS_H

#include <SDL/SDL_byteorder.h>
#include <stdint.h>
#include "vector3d.h"

inline uint8_t get_uint8(const char *data) {
  return *(reinterpret_cast<const unsigned char *>(data));
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN

inline uint32_t get_LE_uint32(const char *data) {
  const unsigned char *udata = reinterpret_cast<const unsigned char *>(data);
  return udata[0] | (udata[1] << 8) | (udata[2] << 16) | (udata[3] << 24);
}

inline uint16_t get_LE_uint16(const char *data) {
  const unsigned char *udata = reinterpret_cast<const unsigned char *>(data);
  return udata[0] | (udata[1] << 8);
}

inline uint16_t get_BE_uint16(const char *data) {
  return *(reinterpret_cast<const uint16_t *>(data));
}

inline uint32_t get_BE_uint32(const char *data) {
  return *(reinterpret_cast<const uint32_t *>(data));
}

#else

inline uint32_t get_LE_uint32(const char *data) {
  return *(reinterpret_cast<const uint32_t *>(data));
}

inline uint16_t get_LE_uint16(const char *data) {
  return *(reinterpret_cast<const uint16_t *>(data));
}

inline uint16_t get_BE_uint16(const char *data) {
  const unsigned char *udata = reinterpret_cast<const unsigned char *>(data);
  return (udata[0] << 8) | udata[1];
}

inline uint32_t get_BE_uint32(const char *data) {
  const unsigned char *udata = reinterpret_cast<const unsigned char *>(data);
  return (udata[0] << 24) | (udata[1] << 16) | (udata[2] << 8) | udata[3];
}

#endif

#ifdef i386

inline float get_float(const char *data) {
  return *(reinterpret_cast<const float *>(data));
}

#else

#error get_float not implemented on non-i386 machines yet

#endif

inline Vector3d get_vector3d(const char *data) {
  return Vector3d(get_float(data), get_float(data + 4), get_float(data + 8));
}

#endif
