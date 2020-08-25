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

#ifndef SCI_DETECTION_DEFINES_H
#define SCI_DETECTION_DEFINES_H

namespace Sci {

// GUI-options, primarily used by detection_tables.h
#define GAMEOPTION_PREFER_DIGITAL_SFX       GUIO_GAMEOPTIONS1
#define GAMEOPTION_ORIGINAL_SAVELOAD        GUIO_GAMEOPTIONS2
#define GAMEOPTION_MIDI_MODE                GUIO_GAMEOPTIONS3
#define GAMEOPTION_JONES_CDAUDIO            GUIO_GAMEOPTIONS4
#define GAMEOPTION_KQ6_WINDOWS_CURSORS      GUIO_GAMEOPTIONS5
#define GAMEOPTION_SQ4_SILVER_CURSORS       GUIO_GAMEOPTIONS6
#define GAMEOPTION_EGA_UNDITHER             GUIO_GAMEOPTIONS7
// HIGH_RESOLUTION_GRAPHICS availability is checked for in SciEngine::run()
#define GAMEOPTION_HIGH_RESOLUTION_GRAPHICS GUIO_GAMEOPTIONS8
#define GAMEOPTION_ENABLE_BLACK_LINED_VIDEO GUIO_GAMEOPTIONS9
#define GAMEOPTION_HQ_VIDEO                 GUIO_GAMEOPTIONS10
#define GAMEOPTION_ENABLE_CENSORING         GUIO_GAMEOPTIONS11
#define GAMEOPTION_LARRYSCALE               GUIO_GAMEOPTIONS12
#define GAMEOPTION_UPSCALE_VIDEOS           GUIO_GAMEOPTIONS13

} // End of namespace Sci

#endif
