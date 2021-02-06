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

#ifndef AGS_ENGINE_AC_SYSTEMAUDIO_H
#define AGS_ENGINE_AC_SYSTEMAUDIO_H

#include "ags/engine/ac/dynobj/scriptaudiochannel.h"

namespace AGS3 {

int     System_GetColorDepth();
int     System_GetOS();
int     System_GetScreenWidth();
int     System_GetScreenHeight();
int     System_GetViewportHeight();
int     System_GetViewportWidth();
const char *System_GetVersion();
int     System_GetHardwareAcceleration();
int     System_GetNumLock();
int     System_GetCapsLock();
int     System_GetScrollLock();
void    System_SetNumLock(int newValue);
int     System_GetVsync();
void    System_SetVsync(int newValue);
int     System_GetWindowed();
int     System_GetSupportsGammaControl();
int     System_GetGamma();
void    System_SetGamma(int newValue);
int     System_GetAudioChannelCount();
ScriptAudioChannel *System_GetAudioChannels(int index);
int     System_GetVolume();
void    System_SetVolume(int newvol);
const char *System_GetRuntimeInfo();

} // namespace AGS3

#endif
