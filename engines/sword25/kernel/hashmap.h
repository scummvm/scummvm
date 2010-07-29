// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef BS_HASHMAP_H
#define BS_HASHMAP_H

// stdext::hash_map wird erst seit VC7 untersützt, bei älteren Microsoft-Compilern wird auf std::map zurückgegriffen
#include "kernel/memlog_off.h"
#if _MSC_VER >= 1300
#include <hash_map>
#define BS_Hashmap stdext::hash_map
#else
#include <map>
#define BS_Hashmap std::map
#endif
#include "kernel/memlog_on.h"

#endif
