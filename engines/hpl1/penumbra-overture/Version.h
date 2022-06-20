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
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of Penumbra Overture.
 */

#ifndef VERSION_H_
#define VERSION_H_
/*
 * This is the Penumbra EP1 Version Log file
 * Add a revision history for ever release made (and commit so the magic version # gets updated)
 */
#define PRODUCT_NAME "Penumbra: Overture"
#define PRODUCT_VERSION "$Rev: 3013 $"
#define PRODUCT_DATE "$Date: 2010-02-06 22:45:08 +0000 $"
/* Revision History */
/*
 * Linux: 2007-05-28
 *  * http://frictionalgames.com/forum/showthread.php?tid=1125
 *  * * No Steam in Refinery
 *  * http://frictionalgames.com/forum/showthread.php?tid=1122
 *  * * Slowness in Refinery
 */
/*
 * Mac: 2007-06-02
 *  * Workaround for Cg issues (always use ARBVP/FP).  Added debugging so "available" profiles will be logged on users systems.
 *  * Fixed all Shaders to use float2 instead of float3 for texture coordinates (apple opengl doesn't like it)
 */
/*
 * Mac: 2007-06-15
 *  * Add in override configs for ARB/FP
 *  * Fixed shaders to use float3 instead of float4 for spotlightUv (tex2Dproj expects float3 not float4)
 *  * make mouse unlock in menu when windowed
 */
/*
 * Mac: 2007-10-22
 *  * Fix Shadows on ATI
 *  * Fix saturated lighting
 */
/*
 * Mac: 2007-10-30
 *  * Fix Motion Blur
 *  * Fix Crash
 */
/*
 * Mac: 2007-11-12
 *  * Fix Multiple Crashes
 *  * Allow Install KEy to be in either game directory OR personal data directory.
 */
/*
 * Mac: 2007-11-19
 *  * Fix more crashes
 *  * Update Skeletal Optimization
 */
/*
 * Mac & Linux: 2008-03-06
 *  * Updated to latest version of the engine
 *  * Many iterator crash fixes fixed
 *  * Fixed them bloody Shaders again
 */
/*
 * Mac & Linux: 2008-04-09
 *  * Final Mac Build
 *  * Initial linux builds
 */
/**
 * Mac & Linux: 2009-07-23
 * - Include latest ATI fixes
 */
/**
 * Mac & Linux: 2010-02-06
 * - OpenAL fixes
 * - map->erase() fixes
 */
#endif /*VERSION_H_*/
