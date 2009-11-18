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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CESCALER
#define CESCALER

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "graphics/scaler/intern.h"

/**
 * This filter (down)scales the source image horizontally by a factor of 3/4.
 * For example, a 320x200 image is scaled to 240x200.
 */
DECLARE_SCALER(PocketPCPortrait);

/**
 * This filter (up)scales the source image vertically by a factor of 6/5.
 * For example, a 320x200 image is scaled to 320x240.
 *
 * The main difference to the code in graphics/scaler/aspect.cpp is the
 * out-of-place operation, omitting a straight blit step the sdl backend
 * does. Also, tests show unaligned access errors with the stock aspect scaler.
 */
DECLARE_SCALER(PocketPCLandscapeAspect);

/**
 * This filter (down)scales the source image horizontally by a factor of 2/3
 * and vertically by 7/8. For example, a 320x200 image is scaled to 213x175.
 *
 * @note The ARM asm version seems to work differently ?!? It apparently scales
 * horizontally by 11/16. Thus a 320x200 image is scaled to 220x175.
 */
DECLARE_SCALER(SmartphoneLandscape);

#endif
