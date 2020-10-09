/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_DIRECT_SOUND_H__
#define ICB_DIRECT_SOUND_H__

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_types.h"

namespace ICB {

class MusicManager;
class SpeechManager;
class FxManager;

#define MAX_FX 24 // 16 Sound Effects channels

extern SpeechManager *g_theSpeechManager;
extern FxManager *g_theFxManager;
extern bool8 g_TimerOn;

bool8 Init_Sound_Engine();  // Setup the sound engine
bool8 Close_Sound_Engine(); // Shutdown the sound engine

// This is called ten times a second on an internal timer
bool8 Poll_Sound_Engine(); // Updates the sound engine

} // End of namespace ICB

#endif //__DIRECT_SOUND_H__
