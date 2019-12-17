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

#include "ultima/ultima8/misc/pent_include.h"

namespace Ultima {
namespace Ultima8 {
namespace PentagramVersion {

const char *const version = "ScummVM";

const char *features = ""
#ifdef DEBUG
                       "debug "
#endif
#ifdef USE_VALGRIND
                       "valgrind "
#endif
#ifdef USE_TIMIDITY_MIDI
                       "Timidity "
#endif
#ifdef USE_FMOPL_MIDI
                       "FMOPL "
#endif
#ifdef USE_FLUIDSYNTH_MIDI
                       "FluidSynth "
#endif
#ifdef USE_ALSA_MIDI
                       "ALSA "
#endif
#ifdef USE_HQ2X_SCALER
                       "hq2x "
#endif
#ifdef USE_HQ3X_SCALER
                       "hq3x "
#endif
#ifdef USE_HQ4X_SCALER
                       "hq4x "
#endif
#ifdef BUILTIN_DATA
                       "nodata "
#endif
                       ;

const char *buildtime = __DATE__ " " __TIME__;

} // End of namespace PentagramVersion
} // End of namespace Ultima8
} // End of namespace Ultima
