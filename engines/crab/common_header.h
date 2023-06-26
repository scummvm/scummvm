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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_COMMON_HEADER_H
#define CRAB_COMMON_HEADER_H

#include "common/scummsys.h"
#include "common/path.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/stablemap.h"
#include "common/multimap.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/events.h"
#include "common/savefile.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"
#include "graphics/cursor.h"
#include "graphics/wincursor.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"

#define RAPIDXML_NO_EXCEPTIONS
#include "crab/rapidxml/rapidxml.hpp"
#include "crab/rapidxml/rapidxml_print.hpp"

#endif // CRAB_COMMON_HEADER_H
